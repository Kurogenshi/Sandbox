set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../imgui")

add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_library(imgui::imgui ALIAS imgui)

target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${IMGUI_DIR}>
    $<BUILD_INTERFACE:${IMGUI_DIR}/backends>
)

target_compile_definitions(imgui PUBLIC
    IMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/ImGuiConfig.h"
    GLFW_INCLUDE_NONE
)

target_link_libraries(imgui PUBLIC glfw glad::glad)

if(MSVC)
    target_compile_options(imgui PRIVATE /W0)
else()
    target_compile_options(imgui PRIVATE -w)
endif()