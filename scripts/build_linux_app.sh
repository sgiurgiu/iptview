#!/bin/bash

set -x

export VCPKG_BINARY_SOURCES=files,/opt/a,readwrite

CMAKE_ARGS="-GNinja -DVCPKG_INSTALLED_DIR=/opt/vcpkg/installed \
            -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
            -DCMAKE_BUILD_TYPE=Release \
            -DENABLE_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr"
distro=""
if [ -n "$1" ]
then
    distro=$1
    CMAKE_ARGS="${CMAKE_ARGS} -DCPACK_DISTRIBUTION=${distro}"
    if [[ "${distro}" == "ubuntu" || "${distro}" == "debian" ]]
    then
        echo "Building a deb package"
        CMAKE_ARGS="${CMAKE_ARGS} -DCPACK_GENERATOR=DEB"
    fi
    if [[ "${distro}" == "fedora" || "${distro}" == "redhat" || "${distro}" == "centos" || "${distro}" == "suse" ]]
    then
        echo "Building an rpm package"
        CMAKE_ARGS="${CMAKE_ARGS} -DCPACK_GENERATOR=RPM"
    fi
fi

cmake_exe=$(find /opt/ -name cmake -type f -executable -print)
if [[ -z ${cmake_exe} ]]
then
    echo "Cannot find cmake in /opt using whatever is installed"
    cmake_exe="cmake"
fi

${cmake_exe} -B/tmp/build -S/tmp/iptview ${CMAKE_ARGS}
${cmake_exe} --build /tmp/build -- package

mkdir -p /tmp/iptview/packages
cp /tmp/build/iptview-*-${distro}.* /tmp/iptview/packages/
