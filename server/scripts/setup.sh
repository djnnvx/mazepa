#!/bin/env bash

# this script is useful to install
# all the dependencies required to run the server
#
# for now, it only supports (and targets) Debian-based distros

set -euo pipefail


install_debian() {
    sudo apt install -y clang clang-tools libxkbcommon-dev
}

apt search clang >/dev/null || { echo "Only debian-based distros are supported" && exit 1 ; }
install_debian

