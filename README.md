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

TODO: instructions for downloading dependencies


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

Add toxfsd as a friend and then you can send command: `toxfs-send <file>`.

```
# Absolute
toxfs-send /mnt/myshared/myfile.txt

# Relative, implies /mnt/myshared/myfile2.txt
toxfs-send myfile2.txt

# Fails (currently only logs an error on the console)
toxfs-send /mnt/notmyshared/notmyfile.txt
```

## Dependencies

* Build and Runtime
  * toxcore >= 0.2.10
  * libfuse >= (TODO)
* Build Only
  * A C++17 compliant compiler (GCC > 8 or Clang > 9)
  * CMake >= 3.16
  * fmtlib >= 8

TODO: mention downloaded dependencies


## GPG fingerprints

GPG keys used to sign commits:

```
Jack Guo - CA78E66A15436ADA83E31E011CF19C2F3DD00269
```
