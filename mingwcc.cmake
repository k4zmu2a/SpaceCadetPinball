set(TOOLCHAIN_PREFIX "x86_64-w64-mingw32")

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}-g++")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}-gcc")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}-objcopy")
set(CMAKE_STRIP "${TOOLCHAIN_PREFIX}-strip")
set(CMAKE_SIZE "${TOOLCHAIN_PREFIX}-size")
set(CMAKE_AR "${TOOLCHAIN_PREFIX}-ar")
set(ASSEMBLER "${TOOLCHAIN_PREFIX}-as")


set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
#set(CMAKE_SYSROOT "/usr/x86_64-w64-mingw32")

# adjust the default behavior of the find commands:
# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY) 

# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

set(SDL2_PATH /usr/x86_64-w64-mingw32)

#include_directories(BEFORE SYSTEM "/usr/x86_64-w64-mingw32/include/SDL2")
#list(APPEND CMAKE_IGNORE_PATH /usr/include/SDL2)
#message("CMAKE_IGNORE_PATH: ${CMAKE_IGNORE_PATH}")