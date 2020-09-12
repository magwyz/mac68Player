# mkdir -p build ; cd build
### Replace the path to Retr68-build in this line:
# cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/josh/Retro68-build/build-target/cmake/retro68.toolchain.cmake
# make
# Then re-run make in the build directory when you want a fresh build.

cmake_minimum_required(VERSION 2.8)

include_directories(include)

add_application(mac68Player
    CREATOR MagS
    src/main.c
    src/audio.c
    CONSOLE
)
