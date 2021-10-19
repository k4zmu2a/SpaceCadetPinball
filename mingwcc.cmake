set(TOOLCHAIN_PREFIX "x86_64-w64-mingw32")

set(CMAKE_SYSTEM_NAME Windows)

#posix for std::this_thread, might work with just g++
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}-g++-posix")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}-gcc")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}-objcopy")
set(CMAKE_STRIP "${TOOLCHAIN_PREFIX}-strip")
set(CMAKE_SIZE "${TOOLCHAIN_PREFIX}-size")
set(CMAKE_AR "${TOOLCHAIN_PREFIX}-ar")
set(ASSEMBLER "${TOOLCHAIN_PREFIX}-as")
set(CMAKE_RC_COMPILER "${TOOLCHAIN_PREFIX}-windres")


set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# adjust the default behavior of the find commands:
# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY) 

# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Add the path to your toolchain version of SDL2
set(SDL2_PATH /usr/${TOOLCHAIN_PREFIX})