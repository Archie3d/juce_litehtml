namespace juce_litehtml {

struct WebPage::Impl
{
    WebContext context;
    litehtml::document::ptr document { nullptr };
    litehtml::document_container* renderer { nullptr };
    URL pageUrl;

    WebPage::ViewClient* viewClient { nullptr };
    WebPage::Client* client { nullptr };

    Impl()
    {
    }

    void loadFromURL (const URL& url)
    {
        if (renderer == nullptr)
            return;

        auto& loader { context.getLoader() };

        const auto fixedUrl { loader.fixUpURL (url) };
        pageUrl = fixedUrl;

        // @note This is a workaround for the server-generated reources
        //       which will have the same URL, so will be delivered from
        //       cache and will never update.
        //loader.purgeCache();

        loader.setBaseURL (fixedUrl);

        loader.loadAsync<String> (fixedUrl, [this](bool ok, const String& html) -> void {
            if (ok)
            {
                loadFromHTML (html);
            }
        });
    }

    void loadFromHTML (const String& html)
    {
        if (renderer == nullptr)
            return;

        if (viewClient != nullptr)
            viewClient->documentAboutToBeReloaded();

        document = litehtml::document::createFromUTF8 (html.toRawUTF8(), renderer, &context);

        if (viewClient != nullptr)
            viewClient->documentLoaded();
    }

    void reload()
    {
        loadFromURL (pageUrl);
    }
};

//==============================================================================

WebPage::WebPage()
    : d { std::make_unique<Impl>() }
{
}

WebPage::~WebPage() = default;

WebContext& WebPage::getContext()
{
    return d->context;
}

void WebPage::loadFromURL (const URL& url)
{
    d->loadFromURL (url);
}

void WebPage::loadFromHTML (const String& html)
{
    d->loadFromHTML (html);
}

void WebPage::reload()
{
    d->reload();
}

void WebPage::followLink (const URL& url)
{
    const auto fixedURL { d->context.getLoader().fixUpURL (url) };
    bool shouldFollow { true };

    if (d->client != nullptr)
        shouldFollow = d->client->followLink (fixedURL);

    if (shouldFollow)
        d->loadFromURL (fixedURL);
}

URL WebPage::getURL() const
{
    return d->pageUrl;
}

litehtml::document::ptr WebPage::getDocument()
{
    return d->document;
}

WebLoader& WebPage::getLoader()
{
    return d->context.getLoader();
}

void WebPage::setClient (Client* client)
{
    d->client = client;
}

WebPage::Client* WebPage::getClient()
{
    return d->client;
}

void WebPage::setRenderer (litehtml::document_container* renderer)
{
    d->renderer = renderer;
}

void WebPage::setViewClient (ViewClient* view)
{
    d->viewClient = view;
}

} // namespace juce_litehtml
