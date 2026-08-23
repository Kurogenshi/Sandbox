set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw EXCLUDE_FROM_ALL)

set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(glm EXCLUDE_FROM_ALL)

add_subdirectory(glad)

add_subdirectory(imgui-cmake)

add_subdirectory(stb-cmake)

function(sandbox_mark_external directory)
    get_property(targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        set_property(TARGET ${target} PROPERTY FOLDER "External")
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "UTILITY" AND NOT CMAKE_VERSION VERSION_LESS 3.25)
            set_property(TARGET ${target} PROPERTY SYSTEM ON)
        endif()
    endforeach()

    get_property(subdirectories DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)
    foreach(subdirectory IN LISTS subdirectories)
        sandbox_mark_external("${subdirectory}")
    endforeach()
endfunction()

sandbox_mark_external("${CMAKE_CURRENT_SOURCE_DIR}")
