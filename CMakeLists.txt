add_subdirectory(juce_quickjs)
add_subdirectory(juce_litehtml)

juce_add_modules(
    ALIAS_NAMESPACE juce
    juce_quickjs
    juce_litehtml
)

target_include_directories(juce_quickjs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/juce_quickjs)
target_include_directories(juce_litehtml INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/juce_litehtml)

target_link_libraries(juce_quickjs INTERFACE quickjs)
