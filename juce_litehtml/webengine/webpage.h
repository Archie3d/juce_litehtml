#pragma once

namespace juce_litehtml {

class WebPage final
{
public:
    WebPage (WebView* view = nullptr);
    ~WebPage();

    void setView (WebView* view);

    void loadFromURL (const juce::URL& url);
    void loadFromHTML (const juce::String& html);

private:

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebPage)
};

} // namespace juce_litehtml
