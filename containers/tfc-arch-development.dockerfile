FROM archlinux:latest

# Setup non root build user
RUN useradd -m --shell=/bin/bash build && usermod -L build
RUN echo "build ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
RUN echo "root ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Install yay and aur helper
RUN pacman -Syu --needed git sudo base-devel --noconfirm
RUN sudo -u build git clone https://aur.archlinux.org/yay.git /tmp/yay-bin && cd /tmp/yay-bin && sudo -u build makepkg -si --noconfirm
RUN sudo -u build yay --version

# Install dependencies
RUN sudo -u build yay -S cmake-git llvm-git gcc-git --noconfirm

RUN git clone https://github.com/microsoft/vcpkg /opt/vcpkg

# RUN sed -i 's|deny own=|allow own=|g' /usr/share/dbus-1/system.conf
# RUN sed -i 's|deny send_type="method_call"|allow send_type="method_call"|g' /usr/share/dbus-1/system.conf
