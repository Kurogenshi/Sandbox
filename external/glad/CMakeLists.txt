add_library(glad STATIC
    src/gl.c
    include/glad/gl.h
    include/KHR/khrplatform.h
)

add_library(glad::glad ALIAS glad)

target_include_directories(glad PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})

if(MSVC)
    target_compile_options(glad PRIVATE /W0)
else()
    target_compile_options(glad PRIVATE -w)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    src/gl.c
    include/glad/gl.h
    include/KHR/khrplatform.h
)
