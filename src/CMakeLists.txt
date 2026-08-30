set(SANDBOX_SOURCES
    main.cpp
    core/window.cpp
	renderer/Shader.cpp
	renderer/Texture.cpp
)

set(SANDBOX_HEADERS
    core/window.h
	core/Macros.h
	core/ScopeExit.h
	renderer/GlHandle.h
	renderer/Shader.h
	renderer/Texture.h
	io/Binary.h
)

add_executable(${PROJECT_NAME}
    ${SANDBOX_SOURCES}
    ${SANDBOX_HEADERS}
)

if(TARGET glm::glm)
    set(SANDBOX_GLM glm::glm)
elseif(TARGET glm-header-only)
    set(SANDBOX_GLM glm-header-only)
else()
    message(FATAL_ERROR "No GLM target found. Are the submodules initialized?")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad::glad
    imgui::imgui
	stb::stb
    ${SANDBOX_GLM}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLFW_INCLUDE_NONE
    GLM_FORCE_RADIANS
	GLM_FORCE_XYZW_ONLY
    GLM_FORCE_EXPLICIT_CTOR
    GLM_ENABLE_EXPERIMENTAL
    SANDBOX_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
    $<$<CONFIG:Debug>:SANDBOX_DEBUG>
    $<$<CONFIG:Release>:SANDBOX_RELEASE>
)

if(MSVC)

    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /permissive-
        /utf-8
        /Zc:__cplusplus
        /Zc:preprocessor
        /Zc:inline
        /Zc:throwingNew
        /MP
        /external:W0

        /w14062
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
        /w14289
        /w14296
        /w14311
        /w14545
        /w14546
        /w14547
        /w14549
        /w14555
        /w14619
        /w14640
        /w14826
        /w14905
        /w14906
        /w14928
        /w15038
        /w15204
        /w15219
        /w15240
        /w15262
        /w15263

        $<$<CONFIG:Debug>:/sdl>
        $<$<AND:$<CONFIG:Debug>,$<NOT:$<BOOL:${SANDBOX_SANITIZE}>>>:/JMC>

        $<$<CONFIG:Release>:/Zi>
        $<$<CONFIG:Release>:/Gw>
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
        $<$<CONFIG:Release>:/INCREMENTAL:NO>
    )

    if(SANDBOX_WARNINGS_AS_ERRORS)
        target_compile_options(${PROJECT_NAME} PRIVATE /WX)
    endif()

    if(SANDBOX_SANITIZE)
        target_compile_options(${PROJECT_NAME} PRIVATE /fsanitize=address)
        target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO)
    endif()

    set_property(TARGET ${PROJECT_NAME}
                 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

else()

    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wdouble-promotion
        -Wcast-align
        -Wcast-qual
        -Wold-style-cast
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wnull-dereference
        -Wimplicit-fallthrough
        -Wmisleading-indentation
        -Wextra-semi
        -Wformat=2

        $<$<CONFIG:Debug>:-g3>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>

        $<$<CONFIG:Release>:-g>
        $<$<CONFIG:Release>:-ffunction-sections>
        $<$<CONFIG:Release>:-fdata-sections>
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
        )
    endif()

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>
    )

    if(NOT APPLE)
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:LINKER:--gc-sections>
        )
    endif()

    if(SANDBOX_WARNINGS_AS_ERRORS)
        target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
    endif()

    if(SANDBOX_SANITIZE)
        target_compile_options(${PROJECT_NAME} PRIVATE
            -fsanitize=address,undefined
            -fno-sanitize-recover=all
            -fno-omit-frame-pointer
        )
        target_link_options(${PROJECT_NAME} PRIVATE
            -fsanitize=address,undefined
        )
    endif()

endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    ${SANDBOX_SOURCES}
    ${SANDBOX_HEADERS}
)
