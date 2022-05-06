#include "litehtml.h"

namespace juce_litehtml {

using namespace litehtml;

static Colour webColour (const web_color& c)
{
    return Colour (c.red, c.green, c.blue, c.alpha);
}

static String tcharString (const tchar_t* s)
{
#ifdef LITEHTML_UTF8
    return String::fromUTF8 (s);
#else
    return String (s);
#endif
}

static String tstringString (const tstring& s)
{
    return String (s.c_str());
}

//==============================================================================

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
        String fontName { tcharString (faceName) };
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
            return font->getStringWidth (tcharString (text));

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
            g->drawText (tcharString (text), rect, Justification::left);
        }
    }

    int pt_to_px (int pt) const override
    {
        const auto& displays { Desktop::getInstance().getDisplays() };
        auto& display = displays.getMainDisplay();
        return (int) (display.dpi * pt / 72.0 / display.scale);
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
            String url (tcharString (src));

            auto image { loader->loadImage (url) };

            if (! image.isNull())
            {
                // @todo Add image to cache (to cache its size);
            }

            if (redraw_on_ready && !image.isNull())
            {
                // @todo Handle asynchronous load.
            }
        }
    }

    void get_image_size (const tchar_t* src, const tchar_t* baseurl, litehtml::size& sz) override
    {
        if (auto* loader { getLoader() })
        {
            String url (tcharString (src));
            auto image { loader->loadImage (url) };

            if (! image.isNull())
            {
                sz.width = image.getWidth();
                sz.height = image.getHeight();
            }
            else
            {
                sz.width = 0;
                sz.height = 0;
            }
        }
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
                auto image { loader->loadImage (tstringString (bg.image)) };

                if (!image.isNull())
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

    void draw_borders (uint_ptr hdc, const borders& borders, const position& draw_pos, bool root) override
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
            const String sUrl (tcharString (base_url));
            loader->setBaseURL (sUrl);
        }
    }

    void link (const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override
    {
        // @todo
    }

    void on_anchor_click (const tchar_t* url, const litehtml::element::ptr& el) override
    {
        String sUrl (tcharString (url));
        sUrl = sUrl.trim();

        // @todo Navigate to the clicked URL
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
        const String cursorName { tcharString (cursor).toLowerCase().trim() };

        if (const auto it { cursorTypes.find (cursorName) }; it != cursorTypes.end())
            cur = it->second;

        // @todo Updaye mouse cursor
        // webView.setMouseCursor (cur);
    }

    void transform_text (litehtml::tstring& text, litehtml::text_transform tt) override
    {
        String str { tcharString (text.c_str()) };

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

    void import_css (tstring &text, const tstring &url, tstring &baseurl) override
    {
        // @todo Load css
        jassertfalse;
    }

    void set_clip (const position &pos, const border_radiuses &bdr_radius, bool valid_x, bool valid_y) override
    {
        // @todo
        jassertfalse;
    }

    void del_clip() override
    {
        // @todo
        jassertfalse;
    }

    void get_client_rect (position& client) const override
    {
        client.x = 0;
        client.y = 0;
        client.width = webView.getWidth();
        client.height = webView.getHeight();
    }

    litehtml::element::ptr create_element (const tchar_t *tag_name,
                                           const string_map &attributes,
                                           const litehtml::document::ptr& doc) override
    {
        // @todo Create element
        return nullptr;
    }

    void get_media_features(litehtml::media_features &media) const override
    {
        const auto& displays { Desktop::getInstance().getDisplays() };
        auto& display { displays.getMainDisplay() };

        media.type = litehtml::media_type_screen;
        media.width = display.userArea.getWidth();
        media.height = display.userArea.getHeight();
        media.color = 8;
        media.monochrome = 0;
        media.color_index = 256;
        media.resolution = (int) display.dpi;
        media.device_width = display.totalArea.getWidth();
        media.device_height = display.totalArea.getHeight();
    }

    void get_language (tstring &language, tstring &culture) const override
    {
        language = _t("en");
        culture = _t("en-GB");
    }

    tstring resolve_color (const tstring &color) const override
    {
        String colorName { tcharString (color.c_str()) };

        Colour c { Colours::findColourForName (colorName, Colour::fromString (colorName)) };
        String strColour = String ("#") + c.toDisplayString (true);

    #ifdef LITEHTML_UTF8
        return litehtml::tstring (strColour.toStdString());
    #else
        return litehtml::tstring (strColour.toWideCharPointer());
    #endif
    }

private:

    WebLoader* getLoader()
    {
        if (auto* page { webView.getPage() })
            return &page->getLoader();

        return nullptr;
    }

    WebView& webView;
};

//==============================================================================

struct WebView::Impl : public WebPage::Listener
{
    WebView& self;
    Renderer renderer;
    WebPage* page { nullptr };
    litehtml::document::ptr document { nullptr };

    Impl (WebView& wv)
        : self { wv },
          renderer (wv)
    {
    }

    void setPage (WebPage* newPage)
    {
        if (page != nullptr)
        {
            page->removeListener (this);
            page->setRenderer (nullptr);
        }

        page = newPage;

        if (page != nullptr)
        {
            page->addListener (this);
            page->setRenderer (&renderer);
            document = page->getDocument();
        }
    }

    void render()
    {
        if (document == nullptr)
            return;

        const auto width { self.getWidth() };
        document->render (width, litehtml::render_all);
    }

    void paint (Graphics& g)
    {
        // @todo Get the colour from <body> style
        g.fillAll (Colours::white);

        if (document == nullptr)
            return;

        litehtml::position clip (0, 0, self.getWidth(), self.getHeight());

        // @todo Use scroll position for drawing
        document->draw ((litehtml::uint_ptr)&g, 0, 0, &clip);
    }

    // WebPage::Listener
    void documentLoaded()
    {
        document = page->getDocument();
        self.resized();
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

} // namespace juce_litehtml
