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

    /** Element of the DOM.

        This class is a wrapper over the litehtml element
        that exposes its scriptable capabilities.
     */
    class Element : public js::Object<Element>
    {
    public:
        Element (litehtml::element::ptr el, WebDOM::Ptr d);
        ~Element();

        /** Returns the DOM this element is associated with. */
        WebDOM::Ptr getDOM() { return dom; }

        /** Returns element's tag name. */
        juce::String getTagName() const;

        /** Returns element's id attribute. */
        juce::String getId() const;

        /** Assign element's id attribute. */
        void setId (const juce::String& id);

        /** Returns parent element.

            This returns a new wrapper to the parent element
            of this one. Each call of this method creates a
            new wrapper to the same parent element.
         */
        std::unique_ptr<WebDOM::Element> getParentElement() const;

        /** Append a child element to this one. */
        void appendChild (Element& child);

        static void registerJSProto (JSContext* jsContext, JSValue proto);

    private:
        litehtml::element::ptr element;
        WebDOM::Ptr dom;
    };

    //==========================================================================

    WebDOM() = delete;
    WebDOM (WebContext& ctx, litehtml::document::ptr doc);
    virtual ~WebDOM();

    WebContext& getContext() { return context; }

    /** Returns inner litehtml document. */
    litehtml::document::ptr getDocument() { return document; }

    /** Create a DOM Element.

        This method creates an element of a given tag type.
        The element will not be attached to the DOM.
     */
    std::unique_ptr<WebDOM::Element> createElement (const juce::String& tagName);

    static void registerJSProto (JSContext* jsContext, JSValue proto);

private:

    WebContext& context;
    litehtml::document::ptr document { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebDOM);
};

} // namespace juce_litehtml
