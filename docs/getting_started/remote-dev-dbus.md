## Connecting to dbus apis on a remote machine
Soon after starting a device you will find a need to 
connect to the devices dbus apis remotely.

Exposing the dbus interface over tcp is not secure.
You can run `systemd-stdio-bridge` over ssh that 
links remote dbus apis.

See [https://man.archlinux.org/man/core/systemd/systemd-stdio-bridge.1.en](https://man.archlinux.org/man/core/systemd/systemd-stdio-bridge.1.en)
This mail also describes the solution well [https://lists.freedesktop.org/archives/dbus/2018-April/017447.html](https://lists.freedesktop.org/archives/dbus/2018-April/017447.html)