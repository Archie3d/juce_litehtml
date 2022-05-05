add_subdirectory(juce_litehtml)

juce_add_modules(
    juce_litehtml
)

target_include_directories(juce_litehtml INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/juce_litehtml)
