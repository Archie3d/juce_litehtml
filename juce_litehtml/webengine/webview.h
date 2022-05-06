#pragma once

#include "litehtml.h"

namespace juce_litehtml {

class WebView : public juce::Component
{
public:

    WebView();
    ~WebView();

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setDocument (litehtml::document::ptr doc);
    litehtml::document_container& getRenderer() noexcept;

private:

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebView)
};

} // namespace juce_litehtml

