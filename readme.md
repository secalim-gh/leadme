# leadme
Simple LeaderKey style launcher.

## Install
Just git clone the repo and launch install.sh, that will build and install the binary to /usr/local/bin/
```sh
git clone https://github.com/secalim-gh/leadme.git
cd leadme
./install.sh
```
## Configuration
Create ~/.config/leadme/config
```
of=firefox
t=ghostty
```
That would create o->f path for firefox, and a direct mapping for ghostty with t.

## How to
Launch
```sh
leadme -s
```
To start the server. Then launch
```sh
leadme
```
For the client.
## Deps
On certain systems (Wayland) is better to have gtk-layer-shell
```bash
    sudo dnf install -y gtk3 gtk3-devel gtk-layer-shell gtk-layer-shell-devel
```

