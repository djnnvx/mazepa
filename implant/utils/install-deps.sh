#!/bin/env bash

#
# install the dev dependencies to
# compile this program
#
# assumes you are running a debian-based distro
# if this is not what you use, you are welcome to
# change the script for your needs :)~
#


set -euo pipefail


# install using APT
install_apt () {
    sudo apt install -y \
        libxkbcommon-dev
}


install_apt
