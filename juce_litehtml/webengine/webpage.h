#pragma once

namespace juce_litehtml {

class WebPage final
{
public:
    WebPage();
    ~WebPage();

private:

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebPage)
};

} // namespace juce_litehtml
