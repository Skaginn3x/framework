# Windows

Creating a developing environment on Windows can be a bit of a hassle, this document should make it easier for anyone setting it up.

This will cover the following
1. Make Linux environment with windows subsystem for Linux.
2. Setting up a compiler to make new applications (todo)
3. Use [Cockpit](https://github.com/cockpit-project/cockpit/) to configure new and or current applications of `tfc`
4. Propose methods for developing `HMI` with `flutter` (todo)


## Create Linux environment

This wiki will use Debian to host the Linux environment, why, good question, it is easy to configure it to be rolling release distribution. Why do we need rolling release, the base of `tfc` uses the newest and greatest of `C++` so it makes the developer experience easier to use an up to date distribution. If you like to use an old distribution, like Ubuntu, you would either need to add `PPA` to fetch a new compiler set or build your own compiler.

Let's get to it.

Check that version of wsl is compliant for the following steps, minimum required is 0.67.6. Please update you have too old
```powershell
wsl --version
```

Follow the instructions on [windows-wsl](https://learn.microsoft.com/en-us/windows/wsl/install) or open powershell and run the following command
```powershell
wsl --install Debian
```

Now you should be inside a Linux environment, if not enter it by running `wsl` in powershell, enable [systemd](https://github.com/systemd/systemd/) in our environment:

```bash
sudo nano /etc/wsl.conf # write the following text, CTRL+O to save and CTRL+X to exit.

[boot]
systemd=true
```

Now shutdown the Linux environment, run `exit` inside it and then the following `wsl --shutdown` in the powershell window.

Turn back on wsl by running `wsl`.

Verify systemd by running `top` and `PID 1` should be systemd, CTRL+C to exit.

Update the environment
```bash
sudo apt update
sudo apt upgrade
```

Install cockpit
```bash
sudo apt install cockpit
```

Enable cockpit
```bash
sudo systemctl enable --now cockpit.socket
```

Reboot environment
```bash
sudo reboot
```

Run `wsl` again
```powershell
wsl
```

Fetch the IP address of `eth0`
```bash
ip addr
```

Login to the <ip>:9090 in your preferred browser.
You should be able to login

Now let's change the Debian to rolling release,

```bash
sudo nano /etc/apt/sources.list # Add the following line, CTRL+O to save and CTRL+X to exit. 

deb http://deb.debian.org/debian testing main
```

Update and restart
```bash
sudo apt update
sudo apt upgrade
sudo reboot
```


Download and install the latest release of `tfc`
```bash
sudo apt update
sudo apt install curl wget
curl -s https://api.github.com/repos/skaginn3x/framework/releases/latest \
                              | grep "tfc-framework-release-x86_64.deb" \
                              | cut -d : -f 2,3 \
                              | tr -d \" \
                              | wget -qi -
sudo apt install ./tfc-framework-release-x86_64.deb
```


Start ipc-ruler
```bash
sudo systemctl enable --now ipc-ruler@def.service
```

Check its status
```bash
sudo systemctl status ipc-ruler@def.service
```

Now you should be able to get access to all of `tfc` services through the Cockpit UI.
