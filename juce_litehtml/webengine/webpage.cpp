#include "litehtml.h"

namespace juce_litehtml {

struct WebPage::Impl
{
    WebLoader loader;
    litehtml::context context;
    litehtml::document::ptr document { nullptr };
    litehtml::document_container* renderer { nullptr };

    ListenerList<WebPage::Listener> listeners;

    Impl()
    {
        context.load_master_stylesheet(master_css);
    }

    void loadFromURL (const URL& url)
    {
        if (renderer == nullptr)
            return;

        loader.setBaseURL (url);
        const auto html { loader.loadText (url) };
        loadFromHTML (html);
    }

    void loadFromHTML (const String& html)
    {
        if (renderer == nullptr)
            return;

        document = litehtml::document::createFromUTF8 (html.toRawUTF8(), renderer, &context);
        notifyDocumentLoaded();
    }

    void notifyDocumentLoaded()
    {
        listeners.call([](Listener& listener) { listener.documentLoaded(); });
    }
};

//==============================================================================

WebPage::WebPage()
    : d { std::make_unique<Impl>() }
{
}

WebPage::~WebPage() = default;

void WebPage::loadFromURL (const URL& url)
{
    d->loadFromURL (url);
}

void WebPage::loadFromHTML (const String& html)
{
    d->loadFromHTML (html);
}

void WebPage::addListener (Listener* listener)
{
    d->listeners.add (listener);
}

void WebPage::removeListener (Listener* listener)
{
    d->listeners.remove (listener);
}

litehtml::document::ptr WebPage::getDocument()
{
    return d->document;
}

WebLoader& WebPage::getLoader()
{
    return d->loader;
}

void WebPage::setRenderer (litehtml::document_container* renderer)
{
    d->renderer = renderer;
}

} // namespace juce_litehtml
