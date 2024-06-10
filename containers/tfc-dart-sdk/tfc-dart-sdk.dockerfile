FROM ghcr.io/skaginn3x/skaginn3x/framework/tfc-runtime:latest

# run with docker run -it --rm --privileged --volume=/sys/fs/cgroup:/sys/fs/cgroup:rw --cgroupns=host --name tfc-dart ghcr.io/skaginn3x/skaginn3x/framework/tfc-dart-sdk:latest

RUN apt install -y wget gpg
RUN wget -qO- https://dl-ssl.google.com/linux/linux_signing_key.pub \
      | gpg  --dearmor -o /usr/share/keyrings/dart.gpg

RUN echo 'deb [signed-by=/usr/share/keyrings/dart.gpg arch=amd64] https://storage.googleapis.com/download.dartlang.org/linux/debian stable main' \
      | tee /etc/apt/sources.list.d/dart_stable.list

RUN apt-get update && apt-get install dart

RUN apt remove -y wget gpg
RUN apt auto-remove -y
RUN apt-get clean autoclean
RUN rm -rf /var/lib/{apt,dpkg,cache,log}/

# dart does not start as com.skaginn3x service and therefore the tfc.conf does not allow communication between test and tfc
RUN sed -i 's|deny own=|allow own=|g' /usr/share/dbus-1/system.conf
RUN sed -i 's|deny send_type="method_call"|allow send_type="method_call"|g' /usr/share/dbus-1/system.conf

VOLUME [ "/sys/fs/cgroup" ]
CMD ["/lib/systemd/systemd", "--system"]
