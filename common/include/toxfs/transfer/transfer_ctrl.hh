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

#include "toxfs/tox/tox_if.hh"
#include "toxfs/util/message_queue.hh"
#include "toxfs/util/chunked_progress.hh"

#include <filesystem>
#include <future>
#include <memory>
#include <unordered_map>
#include <thread>
#include <functional>
#include <fstream>
#include <string_view>

namespace toxfs::transfer
{

class transfer_ctrl : public tox::file_callback_if
{
public:
    /**
     * @brief ctor
     * @param[in] tox_if - tox_if
     * @param[in] root_dir - the root directory
     */
    transfer_ctrl(
        std::shared_ptr<tox::tox_if> tox_if,
        std::filesystem::path root_dir);

    ~transfer_ctrl() noexcept override;

    /**
     * @brief send a file or directory of files to a friend
     * @param[in] fr_id - the friend id to send to
     * @param[in] filepath - the file/dir to send
     * @return TODO
     */
    void send_path(tox::friend_id_t fr_id, std::string_view path_str);

private:

    /* tox::file_callback_if */

    void on_tox_file_recv(tox::unique_file_id_t id, tox::file_info_t info) noexcept override;

    void on_tox_file_control(tox::unique_file_id_t id, tox::file_control_t control) noexcept override;

    void on_tox_file_chunk_request(tox::unique_file_id_t id, tox::file_chunk_request_t request) noexcept override;

    void on_tox_file_chunk_receive(tox::unique_file_id_t id, tox::file_chunk_t chunk) noexcept override;

    void on_tox_file_error(tox::unique_file_id_t id, tox::tox_error err) noexcept override;

    /* END tox::file_callback_if */

    void work_thread_run_() noexcept;

    // TODO: these are probably be moved outside
    enum transfer_type_t
    {
        send,
        recv
    };

    struct transfer_t
    {
        transfer_type_t transfer_type;
        std::fstream stream;
        std::fstream::pos_type lastPos = 0u;
        chunked_progress progress;
        bool active = false;

        transfer_t(transfer_type_t t, std::filesystem::path const& path, uint64_t filesize);
    };

    std::shared_ptr<tox::tox_if> tox_if_;
    std::filesystem::path root_dir_;
    std::unordered_map<tox::unique_file_id_t, transfer_t> transfers_;

    // TODO: proper multi-threading
    message_queue<std::function<void()>, 256> work_queue_;
    std::thread work_thread_;
};

} // namespace toxfs::transfer
