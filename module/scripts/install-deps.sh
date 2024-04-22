#!/usr/bin/env bash

set -euo pipefail

# installing required headers
sudo apt-get install linux-headers-`uname -r`

# install and configure xbuild for x86
cargo install cargo-xbuild
rustup component add --toolchain=nightly rust-src
rustup component add rustfmt-preview

