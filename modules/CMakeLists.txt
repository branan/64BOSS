
# Usage: add_module(${MODNAME} [DEFAULT_ENABLE] [DEFAULT_BUILTIN] ${MODULE_SRCS})
macro(add_module MODNAME)
    set(DEFAULT_ENABLE FALSE)
    set(DEFAULT_BUILTIN FALSE)

    foreach(ARG ${ARGN})
        if(${ARG} STREQUAL DEFAULT_ENABLE)
            set(DEFAULT_ENABLE TRUE)
        elseif(${ARG} STREQUAL DEFAULT_BUILTIN)
            set(DEFAULT_BUILTIN TRUE)
        else()
            set(MODULE_SRCS ${MODULE_SRCS} ${ARG})
        endif()
    endforeach()

    option(ENABLE_${MODNAME} "Build the ${MODNAME} kernel module" ${DEFAULT_ENABLE})
    option(BUILTIN_${MODNAME} "Build the ${MODNAME} module as a builtin module" ${DEFAULT_BUILTIN})

    if(ENABLE_${MODNAME})
        if(BUILTIN_${MODNAME})
            add_definitions(-DMODULE_BUILTIN)
            add_library(${MODNAME} STATIC ${MODULE_SRCS})
            set_target_properties(${MODNAME} PROPERTIES
                COMPILE_FLAGS "-m64 -fno-builtin -fno-exceptions -fno-rtti -nostdlib -mcmodel=kernel"
                COMPILE_DEFINITIONS MODULE_BUILTIN)
            set(64BOSS_BUILTIN_MODULES ${64BOSS_BUILTIN_MODULES} ${MODNAME} PARENT_SCOPE)
        else(BUILDTIN_${MODNAME})
            add_library(${MODNAME} SHARED ${MODULE_SRCS})
            set_target_properties(${MODNAME} PROPERTIES
                LINK_FLAGS "-fPIC -m64 -T${CMAKE_SOURCE_DIR}/module.ld -nostdlib -nostartfiles -nodefaultlibs -z max-page-size=0x1000"
                COMPILE_FLAGS "-fPIC -m64 -fno-builtin -fno-exceptions -fno-rtti -nostdlib -mcmodel=kernel"
                SUFFIX ".kmod"
                PREFIX ""
                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/modules)
        endif(BUILTIN_${MODNAME})
    endif(ENABLE_${MODNAME})
endmacro(add_module)

include_directories(${CMAKE_SOURCE_DIR}/modules)

add_subdirectory(exeformat)
message(STATUS Var in modules: ${64BOSS_BUILTIN_MODULES})
# We need to push this variable all the way up to the 
# main kernel
set(64BOSS_BUILTIN_MODULES ${64BOSS_BUILTIN_MODULES} PARENT_SCOPE)
