add_library(quickjs STATIC)

file(GLOB_RECURSE src
    ${CMAKE_CURRENT_SOURCE_DIR}/quickjs/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quickjs/*.c
)

target_include_directories(quickjs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/quickjs
)

target_sources(quickjs PRIVATE ${src})

target_compile_definitions(quickjs
    PUBLIC
        JS_STRICT_NAN_BOXING=1
        CONFIG_BIGNUM=1
        CONFIG_JSX=1
    PRIVATE
        _SRC_SECURE_NO_WARNINGS
)

# ==============================================================================

add_library(litehtml STATIC)

file(GLOB_RECURSE src
    ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/src/*.cpp
)

target_include_directories(litehtml
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/include/litehtml
        ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/src
        ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/src/gumbo/include
        ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/src/gumbo/include/gumbo
)

if(MSVC)
    target_include_directories(litehtml
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/litehtml/src/gumbo/visualc/include
    )
endif()

target_compile_definitions(litehtml
    PUBLIC
        LITEHTML_UTF8=1
)

target_sources(litehtml PRIVATE ${src})

target_link_libraries(litehtml PUBLIC quickjs)
