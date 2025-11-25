#!/usr/bin/pwsh
git submodule update --init
cmake --preset windows-debug
cmake --build --preset windows-debug