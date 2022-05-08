namespace juce_litehtml {

using namespace litehtml;

class Renderer final : public litehtml::document_container
{
public:
    Renderer (WebView& view)
        : webView (view)
    {}

    uint_ptr create_font (const tchar_t* faceName,
                          int size,
                          int weight,
                          font_style style,
                          unsigned int decoration,
                          font_metrics* fm) override
    {
        String fontName { juceString (faceName) };
        int styleFlags { Font::plain };

        if (decoration & font_decoration_underline)
            styleFlags |= Font::underlined;
        if (style == litehtml::fontStyleItalic)
            styleFlags |= Font::italic;
        if (weight >= 500)
            styleFlags |= Font::bold;

        auto font { std::make_unique<Font> (fontName, (float) size, styleFlags) };

        if (fm != nullptr)
        {
            fm->ascent = (int)font->getAscent();
            fm->descent = (int)font->getDescent();
            fm->height = (int)font->getHeight();
            fm->x_height = fm->height;  // 'x' character height
            fm->draw_spaces = (style == litehtml::fontStyleItalic || decoration != 0);
        }

        return (uint_ptr) font.release();
    }

    void delete_font (uint_ptr hFont) override
    {
        auto* font { static_cast<Font*>((void*) hFont) };
        delete font;
    }

    int text_width (const tchar_t* text, uint_ptr hFont) override
    {
        if (auto* font { static_cast<Font*>((void*) hFont) })
            return font->getStringWidth (juceString (text));

        return 0;
    }

    void draw_text (uint_ptr hdc, const tchar_t* text, uint_ptr hFont, web_color color, const position& pos) override
    {
        if (auto* g { static_cast<Graphics*>((void*) hdc) })
        {
            if (auto* font { static_cast<Font*> ((void*) hFont) })
                g->setFont(*font);

            g->setColour (webColour (color));

            Rectangle<int> rect (pos.x, pos.y, pos.width, pos.height);
            g->drawText (juceString (text), rect, Justification::left);
        }
    }

    int pt_to_px (int pt) const override
    {
        const auto& displays { Desktop::getInstance().getDisplays() };

        if (auto* display { displays.getPrimaryDisplay() })
            return (int) (display->dpi * pt / 72.0 / display->scale);

        return pt * 8 / 6;
    }

    int get_default_font_size() const override
    {
        return 16;
    }

    const tchar_t* get_default_font_name() const override
    {
        return _t("Tahoma");
    }

    void draw_list_marker (uint_ptr hdc, const list_marker& marker) override
    {
        if (auto* g { static_cast<Graphics*> ((void*) hdc) })
        {
            const auto& pos { marker.pos };
            g->setColour (webColour (marker.color));

            switch (marker.marker_type)
            {
                case list_style_type_none:
                    break;
                case list_style_type_circle:
                    g->fillEllipse (pos.x, pos.y, pos.width, pos.height);
                    break;
                case list_style_type_disc:
                    g->drawEllipse (pos.x, pos.y, pos.width, pos.height, 1.0f);
                    break;
                case list_style_type_square:
                    g->fillRect (pos.x, pos.y, pos.width, pos.height);
                    break;
                default:
                    g->drawRect (pos.x, pos.y, pos.width, pos.height);
                    break;
            }
        }
    }

    void load_image (const tchar_t* src, const tchar_t* baseurl, bool redraw_on_ready) override
    {
        if (auto* loader { getLoader() })
        {
            const URL url (juceString (src));

            loader->loadAsync<Image> (url, [this, url, redraw_on_ready](bool ok, const Image& image) {
                if (ok && ! image.isNull())
                {
                    // Cache image size
                    imageSizeCache[url.toString (true).hash()] = { image.getWidth(), image.getHeight() };

                    if (redraw_on_ready)
                        webView.resized();
                        webView.repaint();
                }
            });
        }
    }

    void get_image_size (const tchar_t* src, const tchar_t* baseurl, litehtml::size& sz) override
    {
        if (auto* loader { getLoader() })
        {
            URL url (juceString (src));

            if (auto it { imageSizeCache.find (url.toString (true).hash()) }; it != imageSizeCache.end())
            {
                sz.width = it->second.width;
                sz.height = it->second.height;
                return;
            }

            // Image has not been cached, force local load
            Image image{};
            const bool ok { loader->loadLocal (url, image) };

            if (ok && ! image.isNull())
            {
                sz.width = image.getWidth();
                sz.height = image.getHeight();
                return;
            }
        }

        sz.width = 0;
        sz.height = 0;
    }

    void draw_background (uint_ptr hdc, const background_paint &bg) override
    {
        if (auto* g { static_cast<Graphics*> ((void*) hdc) })
        {
            if (bg.image.empty())
            {
                Rectangle<int> rect;
                rect.setX (bg.position_x + bg.clip_box.left());
                rect.setY (bg.position_y + bg.clip_box.top());
                rect.setWidth (bg.clip_box.width);
                rect.setHeight (bg.clip_box.height);

                g->setColour (webColour (bg.color));
                g->fillRect (rect);
            }
            else if (auto* loader { getLoader() })
            {
                const URL url (juceString (bg.image));
                Image image;
                const bool ok { loader->loadLocalOrCached<Image> (url, image) };

                if (ok && !image.isNull())
                {
                    // @todo handle background paint correctly
                    Rectangle<float> frect;
                    frect.setLeft (bg.position_x);
                    frect.setTop (bg.position_y);
                    frect.setWidth (bg.clip_box.width);
                    frect.setHeight (bg.clip_box.height);

                    if (bg.repeat == background_repeat_repeat)
                        g->drawImage (image, frect, RectanglePlacement::fillDestination);
                    else
                        g->drawImage (image, frect, RectanglePlacement::stretchToFit);
                }
            }
        }
    }

    void draw_borders (uint_ptr hdc, const borders& borders, const position& draw_pos, [[maybe_unused]] bool root) override
    {
        if (auto* g { static_cast<Graphics*> ((void*) hdc) })
        {
            int bdr_top { 0 };
            int bdr_bottom { 0 };
            int bdr_left { 0 };
            int bdr_right { 0 };

            if (borders.top.width > 0 && borders.top.style > border_style_hidden)
                bdr_top = borders.top.width;

            if (borders.bottom.width > 0 && borders.bottom.style > border_style_hidden)
                bdr_bottom = borders.bottom.width;

            if (borders.left.width > 0 && borders.left.style > border_style_hidden)
                bdr_left = borders.left.width;

            if (borders.right.width > 0 && borders.right.style > border_style_hidden)
                bdr_right = borders.right.width;

            // @todo Draw rounded boxes

            if (bdr_top)
            {
                g->setColour (webColour (borders.top.color));
                g->drawLine (draw_pos.left(), draw_pos.top(), draw_pos.right(), draw_pos.top());
            }

            if (bdr_bottom)
            {
                g->setColour (webColour (borders.bottom.color));
                g->drawLine (draw_pos.left(), draw_pos.bottom(), draw_pos.right(), draw_pos.bottom());
            }

            if (bdr_left)
            {
                g->setColour (webColour (borders.left.color));
                g->drawLine (draw_pos.left(), draw_pos.top(), draw_pos.left(), draw_pos.bottom());
            }

            if (bdr_right)
            {
                g->setColour (webColour (borders.right.color));
                g->drawLine (draw_pos.right(), draw_pos.top(), draw_pos.right(), draw_pos.bottom());
            }
        }
    }

    void set_caption (const tchar_t* caption) override
    {
       // @todo Notify caption changed
    }

    void set_base_url (const tchar_t* base_url) override
    {
        if (auto* loader { getLoader() })
        {
            const String sUrl (juceString (base_url));
            loader->setBaseURL (sUrl);
        }
    }

    void link (const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override
    {
        // @todo
        // Called when el_link element gets parsed
    }

    void on_anchor_click (const tchar_t* turl, const litehtml::element::ptr& el) override
    {
        if (followLink)
        {
            const URL url (juceString (turl));
            followLink (url);
        }
    }

    void set_cursor (const tchar_t* cursor) override
    {
        static std::map<String, MouseCursor::StandardCursorType> cursorTypes
        {
            { "none",       MouseCursor::NoCursor           },
            { "auto",       MouseCursor::ParentCursor       },
            { "pointer",    MouseCursor::PointingHandCursor },
            { "crosshair",  MouseCursor::CrosshairCursor    }
        };

        auto cur { MouseCursor::NormalCursor };
        const String cursorName { juceString (cursor).toLowerCase().trim() };

        if (const auto it { cursorTypes.find (cursorName) }; it != cursorTypes.end())
            cur = it->second;

        // @todo Updaye mouse cursor
        // webView.setMouseCursor (cur);
    }

    void transform_text (litehtml::tstring& text, litehtml::text_transform tt) override
    {
        String str { juceString (text.c_str()) };

        switch (tt)
        {
            case text_transform::text_transform_uppercase:
                str = str.toUpperCase();
                break;
            case text_transform::text_transform_lowercase:
                str = str.toLowerCase();
                break;
            case text_transform::text_transform_capitalize:
                if (str.isNotEmpty())
                {
                    str = str.substring (0,1).toUpperCase() + str.substring (1, str.length() - 1);
                }
            case text_transform::text_transform_none:
            default:
                break;
        }

    #ifdef LITEHTML_UTF8
        text = litehtml::tstring (str.toStdString());
    #else
        text = litehtml::tstring (str.toWideCharPointer());
    #endif
    }

    void import_css (tstring &text, const tstring &tsurl, tstring &baseurl) override
    {
        if (auto* loader { getLoader() })
        {
            const URL url (juceString (tsurl));
            const String content { loader->loadTextSync (url) };

            text = to_tstring (content);
        }
    }

    void set_clip (litehtml::uint_ptr hdc, const position &pos, const border_radiuses &bdr_radius, bool valid_x, bool valid_y) override
    {
        if (auto* g { static_cast<Graphics*> ((void*) hdc) })
        {
            g->saveState();
            const auto clip { g->getClipBounds() };
            g->reduceClipRegion(valid_x ? pos.x : clip.getX(),
                                valid_y ? pos.y : clip.getY(),
                                valid_x ? pos.width : clip.getWidth(),
                                valid_y ? pos.height : clip.getHeight());
        }
    }

    void del_clip(litehtml::uint_ptr hdc) override
    {
        if (auto* g { static_cast<Graphics*> ((void*) hdc) })
        {
            g->restoreState();
        }
    }

    void get_client_rect (position& client) const override
    {
        client.x = 0;
        client.y = 0;
        client.width = webView.getWidth();
        client.height = webView.getHeight();
    }

    litehtml::element::ptr create_element (const litehtml::tchar_t *tag_name,
                                           const litehtml::string_map &attributes,
                                           const litehtml::document::ptr& doc) override
    {
        if (auto* page { webView.getPage() })
            return page->getContext().create_element (tag_name, attributes, doc);

        return nullptr;
    }

    void get_media_features(litehtml::media_features &media) const override
    {
        const auto& displays { Desktop::getInstance().getDisplays() };

        if (auto* display { displays.getPrimaryDisplay() })
        {
            media.width = display->userArea.getWidth();
            media.height = display->userArea.getHeight();
            media.resolution = (int) display->dpi;
            media.device_width = display->totalArea.getWidth();
            media.device_height = display->totalArea.getHeight();
        }
        else
        {
            // Dummy values
            media.width = 1024;
            media.height = 768;
            media.resolution = 72;
            media.device_width = 1024;
            media.device_height = 768;
        }

        media.type = litehtml::media_type_screen;
        media.color = 8;
        media.monochrome = 0;
        media.color_index = 256;
    }

    void get_language (tstring &language, tstring &culture) const override
    {
        language = _t("en");
        culture = _t("en-GB");
    }

    tstring resolve_color (const tstring &color) const override
    {
        String colorName { juceString (color.c_str()) };

        Colour c { Colours::findColourForName (colorName, Colour::fromString (colorName)) };
        String strColour = String ("#") + c.toDisplayString (true);

    #ifdef LITEHTML_UTF8
        return litehtml::tstring (strColour.toStdString());
    #else
        return litehtml::tstring (strColour.toWideCharPointer());
    #endif
    }

    // Callbacks

    std::function<void (const URL&)> followLink{};

private:

    WebLoader* getLoader()
    {
        if (auto* page { webView.getPage() })
            return &page->getLoader();

        return nullptr;
    }

    WebView& webView;

    struct ImageSize
    {
        int width;
        int height;
    };

    std::map<size_t, ImageSize> imageSizeCache;
};

//==============================================================================

constexpr static int scrollBarSize { 10 };

struct WebView::Impl : public WebPage::ViewClient,
                       public ScrollBar::Listener
{
    WebView& self;
    Renderer renderer;
    WebPage* page { nullptr };
    WebDOM::Ptr dom { nullptr };

    ScrollBar vScrollBar;
    ScrollBar hScrollBar;
    int scrollX { 0 };
    int scrollY { 0 };

    Impl (WebView& wv)
        : self { wv },
          renderer (wv),
          vScrollBar (true),
          hScrollBar (false)
    {
        renderer.followLink = [this](const URL& url) -> void { followLink (url); };

        vScrollBar.setAutoHide (false);
        hScrollBar.setAutoHide (false);

        vScrollBar.setSingleStepSize (16);
        hScrollBar.setSingleStepSize (16);

        self.addAndMakeVisible (vScrollBar);
        self.addAndMakeVisible (hScrollBar);

        vScrollBar.addListener (this);
        hScrollBar.addListener (this);
    }

    void setPage (WebPage* newPage)
    {
        if (page != nullptr)
        {
            page->setViewClient (nullptr);
            page->setRenderer (nullptr);
        }

        page = newPage;

        if (page != nullptr)
        {
            page->setViewClient (this);
            page->setRenderer (&renderer);
            dom = page->getDOM();
        }
    }

    void render()
    {
        if (dom == nullptr)
            return;

        auto document { dom->getDocument() };

        const auto width { self.getWidth() };
        const auto height { self.getHeight() };

        document->render (width, litehtml::render_all);

        const auto documentWidth { document->width() };
        const auto documentHeight { document->height() };

        const auto hRange { jmax (0, documentWidth - width) };
        const auto vRange { jmax (0, documentHeight - height) };

        hScrollBar.setVisible (hRange > 0);
        vScrollBar.setVisible (vRange > 0);


        hScrollBar.setRangeLimits (0, documentWidth, dontSendNotification);
        vScrollBar.setRangeLimits (0, documentHeight, dontSendNotification);

        hScrollBar.setCurrentRange (scrollX, width - (vRange > 0 ? scrollBarSize : 0));
        vScrollBar.setCurrentRange (scrollY, height - (hRange > 0 ? scrollBarSize : 0));

        hScrollBar.setBounds(0, height - scrollBarSize, vRange > 0 ? width - scrollBarSize : width, scrollBarSize);
        vScrollBar.setBounds(width - scrollBarSize, 0, scrollBarSize, hRange > 0 ? height - scrollBarSize : height);
    }

    void paint (Graphics& g)
    {
        // @todo Get the colour from <body> style
        g.fillAll (Colours::white);

        if (dom == nullptr)
            return;

        auto document { dom->getDocument() };

        const auto width { vScrollBar.isVisible() ? self.getWidth() - vScrollBar.getWidth() : self.getWidth() };
        const auto height { hScrollBar.isVisible() ? self.getHeight() - hScrollBar.getHeight() : self.getHeight() };

        litehtml::position clip (0, 0, width, height);

        // Draw document at scroll position
        document->draw ((litehtml::uint_ptr)&g, -scrollX, -scrollY, &clip);
    }

    void mouseMove(const MouseEvent& event)
    {
        if (dom == nullptr)
            return;

        const int x { event.x + scrollX };
        const int y { event.y + scrollY };

        std::vector<litehtml::position> redrawBoxes;

        auto document { dom->getDocument() };

        if (document->on_mouse_over (x, y, x, y, redrawBoxes))
            renderAndPaint();
    }

    void mouseDown(const MouseEvent& event)
    {
        if (dom == nullptr)
            return;

        auto document { dom->getDocument() };

        const int x { event.x + scrollX };
        const int y { event.y + scrollY };

        std::vector<litehtml::position> redrawBoxes;

        if (document->on_lbutton_down (x, y, x, y, redrawBoxes))
            renderAndPaint();
    }

    void mouseUp(const MouseEvent& event)
    {
        if (dom == nullptr)
            return;

        auto document { dom->getDocument() };

        const int x { event.x + scrollX };
        const int y { event.y + scrollY };

        std::vector<litehtml::position> redrawBoxes;

        if (document->on_lbutton_up (x, y, x, y, redrawBoxes))
            renderAndPaint();
    }

    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
    {
        vScrollBar.mouseWheelMove (event, wheel);
        hScrollBar.mouseWheelMove (event, wheel);
    }

    void renderAndPaint()
    {
        render();
        self.repaint();
    }

    void followLink (const URL& url)
    {
        if (page != nullptr)
            page->followLink (url);
    }

    // WebPage::ViewClient
    void documentAboutToBeReloaded() override
    {
    }

    void documentLoaded() override
    {
        dom = page->getDOM();
        jassert (dom != nullptr);

        // Reset the scroll position as a new document has been loaded
        scrollX = 0;
        scrollY = 0;

        renderAndPaint();
    }

    // ScrollBar::Listener
    void scrollBarMoved (ScrollBar* scrollBar, double newRangeStart) override
    {
        if (scrollBar == &vScrollBar)
            scrollY = (int)newRangeStart;
        else if (scrollBar == &hScrollBar)
            scrollX = (int)newRangeStart;

        self.repaint();
    }

};

//==============================================================================

WebView::WebView()
    : d { std::make_unique<Impl>(*this) }
{
}

WebView::~WebView() = default;

void WebView::setPage(WebPage* page)
{
    d->setPage (page);
}

WebPage* WebView::getPage()
{
    return d->page;
}

void WebView::paint (Graphics& g)
{
    d->paint (g);
}

void WebView::resized()
{
    d->render();
}

void WebView::mouseMove (const MouseEvent& event)
{
    d->mouseMove (event);
}

void WebView::mouseDown(const MouseEvent& event)
{
    d->mouseDown (event);
}

void WebView::mouseUp(const MouseEvent& event)
{
    d->mouseUp (event);
}

void WebView::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    d->mouseWheelMove (event, wheel);
}

} // namespace juce_litehtml
