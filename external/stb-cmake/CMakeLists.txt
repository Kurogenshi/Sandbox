set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../stb")

if(NOT EXISTS "${STB_DIR}/stb_image.h")
    message(FATAL_ERROR "stb sources not found. Did you run git submodule update --init?")
endif()

add_library(stb STATIC
    stb_implementation.cpp
    "${STB_DIR}/stb_image.h"
    "${STB_DIR}/stb_image_write.h"
)

add_library(stb::stb ALIAS stb)

target_include_directories(stb PUBLIC
    $<BUILD_INTERFACE:${STB_DIR}>
)

if(MSVC)
    target_compile_options(stb PRIVATE /W0)
else()
    target_compile_options(stb PRIVATE -w)
endif()