namespace juce_litehtml {

//==============================================================================

using namespace litehtml;

class PreloadContainer final : public litehtml::document_container
{
public:
    uint_ptr create_font (const tchar_t*, int, int, font_style, unsigned int, font_metrics*) override { return {}; }
    void delete_font (uint_ptr) override {}
    int text_width (const tchar_t*, uint_ptr) override { return {}; }
    void draw_text (uint_ptr, const tchar_t*, uint_ptr, web_color, const position&) override {}
    int pt_to_px (int) const override { return {}; }
    int get_default_font_size() const override { return {}; }
    const tchar_t* get_default_font_name() const override { return _t(""); }
    void draw_list_marker (uint_ptr, const list_marker&) override {}

    void load_image (const tchar_t* src, const tchar_t* baseurl, bool redraw_on_ready) override
    {
        resources.emplace_back (Resource::Type::IMAGE, URL (juceString (src)));
    }

    void get_image_size (const tchar_t* src, const tchar_t* baseurl, litehtml::size& sz) override
    {
        // @todo
    }

    void draw_background (uint_ptr, const background_paint&) override {}
    void draw_borders (uint_ptr, const borders&, const position&, bool) override {}
    void set_caption (const tchar_t*) override {}

    void set_base_url (const tchar_t* base_url)
    {
        // @todo
    }

    void link (const std::shared_ptr<litehtml::document>&, const litehtml::element::ptr&) override {}
    void on_anchor_click (const tchar_t*, const litehtml::element::ptr&) override {}
    void set_cursor (const tchar_t*) override {}
    void transform_text (litehtml::tstring&, litehtml::text_transform) override {}

    void import_css (tstring&, const tstring& tsurl, tstring& baseurl) override
    {
        resources.emplace_back (Resource::Type::TEXT, URL (juceString (tsurl)));
    }

    void import_script (litehtml::tstring&, const tstring& tsurl) override
    {
        resources.emplace_back (Resource::Type::TEXT, URL (juceString (tsurl)));
    }

    void set_clip (uint_ptr, const position&, const border_radiuses&, bool, bool) override {}
    void del_clip(uint_ptr) override {}
    void get_client_rect (position&) const override {}
    element::ptr create_element (const tchar_t*, const string_map&, const document::ptr&) override { return nullptr; }
    void get_media_features(media_features& media) const override {}
    void get_language (tstring&, tstring&) const override {}
    tstring resolve_color (const tstring&) const override { return {}; }

    void load (WebLoader& loader)
    {
        loadCounter = 0;

        for (const auto& resource : resources)
        {
            const auto fixedUrl { loader.fixUpURL (resource.url) };

            switch (resource.type)
            {
                case Resource::Type::TEXT:
                    loadText (loader, fixedUrl);
                    break;
                case Resource::Type::IMAGE:
                    loadImage (loader, fixedUrl);
                    break;
                default:
                    jassertfalse;
                    break;
            }
        }
    }

    std::function<void()> onLoadFinished{};

private:

    void loadText (WebLoader& loader, const URL& url)
    {
        loader.loadAsync<String> (url, [this](bool, const String&) -> void {
            resourceLoaded();
        });
    }

    void loadImage (WebLoader& loader, const URL& url)
    {
        loader.loadAsync<Image> (url, [this](bool, const Image&) -> void {
            resourceLoaded();
        });
    }

    void resourceLoaded()
    {
        loadCounter += 1;

        if (loadCounter == resources.size())
        {
            // All resources have been loaded
            if (onLoadFinished)
                onLoadFinished();
        }
    }

    struct Resource
    {
        enum class Type { TEXT, IMAGE };

        Resource (Type t, const URL& u)
            : type { t },
              url { u }
        {}

        Type type { Type::TEXT };
        URL url{};
    };

    std::vector<Resource> resources{};

    size_t loadCounter { 0 };
};

//==============================================================================

struct WebPage::Impl
{
    WebContext context;
    litehtml::document::ptr document { nullptr };
    litehtml::document_container* renderer { nullptr };
    URL pageUrl;

    WebPage::ViewClient* viewClient { nullptr };
    WebPage::Client* client { nullptr };

    PreloadContainer preloader;

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
        if (viewClient != nullptr)
            viewClient->documentAboutToBeReloaded();

        preloader.onLoadFinished = [this, htm = html]() {
            reloadAfterPreload (htm);
        };

        // Parse the document for content to be preloaded
        //
        // @todo This will actually block while parsing the document
        //       and its stylesheets. Perhaps we could create a lighter version
        //       to only extract the referenced images, and imported styles and scripts.
        auto preloadDocument = litehtml::document::createFromUTF8 (html.toRawUTF8(), &preloader, &context);

        // Preload the content
        preloader.load (context.getLoader());
    }

    void reload()
    {
        loadFromURL (pageUrl);
    }

private:

    /** Reload the document.

        This assumes that the document referenced content (images, styles, scripts)
        has been already preloaded/cached so can be efficiently delivered now.
     */
    void reloadAfterPreload (const String& html)
    {
        if (renderer == nullptr)
            return;

        document = litehtml::document::createFromUTF8 (html.toRawUTF8(), renderer, &context);

        if (viewClient != nullptr)
            viewClient->documentLoaded();
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

    if (view != nullptr)
        d->context.setView (view->getView());
}

} // namespace juce_litehtml
