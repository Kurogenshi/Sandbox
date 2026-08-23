cmake_minimum_required(VERSION 3.21...4.1)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use a separate build directory.")
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available configurations" FORCE)
set(CMAKE_SUPPRESS_REGENERATION ON)
set(CMAKE_SKIP_INSTALL_RULES    ON)

project(Sandbox
    VERSION     0.1.0
    DESCRIPTION "Sandbox OpenGL 4.6"
    LANGUAGES   C CXX
)

option(SANDBOX_WARNINGS_AS_ERRORS "Treat compiler warnings as errors"                  ON)
option(SANDBOX_SANITIZE           "Enable AddressSanitizer, and UBSan where available" OFF)
option(SANDBOX_IPO                "Enable link time optimization in Release"           ON)

get_property(SANDBOX_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT SANDBOX_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif()

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_C_STANDARD            11)
set(CMAKE_C_STANDARD_REQUIRED   ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(SANDBOX_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT SANDBOX_IPO_SUPPORTED OUTPUT SANDBOX_IPO_REASON)
    if(SANDBOX_IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    else()
        message(STATUS "Link time optimization unavailable: ${SANDBOX_IPO_REASON}")
    endif()
endif()

if(SANDBOX_SANITIZE AND MSVC)
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

add_subdirectory(external)
add_subdirectory(src)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
             PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

message(STATUS "${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  Compiler ............ ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Warnings as errors .. ${SANDBOX_WARNINGS_AS_ERRORS}")
message(STATUS "  Sanitizers .......... ${SANDBOX_SANITIZE}")
message(STATUS "  LTO in Release ...... ${SANDBOX_IPO}")
