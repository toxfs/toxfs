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

## Usage (Proof of Concept)

**WARNING**: This functionality is a experimental hack, use at your own risk.

Currently toxfsd can currently connect to a client and initiate a file transfer based on a message.

To get started run toxfsd with your Tox address and a directory to serve:

```bash
$ toxfsd 0E831AAF... /mnt/myshared
```

After bootstrapping, toxfsd will display it's own Tox address:
```
[tox/tox.cc:305] INFO: My tox address is: a42c8bedab...
```

Add toxfsd as a friend, **it will only accept a friend request of the address provided on the command line**.

After successfully friending, then you can send a message: `toxfs-send <file>`.

```
# Absolute
toxfs-send /mnt/myshared/myfile.txt

# Relative, implies /mnt/myshared/myfile2.txt
toxfs-send myfile2.txt

# Fails (currently only logs an error on the console)
toxfs-send /mnt/notmyshared/notmyfile.txt
```

This will cause toxfsd to initiate a file transfer with your tox client.

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
