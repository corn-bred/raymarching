# Raymarcher
Simple raymarcher with SDFs.

### Dynamically-Linked Libraries

`cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++`
`cmake --build build`

### Statically-Linked Libraries

`cmake -S . -B build -G Ninja -DSTATIC_BUILD=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++`
`cmake --build build`