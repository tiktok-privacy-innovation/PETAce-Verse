# PETAce-Verse

## Introduction

PETAce-Verse is a collection of primitive cryptographic protocols.
It is one of the many components in [the framework PETAce](https://github.com/tiktok-privacy-innovation/PETAce).

PETAce-Verse implements frequently, repeatedly called subprotocols, as implied by the same "Verse".
Examples are oblivious transfer, oblivious pseudorandom functions, (vector) oblivious linear evaluation, etc.
Currently, PETAce-Verse includes: [Naor-Pinkas OT](https://dl.acm.org/doi/10.5555/365411.365502) and [IKNP OT](https://link.springer.com/chapter/10.1007/978-3-540-45146-4_9) with [optimization](https://link.springer.com/article/10.1007/s00145-016-9236-6).

## Requirements

| System | Toolchain                                             |
|--------|-------------------------------------------------------|
| Linux  | Clang++ (>= 5.0) or GNU G++ (>= 5.5), CMake (>= 3.15) |

| Required dependency                                                            | Tested version | Use                             |
|--------------------------------------------------------------------------------|----------------|---------------------------------|
| [PETAce-Solo](https://github.com/tiktok-privacy-innovation/PETAce-Solo)       | 0.1.0          | Cryptography primitives         |
| [PETAce-Network](https://github.com/tiktok-privacy-innovation/PETAce-Network) | 0.1.0          | Network communication protocols |

| Optional dependency                                | Tested version | Use               |
|----------------------------------------------------|----------------|-------------------|
| [GoogleTest](https://github.com/google/googletest) | 1.12.1         | For running tests |

## Building PETAce-Verse

We assume that all commands presented below are executed in the root directory of PETAce-Verse.

To build PETAce-Verse library (optionally with test and example):

```bash
cmake -S . -B build -DVERSE_BUILD_TEST=ON -DVERSE_BUILD_EXAMPLE=ON
cmake --build build
```

Output binaries can be found in `build/lib/` and `build/bin/` directories.

| Compile Options           | Values        | Default | Description                                         |
|---------------------------|---------------|---------|-----------------------------------------------------|
| `CMAKE_BUILD_TYPE`        | Release/Debug | Release | Debug mode decreases run-time performance.          |
| `VERSE_BUILD_SHARED_LIBS` | ON/OFF        | OFF     | Build a shared library if set to ON.                |
| `VERSE_BUILD_EXAMPLE`     | ON/OFF        | ON      | Build C++ example if set to ON.                     |
| `VERSE_BUILD_TEST`        | ON/OFF        | ON      | Build C++ test if set to ON.                        |
| `VERSE_BUILD_DEPS`        | ON/OFF        | ON      | Download and build unmet dependencies if set to ON. |

Here we give a simple example to run protocols in PETAce-Verse.

To run Party A

```shell
./build/bin/verse_example 127.0.0.1 8899 127.0.0.1 8890 0
```

To run Party B

```shell
./build/bin/verse_example 127.0.0.1 8890 127.0.0.1 8899 1
```

## Contribution

Please check [Contributing](CONTRIBUTING.md) for more details.

## Code of Conduct

Please check [Code of Conduct](CODE_OF_CONDUCT.md) for more details.

## License

This project is licensed under the [Apache-2.0 License](LICENSE).

## Citing PETAce

To cite PETAce in academic papers, please use the following BibTeX entries.

### Version 0.1.0

```tex
    @misc{petace,
        title = {PETAce (release 0.1.0)},
        howpublished = {\url{https://github.com/tiktok-privacy-innovation/PETAce}},
        month = July,
        year = 2023,
        note = {TikTok Pte. Ltd.},
        key = {PETAce}
    }
```
