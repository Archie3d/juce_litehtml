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
