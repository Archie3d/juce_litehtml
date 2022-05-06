#pragma once

namespace juce_litehtml {

/** Web page view.

    This component visualizes a web page associated with it.
*/
class WebView : public juce::Component
{
public:

    WebView();
    ~WebView();

    void setPage (WebPage* page);
    WebPage* getPage();

    // juce::Component
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebView)
};

} // namespace juce_litehtml

