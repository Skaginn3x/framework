# syntax=docker/dockerfile:1

# run with docker run -it --rm --privileged --volume=/sys/fs/cgroup:/sys/fs/cgroup:rw --cgroupns=host --name tfc-runtime ghcr.io/skaginn3x/skaginn3x/framework/tfc-runtime:latest

FROM debian:testing-slim

RUN apt-get update && apt-get install -y systemd dbus-broker
# Let's keep the dbus-daemon for now, it is easier to fork from terminal when using the container in workflows
# $ dbus-daemon --system
#RUN apt-get remove -y dbus-daemon

RUN ([ -d /lib/systemd/system/sysinit.target.wants ] && cd /lib/systemd/system/sysinit.target.wants/ && for i in *; do [ $i == \
systemd-tmpfiles-setup.service ] || rm -f $i; done); \
rm -f /lib/systemd/system/multi-user.target.wants/*;\
rm -f /etc/systemd/system/*.wants/*;\
rm -f /lib/systemd/system/local-fs.target.wants/*; \
rm -f /lib/systemd/system/sockets.target.wants/*udev*; \
rm -f /lib/systemd/system/sockets.target.wants/*initctl*; \
rm -f /lib/systemd/system/basic.target.wants/*;\
rm -f /lib/systemd/system/anaconda.target.wants/*

RUN apt install -y curl wget
RUN curl -s https://api.github.com/repos/skaginn3x/framework/releases/latest \
                                  | grep "browser_download_url.*tfc-framework-release-x86_64.deb" \
                                  | cut -d : -f 2,3 \
                                  | tr -d \" \
                                  | wget -qi -

RUN apt install -y ./tfc-framework-release-x86_64.deb
RUN rm -f ./tfc-framework-release-x86_64.deb

RUN apt remove -y curl wget

RUN apt auto-remove -y
RUN apt-get clean autoclean
RUN rm -rf /var/lib/{apt,dpkg,cache,log}/

RUN systemctl enable ipc-ruler@def
RUN systemctl enable operations@def

# todo remove
RUN [ -f /usr/local/share/dbus-1/system.d/tfc.conf ] && mv /usr/local/share/dbus-1/system.d/tfc.conf /etc/dbus-1/system.d/

VOLUME [ "/sys/fs/cgroup" ]
CMD ["/lib/systemd/systemd", "--system"]
