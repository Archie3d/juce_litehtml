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

    class Client
    {
    public:
        virtual ~Client() = default;
        virtual bool followLink (const juce::URL& url) = 0;
    };

    WebPage();
    ~WebPage();

    void loadFromURL (const juce::URL& url);
    void loadFromHTML (const juce::String& html);
    void reload();

    void followLink (const juce::URL& url);

    litehtml::document::ptr getDocument();
    WebLoader& getLoader();

    void setClient (Client* client);
    Client* getClient();

private:

    friend class WebView;

    void setRenderer (litehtml::document_container* renderer);

    class ViewClient
    {
    public:
        virtual ~ViewClient() = default;
        virtual void documentAboutToBeReloaded() = 0;
        virtual void documentLoaded() = 0;
    };

    void setViewClient (ViewClient* view);

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebPage)
};

} // namespace juce_litehtml
