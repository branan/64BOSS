project(64boss)
cmake_minimum_required(VERSION 2.8)

set(kernel_SRCS
    loader.S
    boot.S

    cxx.cpp
    earlyprint.cpp
    kernel.cpp
    kmain.cpp
    kutils.cpp
    kutils.S
    memallocator.cpp
    modloader.cpp
    pageallocator.cpp
    pagemapper.cpp
)

set_property(SOURCE loader.S PROPERTY LANGUAGE C)
set_property(SOURCE kutils.S PROPERTY LANGUAGE C)
set_property(SOURCE boot.S PROPERTY LANGUAGE C)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(modules)

add_definitions(-DBUILTIN_MODULES="${64BOSS_BUILTIN_MODULES}")
add_executable(kernel ${kernel_SRCS} kernel.ld)
set_target_properties(kernel PROPERTIES
    LINK_FLAGS "-m64 -T${CMAKE_SOURCE_DIR}/kernel.ld -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -z max-page-size=0x1000 -Wl,-whole-archive"
    COMPILE_FLAGS "-m64 -fno-builtin -fno-exceptions -fno-rtti -nostdlib -mcmodel=kernel")
message(STATUS var in root: ${64BOSS_BUILTIN_MODULES})
target_link_libraries(kernel ${64BOSS_BUILTIN_MODULES})

add_custom_target(harddisk.img
    COMMAND ${CMAKE_SOURCE_DIR}/makeimg.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS kernel
)

add_custom_target(run
    COMMAND qemu-system-x86_64 -serial stdio -hda harddisk.img
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS harddisk.img
)

add_custom_target(dbg
    COMMAND qemu-system-x86_64 -serial stdio -hda harddisk.img -s -S
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS harddisk.img
)
