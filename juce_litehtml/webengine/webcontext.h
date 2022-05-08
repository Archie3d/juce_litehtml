namespace juce_litehtml {

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

    /** Returns the JavaScript runtime. */
    js::Runtime& getJavaScriptRuntime() noexcept { return jsRuntime; }

    /** Returns the JavaScript context. */
    js::Context& getJavaScriptContext() noexcept { return jsContext; }

    /** Register a DOM element with the JS context.

        This will register a DOM element class with the
        JS context. This is required to track Element objects
        and distinguish them from other objects injected into
        the JS context.
     */
    template <class T>
    void registerElement (juce::StringRef name)
    {
        jsContext.registerClass<T> (name);
        elementClassIDs.insert (T::jsClassID);
    }

    /** Tells whether the JS class ID is registered for a DOM element. */
    bool isRegisteredElementClassID (JSClassID id) const;

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

    js::Runtime jsRuntime;
    js::Context jsContext;

    std::set<JSClassID> elementClassIDs;
};

} // namespace juce_litehtml
