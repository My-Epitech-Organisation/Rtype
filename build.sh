git submodule update --init
cmake -S . -B my-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build my-build-debug
