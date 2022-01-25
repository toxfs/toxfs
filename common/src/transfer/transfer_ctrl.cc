/**
 * Copyright (C) 2021 by The Toxfs Project Contributers
 *
 * This file is part of Toxfs.
 *
 * Toxfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toxfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Toxfs.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "toxfs/transfer/transfer_ctrl.hh"
#include "toxfs/tox/file_types_fmt.hh"
#include "toxfs/logging.hh"
#include "toxfs/exception.hh"

#include <vector>
#include <utility>

namespace toxfs::transfer
{

transfer_ctrl::transfer_t::transfer_t(transfer_type_t t, std::filesystem::path const& path, uint64_t filesize)
    : transfer_type(t)
    , stream(path, (t == transfer_type_t::send ? std::ios_base::in : (std::ios_base::out | std::ios_base::trunc))
            | std::ios_base::binary)
    , progress(filesize)
{}

transfer_ctrl::transfer_ctrl(
    std::shared_ptr<tox::tox_if> tox_if,
    std::filesystem::path root_dir)
    : tox_if_(std::move(tox_if))
    , root_dir_(std::move(root_dir))
{
    tox_if_->register_file_callback_if(*this);
    work_thread_ = std::thread([this]() { work_thread_run_(); });
}

transfer_ctrl::~transfer_ctrl() noexcept
{
    work_thread_.join();
    tox_if_->unregister_file_callback_if(*this);
}

void transfer_ctrl::send_path(tox::friend_id_t fr_id, std::string_view path_str)
{
    std::filesystem::path path{path_str};

    if (path.is_relative())
        path = root_dir_ / path;

    if (!std::filesystem::exists(path))
        throw TOXFS_EXCEPTION(runtime_error, fmt::format("Cannot send {}: file does not exist", path.native()));

    path = std::filesystem::canonical(path);
    std::error_code ec;
    auto rel_path = std::filesystem::relative(path, root_dir_, ec);
    if (ec || *rel_path.begin() == "..")
        throw TOXFS_EXCEPTION(runtime_error,
                fmt::format("Cannot send {}: file not in root dir ({})!", path.native(), root_dir_.native()));

    std::vector<std::filesystem::path> send_files;
    if (std::filesystem::is_symlink(path))
    {
        throw TOXFS_EXCEPTION(runtime_error, "TODO: Symlinks not supported");
    }
    else if (std::filesystem::is_directory(path))
    {
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{path})
        {
            if (dir_entry.is_directory())
                continue;

            if (!dir_entry.is_regular_file())
                throw TOXFS_EXCEPTION(runtime_error, "TODO: Cannot handle special files!");

            send_files.push_back(dir_entry.path());
        }

        TOXFS_LOG_INFO("Sending a directory to Friend#{} with {} files", fr_id.id, send_files.size());
    }
    else
    {
        send_files = {path};
    }

    std::vector<std::tuple<std::filesystem::path, uintmax_t, std::future<tox::unique_file_id_t>>> send_results;
    send_results.reserve(send_files.size());
    for (auto const& send_file : send_files)
    {
        auto const filename = send_file.filename().string();
        auto const filesize = std::filesystem::file_size(send_file);

        TOXFS_LOG_INFO("Sending a file to Friend#{} with name {} size {}", fr_id.id, filename, filesize);
        send_results.emplace_back(std::move(send_file), filesize,
                tox_if_->send_file(fr_id, tox::file_info_t{std::move(filename), filesize}));
    }

    for (auto& [send_file, filesize, future] : send_results)
    {
        // FIXME: this does a blocking operation
        auto id = future.get();

        work_queue_.push([this, id, send_file = std::move(send_file), &filesize]()
        {
            // TODO: handle not inserting
            transfers_.emplace(id, transfer_t{transfer_type_t::send, send_file, filesize});
        });
    }
}

void transfer_ctrl::on_tox_file_recv(tox::unique_file_id_t id, tox::file_info_t info) noexcept
{
    TOXFS_LOG_INFO("Received a file {} with name {} size {}", id, info.filename, info.filesize);

    auto path = root_dir_ / info.filename;

    if (std::filesystem::exists(path))
    {
        TOXFS_LOG_WARNING("Overwriting existing file: {}", path.native());
    }
    else
    {
        TOXFS_LOG_INFO("Saving file to: {}", path.native());
    }

    work_queue_.push([this, id, path = std::move(path), filesize = info.filesize]()
    {
        auto [it, ok] = transfers_.emplace(id, transfer_t{transfer_type_t::recv, path, filesize});
        if (!ok)
        {
            TOXFS_LOG_ERROR("Transfer already exists: {}", id);
            return;
        }
        it->second.active = true;
        tox_if_->send_file_control(id, tox::file_control_t::resume);
    });
}

void transfer_ctrl::on_tox_file_control(tox::unique_file_id_t id, tox::file_control_t control) noexcept
{
    work_queue_.push([this, id, control]()
    {
        auto it = transfers_.find(id);
        if (it != transfers_.end())
        {
            transfer_t& tr = it->second;
            switch (control)
            {
            case tox::file_control_t::cancel:
                TOXFS_LOG_INFO("Transfer {} has been cancelled", id);
                transfers_.erase(it);
                break;
            case tox::file_control_t::pause:
                TOXFS_LOG_DEBUG("Transfer {} PAUSE", id);
                tr.active = false;
                break;
            case tox::file_control_t::resume:
                TOXFS_LOG_DEBUG("Transfer {} RESUME", id);
                tr.active = true;
                break;
            }
        }
        else
        {
            TOXFS_LOG_WARNING("Control received for {} but this transfer does not exist!", id);
        }
    });
}

void transfer_ctrl::on_tox_file_chunk_request(tox::unique_file_id_t id, tox::file_chunk_request_t request) noexcept
{
    work_queue_.push([this, id, request]()
    {
        auto it = transfers_.find(id);
        if (it != transfers_.end())
        {
            transfer_t& tr = it->second;
            if (tr.transfer_type != transfer_type_t::send)
            {
                TOXFS_LOG_ERROR("Chunk requested for non-send transfer {}", id);
                return;
            }

            if (!tr.active)
            {
                TOXFS_LOG_ERROR("Chunk requested for inactive transfer {}", id);
                return;
            }

            if (request.size == 0)
            {
                TOXFS_LOG_INFO("End of send transfer {}", id);
                transfers_.erase(it);
                return;
            }

            auto chunk_pos = static_cast<std::streamoff>(request.position);
            if (tr.lastPos != chunk_pos)
                tr.stream.seekg(chunk_pos);

            buffer_t buf{request.size};

            auto chunk_size = static_cast<std::streamoff>(request.size);
            tr.stream.read(reinterpret_cast<char*>(buf.data()), chunk_size);
            if (tr.stream)
            {
                buf.set_size(chunk_size);
                tox_if_->send_file_chunk(id, tox::file_chunk_t{request.position, std::move(buf)});
                tr.lastPos += chunk_size;
                tr.progress.update(request.position, request.size);
            }
            else
            {
                TOXFS_LOG_ERROR("Error reading stream of {} at {} size {}", id, request.position, request.size);
                tr.lastPos = tr.stream.tellg();
                tr.stream.clear();
            }
        }
        else
        {
            TOXFS_LOG_WARNING("Chunk requested for {} but this transfer does not exist!", id);
        }
    });
}

void transfer_ctrl::on_tox_file_chunk_receive(tox::unique_file_id_t id, tox::file_chunk_t chunk) noexcept
{
    work_queue_.push([this, id, chunk = std::move(chunk)]()
    {
        auto it = transfers_.find(id);
        if (it != transfers_.end())
        {
            transfer_t& tr = it->second;
            if (tr.transfer_type != transfer_type_t::recv)
            {
                TOXFS_LOG_ERROR("Received chunk for non-recv transfer {}", id);
                return;
            }

            if (!tr.active)
            {
                TOXFS_LOG_ERROR("Received chunk for inactive transfer {}", id);
                return;
            }

            if (chunk.data.size() == 0)
            {
                TOXFS_LOG_INFO("End of recv transfer {}", id);
                transfers_.erase(it);
                return;
            }

            auto chunk_pos = static_cast<std::streamoff>(chunk.position);
            if (tr.lastPos != chunk_pos)
                tr.stream.seekg(chunk_pos);

            auto chunk_size = static_cast<std::streamoff>(chunk.data.size());
            tr.stream.write(reinterpret_cast<const char*>(chunk.data.data()), chunk_size);
            if (tr.stream)
            {
                tr.lastPos += chunk_size;
                tr.progress.update(chunk.position, chunk.data.size());
            }
            else
            {
                TOXFS_LOG_ERROR("Error writing to stream of {}", id);
                tr.lastPos = tr.stream.tellg();
            }
        }
        else
        {
            TOXFS_LOG_WARNING("Chunk received for {} but this transfer does not exist!", id);
        }
    });
}

void transfer_ctrl::on_tox_file_error(tox::unique_file_id_t id, tox::tox_error err) noexcept
{
    TOXFS_LOG_ERROR("{} file error: {}", id, err.what());
}

void transfer_ctrl::work_thread_run_() noexcept
{
    while (true)
    {
        auto func = work_queue_.pop();
        if (func)
        {
            try
            {
                func();
            }
            catch (toxfs::exception const& e)
            {
                TOXFS_LOG_ERROR("Error while running work function: {}", e.what());
            }
            catch (std::exception const& e)
            {
                TOXFS_LOG_ERROR("Error while running work function: {}", e.what());
            }
        }

    }
}

} // namespace toxfs::transfer

