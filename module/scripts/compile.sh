#!/usr/bin/env bash

set -euo pipefail

cd "${1}" || { echo "please choose module path to compile." && exit 1 ; }

export RUST_TARGET_PATH=$(pwd)/.. cargo xbuild --target x86_64-linux-kernel-module

make


echo "Ready to load and test. Please run the following commands:"
echo ""
echo "insmod ${1}.ko"
echo "dmesg"

