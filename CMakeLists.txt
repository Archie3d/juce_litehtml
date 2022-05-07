add_subdirectory(juce_litehtml)

juce_add_modules(
    ALIAS_NAMESPACE juce
    juce_litehtml
)

target_include_directories(juce_litehtml INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/juce_litehtml)

target_link_libraries(juce_litehtml INTERFACE litehtml quickjs)
