# 📦 Downloads {#downloads_page}

## Pre-built Binaries

Pre-built binaries are provided for the following distributions. Please be aware that their versions may not always match the latest Louvre release.

* **Arch** : [louvre](https://aur.archlinux.org/packages/louvre) - *Thanks to [@TrialnError](https://aur.archlinux.org/account/TrialnError)*.
* **NixOS** : [louvre](https://search.nixos.org/packages?channel=unstable&show=louvre&from=0&size=50&sort=relevance&type=packages&query=louvre) - *Thanks to [Marco Rebhan](https://github.com/2xsaiko)*.

## Manual Building

Louvre relies on the following libraries:

* **Wayland Server** >= 1.16
* **EGL** >= 1.5.0
* **GLES 2.0** >= 13.0.6
* **DRM** >= 2.4.85
* **SRM** >= 0.6.0
* **GBM** >= 22.2.0
* **Evdev** >= 1.5.6
* **Libinput** >= 1.6.3
* **XCursor** >= 1.1.15
* **XKB Common** >= 0.7.1
* **Pixman** >= 0.40.0
* **Libseat** >= 0.6.4

Wayland also backends require:

* **Wayland Client** >= 1.22.0
* **Wayland EGL** >= 18.1.0

And the examples also require:

* **Libicu** >= 72.1
* **FontConfig** >= 2.14.1
* **FreeType** >= 2.8.1

And can easily be built with [Meson](https://mesonbuild.com/).

## Debian (Ubuntu, Mint, etc)

If your distribution is based on Debian, all tools and almost all dependencies can be installed with the following commands:

```
$ sudo apt update
$ sudo apt upgrade
$ sudo apt install build-essential meson libegl-dev mesa-common-dev libgles2-mesa-dev libdrm-dev libgbm-dev libevdev-dev libinput-dev libxcursor-dev libxkbcommon-dev libpixman-1-dev libwayland-dev hwinfo libseat-dev libicu-dev libfontconfig-dev libfreetype-dev
```

If the [hwinfo](https://github.com/vcrhonek/hwdata) or [libdisplay-info](https://gitlab.freedesktop.org/emersion/libdisplay-info) packages are not available in your distribution, please download and install them manually in the specified order:

1. [hwinfo](https://packages.debian.org/buster/hwinfo)
2. [libdisplay-info1](https://packages.debian.org/sid/libdisplay-info1)
3. [libdisplay-info-dev](https://packages.debian.org/unstable/libdisplay-info-dev)

To install SRM, follow the instructions provided [here](https://cuarzosoftware.github.io/SRM/md_md__downloads.html).

It is also recommended to install [weston-terminal](https://gitlab.freedesktop.org/wayland/weston), which is compatible with Wayland and is used throughout the tutorial and the examples.

```
$ sudo apt install weston
```

## Red Hat (Fedora, CentOS, etc)

If your distribution is based on Red Hat, all tools and almost all dependencies can be installed with the following commands:

```
$ sudo dnf update
$ sudo dnf install @development-tools make automake gcc gcc-c++ meson libwayland-client libwayland-server wayland-devel libinput-devel libevdev-devel libudev-devel mesa-libEGL-devel libxkbcommon-devel libXcursor-devel pixman-devel libdrm-devel libgbm-devel libseat-devel fontconfig-devel freetype-devel libicu-devel
```

To install SRM, follow the instructions provided [here](https://cuarzosoftware.github.io/SRM/md_md__downloads.html).

It is also recommended to install [weston-terminal](https://gitlab.freedesktop.org/wayland/weston) which is compatible with Wayland and is used throughout the tutorial and the examples.

```
$ sudo dnf install weston
```

# Building Louvre

Run the following commands to compile and install Louvre:

```
$ git clone https://github.com/CuarzoSoftware/Louvre.git
$ cd Louvre/src
$ meson setup build
$ cd build
$ meson install
$ sudo ldconfig
```

To ensure that everything is functioning correctly, you can test one of the available [examples](md_md__examples.html).