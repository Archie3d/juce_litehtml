#pragma once

namespace juce_litehtml {

class WebView : public juce::Component
{
public:

    WebView();
    ~WebView();

private:

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebView)
};

} // namespace juce_litehtml

