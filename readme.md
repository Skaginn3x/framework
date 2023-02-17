# Time For Change 

This repository is supposed to contain the framework used to develop the future components for Baader and Skaginn3x.

## Getting started

### Package management

To begin with executables will be statically linked to third party dependencies but shared linking to libc and libc++/libstdc++.

[vcpkg](https://github.com/microsoft/vcpkg) will be used to declare, build dependencies. To set up on Arch Linux 
```bash
yay -S vcpkg
sudo gpasswd -a $USER vcpkg
sudo pacman -S libc++ lld llvm base-devel
# relogin or $ newgrp vcpkg in terminal
```
Please note the output of installing vcpkg:
```bash
    "VCPKG_ROOT" is set to "/opt/vcpkg"
    "VCPKG_DOWNLOADS" is set to "/var/cache/vcpkg"
    To cooperate with CMake, add "-DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

**please note that for the first release (first deployed product) it will be required to fixate the vcpkg dependencies version**

### Build system

Everything will be built on CMake and using features from 3.23+ version.

# Copyright
Copyright 2023 Skaginn 3X ehf

