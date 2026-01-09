# leadme
Simple [LeaderKey](https://github.com/mikker/LeaderKey) style launcher.

## Example
![output](https://github.com/user-attachments/assets/f0bd5b0c-3956-4566-8c59-17adfade34d0)

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

