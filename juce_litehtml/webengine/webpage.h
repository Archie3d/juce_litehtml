#pragma once

#include "litehtml.h"

namespace juce_litehtml {

/** A single web page.

    This class represents a single web page. It contains
    the HTML document model, which cal be loaded from a
    string or a local or remote resource.

    To be visualized, a page must be associated with a view.

    @see WebView
*/
class WebPage final
{
public:

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void documentLoaded() {};
    };

    WebPage();
    ~WebPage();

    void loadFromURL (const juce::URL& url);
    void loadFromHTML (const juce::String& html);
    void reload();

    void addListener (Listener* listener);
    void removeListener (Listener* listener);

    litehtml::document::ptr getDocument();
    WebLoader& getLoader();

private:

    friend class WebView;

    void setRenderer (litehtml::document_container* renderer);

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebPage)
};

} // namespace juce_litehtml
