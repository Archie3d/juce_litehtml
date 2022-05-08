namespace juce_litehtml {

/** Document object model.

    This is a wripper around litehtml::document that
    provides abstraction layer on top of the document
    and its elements.
*/
class WebDOM : public std::enable_shared_from_this<WebDOM>,
               public js::Object<WebDOM>
{
public:

    using Ptr = std::shared_ptr<WebDOM>;
    using WeakPtr = std::weak_ptr<WebDOM>;

    //==========================================================================

    class Element : public js::Object<Element>
    {
    public:
        Element (litehtml::element::ptr el, WebDOM::Ptr d);
        ~Element();

        juce::String getId() const;

        static void registerJSProto (JSContext* jsContext, JSValue proto);

    private:
        litehtml::element::weak_ptr element;
        WebDOM::WeakPtr dom;
    };

    //==========================================================================

    WebDOM() = delete;
    WebDOM (WebContext& ctx, litehtml::document::ptr doc);
    virtual ~WebDOM();

    litehtml::document::ptr getDocument() { return document; }

    static void registerJSProto (JSContext* jsContext, JSValue proto);

private:

    WebContext& context;
    litehtml::document::ptr document { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebDOM);
};

} // namespace juce_litehtml
