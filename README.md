# Toxfs

**Currently in Development!**

Once completed Toxfs will be a remote FUSE filesystem built on the Tox Protocol.

The project will consist of two parts:

**toxfsd** - the toxfs daemon, it will run on the "server" machine sharing files

**toxfuse** - the toxfs client, it will run on the "client" machine and
              provide a mountable FUSE filesystem

### What is Tox?

Tox is a peer to peer, end-to-end encrypted, chat protocol that toxfs uses for
transporting information. Toxfs specifically uses the C implementation at
[TokTok/c-toxcore](https://github.com/TokTok/c-toxcore).

Learn more about the protocol at [tox.chat](https://tox.chat).


## Building

Built as a CMake project:

```sh
git clone https://github.com/toxfs/toxfs
cd toxfs
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build
cmake --install build
```

### Downloading Dependencies

By default the build will download dependencies as needed from the web. This only happens for
dependencies that support it (marked as downloadable in [Dependencies](#Dependencies)).

This is controlled by the `DOWNLOAD_DEPS` variable in CMake:
```sh
# Never download deps, always use the system version
cmake -DDOWNLOAD_DEPS=NEVER ...

# Try the system version first and download upon fail (default)
cmake -DDOWNLOAD_DEPS=AUTO ...

# Always download
cmake -DDOWNLOAD_DEPS=ALWAYS ...
```

## Usage

**Toxfs is still in the early stages of development, use at your own risk!**

Currently toxfsd is functional and can transfer files back and forth based on messages/files from a tox client.

To start, run toxfsd with the following positional arguments:
 * 1: Your tox client address which will be used to access this toxfs.
 * 2: The directory to serve/save files to
 * 3: Optional, the path to save the tox data.
   * IMPORTANT: If this is not provided toxfsd will generate a new tox address for itself each time.

```bash
$ toxfsd 0E831AAF... /mnt/myshared /path/to/savedata
```

After bootstrapping, toxfsd will display it's own Tox address:
```
[tox/tox.cc:348] INFO: My tox address is: a42c8bedab...
```

Add toxfsd as a friend, **it will only accept a friend request of the address provided on the command line**.
If you provided a savedata path earlier, you can now send the message `save` to toxfsd to tell it to save
it's data.

Now that toxfsd is ready, you can tell it to send a file with: `send <path>`. The path can be either a file
or a directory specified as a absolute or relative path to the share. When the path is a directory
all the contents in the directory and subdirectories will be sent.

```
# Absolute
send /mnt/myshared/myfile.txt

# Relative, implies /mnt/myshared/myfile2.txt
send myfile2.txt

# Fails (currently only logs an error on the console)
send /mnt/notmyshared/notmyfile.txt
```

Sending a file to toxfsd will cause it to save the file at the root of the share. If a file with the same name
already exists, it will be overwritten.


## Dependencies

* Build and Runtime
  * toxcore >= 0.2.10
  * libfuse >= (TODO)
* Build Only
  * A C++17 compliant compiler (GCC > 8 or Clang > 9)
  * CMake >= 3.16
  * fmtlib >= 8.0.1 (downloadable)
  * microsoft-gsl >= 3.1.0 (downloadable)

## GPG fingerprints

GPG keys used to sign commits:

```
Jack Guo - CA78 E66A 1543 6ADA 83E3  1E01 1CF1 9C2F 3DD0 0269
```
