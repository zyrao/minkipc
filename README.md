# Mink IPC

## Background
This repository contains libraries that implement, and clients that utilize, a Mink-IPC interface for communication with the Qualcomm® Trusted Execution Environment (QTEE) via the QCOM-TEE driver registered with the Linux TEE subsystem.

Read more about MinkIPC [here](docs/minkipc.md).

Read more about QCOMTEE [here](https://github.com/quic/quic-teec).

### Mink Adaptor Library

The Mink Adaptor library implements a Mink-IPC interface, which in-turn allows clients to utilize the [Mink-IDL](https://github.com/quic/mink-idl-compiler) programming interface to communicate across security domain boundaries with QTEE via [QCOMTEE](https://github.com/quic/quic-teec).

## Build Instructions
```
git clone https://github.com/quic/minkipc.git
cd minkipc
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=CMakeToolchain.txt && cmake --build . --target install --config Release
```
In order to build this package, you require QCBOR and QCOMTEE. These are not available using the standard Ubuntu repository. If you do not have them installed on your machine, you can fetch, build and install them from below mentioned locations:

1. QCBOR [repository](https://github.com/laurencelundblade/QCBOR). Use `-DQCBOR_DIR_HINT=/path/to/installed/dir` to specify the QCBOR dependency.

2. QCOMTEE [repository](https://github.com/quic/quic-teec). Use `-DQCOMTEE_DIR_HINT=/path/to/installed/dir` to specify the QCOMTEE dependency.

You can optionally specify the path for installation of the `minkipc` package, via `-DCMAKE_INSTALL_PREFIX:PATH=/path/to/minkipc/install`.

## Contributions

Thanks for your interest in contributing to MinkIPC! Please read our [Contributions Page](CONTRIBUTING.md) for more information on contributing features or bug fixes. We look forward to your participation!

## License

**MinkIPC** Project is licensed under the [BSD-3-clause License](https://spdx.org/licenses/BSD-3-Clause.html). See [LICENSE.txt](LICENSE.txt) for the full license text.

