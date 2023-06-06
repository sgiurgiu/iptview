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

IPTVIEW_VERSION=$(git describe --tags)
if [ -z ${IPTVIEW_VERSION+x} ]; then
    IPTVIEW_VERSION="1.0.dev"
fi

if [ -z $1 ]; then
    distros=("debian" "fedora" "ubuntu")
else
    distros=($1)
fi

for distro in "${distros[@]}"
do
    echo "Running podman to build for distribution ${distro}"
    container=$CONTAINER_REGISTRY/qt_apps_$distro:build
    # This is a prebuilt container that has installed and compiled the require vcpkg packages
    podman pull $container
    podman run --rm --privileged=true --name iptview_build \
            -v "${root}":/tmp/iptview/:Z \
            -e IPTVIEW_VERSION="${IPTVIEW_VERSION}" \
            $container \
            /tmp/iptview/scripts/build_linux_app.sh $distro
done
