#include "litehtml.h"

namespace juce_litehtml {

struct WebPage::Impl
{
    WebLoader loader;
    litehtml::context context;
    litehtml::document::ptr document { nullptr };

    WebView* view { nullptr };

    Impl()
    {
        context.load_master_stylesheet(master_css);
    }

    void loadFromURL (const URL& url)
    {
        if (view == nullptr)
            return;

        const auto html { loader.loadText (url) };
        loadFromHTML (html);
    }

    void loadFromHTML (const String& html)
    {
        if (view == nullptr)
            return;

        document = litehtml::document::createFromUTF8 (html.toRawUTF8(), &view->getRenderer(), &context);
        view->setDocument (document);
    }
};

//==============================================================================

WebPage::WebPage (WebView* view)
    : d { std::make_unique<Impl>() }
{
    if (view != nullptr)
        setView (view);
}

WebPage::~WebPage() = default;

void WebPage::setView (WebView* view)
{
    d->view = view;
}

void WebPage::loadFromURL (const URL& url)
{
    d->loadFromURL (url);
}

void WebPage::loadFromHTML (const String& html)
{
    d->loadFromHTML (html);
}

} // namespace juce_litehtml
