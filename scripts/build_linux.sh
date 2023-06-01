#!/bin/bash

set -ex

if [ -z ${CI_PROJECT_DIR+x} ]; then
    root=$(git rev-parse --show-toplevel)
else
    root=${CI_PROJECT_DIR}
fi

if [[ -z ${CONTAINER_REGISTRY} ]]; then
    echo "FATAL: Please set the CONTAINER_REGISTRY environment variable to point to where the containers are located."
    exit 1
fi

if [[ -z "${CONTAINER_REGISTRY+x}" ]]; then
    echo "FATAL: Please set the CONTAINER_REGISTRY environment variable to point to where the containers are located."
    exit 1
fi

if [ -z ${IPTVIEW_VERSION_MAJOR+x} ]; then
    IPTVIEW_VERSION_MAJOR="1"
    IPTVIEW_VERSION_MINOR="0"
    IPTVIEW_VERSION_PATCH="dev"
fi

if [ -z $1 ]; then
    distros=("debian" "fedora" "ubuntu")
else
    distros=($1)
fi

for distro in "${distros[@]}"
do
    echo "Running podman to build for distribution ${distro}"
    # This is a prebuilt container that has installed and compiled the require vcpkg packages
    podman pull $CONTAINER_REGISTRY/qt_apps_$distro:build
    podman run --rm --privileged=true --name iptview_build \
            -v "${root}":/tmp/iptview/:Z \
            -e IPTVIEW_VERSION_MAJOR="${IPTVIEW_VERSION_MAJOR}" \
            -e IPTVIEW_VERSION_MINOR="${IPTVIEW_VERSION_MINOR}" \
            -e IPTVIEW_VERSION_PATCH="${IPTVIEW_VERSION_PATCH}" \
            $CONTAINER_REGISTRY/qt_apps_$distro:build \
            /tmp/iptview/scripts/build_linux_app.sh $distro
done