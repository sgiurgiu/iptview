# IPTV Viewer

![](icons/iptview-icon_128.png?raw=true)

A media player aimed at facilitating watching IPTV streams.

## Usage

Grab the m3u from your IPTV service provider, load it in the application and start watching TV.
While the m3u can contain any kind of files, and therefore the player can play any media files from it (local or remote), the media player provides features suitable for managing the streams and playlists provided by IPTV service providers. For other kind of media, other players (for example VLC) are probably better suited (and have more features).

## Screenshot

![Screenshot](images/Screenshot_20230620_200811.png?raw=true)

## Build it from source

### Prerequisites
- Git
- A C++20 capable compiler (does not need to be fully compliant)
- Cmake 3.18+
- VcPkg
- LibMPV

### Setup your development environment
 - Windows
   - Install VS2022 (can be Community edition) with the C++ Support package
   - Install prerequisites (git, cmake, ninja, wixtoolset, powershell-core)
   - One simple powershell script to install everything via Chocolatey:
        ```
        Set-ExecutionPolicy Bypass -Scope Process -Force; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

        choco feature enable -n allowGlobalConfirmation
        choco install 7zip.install
        choco install git.install
        choco install cmake.install --installargs 'ADD_CMAKE_TO_PATH=System'
        choco install ninja 
        choco install wixtoolset 
        choco install powershell-core
        ```
   - Install vcpkg (assuming the vcpkg folder will be `E:\projects\vcpkg`. Change as desired.)
        ```
        New-Item -ItemType Directory -Force -Path E:\projects\
        cd E:\projects\
        git clone https://github.com/Microsoft/vcpkg.git
        cd vcpkg
        .\bootstrap-vcpkg.bat
        ```
    - Install libmpv for windows prebuilt DLL, and youtube-dl.exe
        ```
        New-Item -ItemType Directory -Force -Path E:\projects\mpv
        cd E:\projects\mpv

        # Download the latest release from libmpv folder, manually
        Start-Process https://sourceforge.net/projects/mpv-player-windows/files/libmpv/

        ```
        Download mpv-dev-x86_64-<latest release>.7z from that sourceforge page. Extract it somewhere (E:\projects\mpv).
        Edit the `mpv.def` file, and insert these 2 lines at the top:
        ```
            LIBRARY MPV-2
            EXPORTS
        ```
        Then in that mpv folder, from a VS2022 Developer command prompt run : `lib /dev:mpv.def /machine:x64 /out:mpv.lib`
    - Done with windows. Wasn't that hard now, was it? 
 - Linux Debian 11 (and Debian based distributions)
    ```
    apt install -y dpkg build-essential cmake gcc g++ git \
                libmpv-dev ca-certificates curl zip unzip tar libglu1-mesa-dev libgl1-mesa-dev \
                pkg-config generate-ninja ninja-build libxmu-dev libxi-dev libgl-dev
    ```
  - Linux Fedora (34+) 
    ```
    dnf install -yq \
    http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-stable.noarch.rpm \
    http://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-stable.noarch.rpm && \
    rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-rpmfusion-free-fedora-latest && \
    rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-rpmfusion-nonfree-fedora-latest
    ```
    ```
    dnf -yq install cmake gcc g++ pkgconf-pkg-config git \
        tar rpm-build ninja-build perl-FindBin perl-English perl-File-Compare \
        ibus-devel libXmu-devel libXi-devel mesa-libGL-devel mesa-libGLU-devel \
        mpv-libs-devel
    ```
### Actually building the executable
`cd <projects folder>`

`git clone https://github.com/sgiurgiu/iptview.git`

- Linux

    `mkdir build && cd build`

    `cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=<vcpkg folder>/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release  <git cloned folder>`

    `ninja`

    And now run it from that folder with `src/iptview`

- Windows
  ```
    cmake -B . -S <git cloned folder> -G Ninja -DCMAKE_TOOLCHAIN_FILE=<vcpkg folder>/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DENABLE_TESTS=False -DCMAKE_BUILD_TYPE=Release -DCPACK_GENERATOR=WIX -DLIBMPV_DIR="<libmpv folder>" -DLIBMPV_INCLUDE="<libmpv include folder>"
    cmake --build .
  ```

The scripts that build it on windows and linux can be found in the `scripts` folder. Those scripts are the authority when it comes to the process of building a package for a particular OS should this readme not be updated.

