#!/bin/bash
git submodule update --init
cmake --preset linux-debug
cmake --build --preset linux-debug