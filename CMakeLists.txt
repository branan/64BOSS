project(64boss)
cmake_minimum_required(VERSION 2.8)

enable_language(ASM)

set(kernel_SRCS
    loader.S
    boot.S

    earlyprint.cpp
    kmain.cpp
    kutils.cpp
    kutils.S
    memallocator.cpp
    pageallocator.cpp
    pagemapper.cpp
)

add_executable(kernel ${kernel_SRCS} kernel.ld)
set_target_properties(kernel PROPERTIES
    LINK_FLAGS "-T${CMAKE_SOURCE_DIR}/kernel.ld -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -z max-page-size=0x1000"
    COMPILE_FLAGS "-nostdlib -fno-builtin -nostartfiles -nodefaultlibs -z max-page-size=0x1000 -mcmodel=large")

add_custom_target(image
    COMMAND ${CMAKE_SOURCE_DIR}/makeimg.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS kernel
)

add_custom_target(run
    COMMAND qemu-system-x86_64 -serial stdio -hda harddisk.img
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS image
)

add_custom_target(dbg
    COMMAND qemu-system-x86_64 -serial stdio -hda harddisk.img -s -S
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS image
)
