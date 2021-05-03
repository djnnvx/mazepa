#!/bin/bash

BIN='k3yl0gg3r'

cd logger && make
which upx >/dev/null || sudo apt install -y upx
upx --best ${BIN} && ./clean_script ${BIN}

cp ${BIN} .. && make fclean
