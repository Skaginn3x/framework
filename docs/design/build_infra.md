# Building infrastructure

This section covers how the project can be built, as well as how it can be packaged.

**FYI, the code base is in its early stages and the following could not strictly apply, that being said the following is what we will try to adhere to.** 

## Developing ethics

The developing ethics of the framework is to use the newest and greatest of the `C++` standard.
However, given the developing mismatch between compilers and standard libraries for new features, if new feature is supposed to be used than
all the compilers used in this project need to support it, with the exception that a substitution can be made (for library features).

This means that normally the newest release of each compiler is needed to build the code.

## API

Backwards compatibility is not maintained for API nor ABI, the intention is to statically link the frameworks library artifacts.

However, the intention is to maintain backwards compatible inter-process communication SDL(schema definition language). Meaning
that older components built with the framework should be able to communicate with newer ones.

## Compiler infrastructure

### GNU 

`gcc` is used as the main compiler, meaning it will be used for production artifacts. 
Secondly, the `C++` library `libstdc++` is used as the main library because our target is Linux. 
Therefore, if needed to use third party dependency with shared libraries the executable binary would use the same
`C++` library, given that the third party uses GNU `C++` library.

### LLVM

`clang` is used as the main developing compiler, because it is faster and more protective of the `C++` standard.
Along with `clang` the library `libc++` is used meaning it needs to exist on the host used to develop on. However, you 
are ofcourse allowed to use gcc as your developing compiler.

### MSVC

`cl` is used to compile on Windows, however, even though the code compiles on Windows does not mean it can run on Windows.
The reason are dependencies to systemd journal and unix domain sockets, this can ofcourse be changed later on if needed.

## Compilation software

### CMake
CMake is used as the main and only compilation software and should be used in most all cases which relate to anything in this context.

### CMakePresets
CMakePresets are heavily utilized and provide the interface to compile this codebase.

### vcpkg
[vcpkg](https://github.com/microsoft/vcpkg) is used for maintaining third party dependencies.

The vpkcg infrastructure is a topic of its own but in this context the vcpkg triplet files control 
the toolchain for building the vcpkg dependencies, and secondly there are chainload toolchain files
to configure the toolchain for building this code base.

## Static analysis

### cppcheck

### clang-tidy

### code checker

## Packaging 

CPack is used to generate packages for debian, rpm and tar.gz.

### Package formats

#### Shared dependencies
Packages are built on latest Ubuntu LTS with shared linking to `glibc` and `libstdc++`.
Both debug and release packages are deployed where debug packages have postfix with `-dbg`.

Note: the debug packages do only include executables and library files.

#### Static
Packages are built on latest Alpine Linux with static linking to `muslc` and `libstdc++`.
Both debug and release packages are deployed where debug packages have postfix with `-dbg`.

All static packages have postfix `-static`.

Note: the debug packages do only include executables and library files.

### Hosted repository

Todo
