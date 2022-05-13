namespace juce_litehtml {

class WebView;

/** Web context.

    This class assembles the loader and the
    JavaScript engine into a single context.
    A web context is constructed by a WebPage
    upon creation.

    @see WebPage
*/
class WebContext final : public litehtml::context
{
public:
    WebContext();
    ~WebContext();

    /** Returns the loader. */
    WebLoader& getLoader() { return loader; }

    void setView (WebView* vc) { view = vc; }
    WebView* getView() { return view; }

    /** Litehtml elements factory.

        This method is called by the document when creating
        each individual html elements. It is used to create
        custom element types.
     */
    litehtml::element::ptr create_element (const litehtml::tchar_t* tag_name,
                                           const litehtml::string_map& attributes,
                                           const litehtml::document::ptr& doc);

private:

    WebLoader loader;

    WebView* view { nullptr };
};

} // namespace juce_litehtml
