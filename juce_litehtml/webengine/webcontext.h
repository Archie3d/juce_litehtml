namespace juce_litehtml {

class WebContext final : public litehtml::context
{
public:
    WebContext();
    ~WebContext();

    WebLoader& getLoader() { return loader; }

    js::Runtime& getJavaScriptRuntime() noexcept { return jsRuntime; }
    js::Context& getJavaScriptContext() noexcept { return jsContext; }

    litehtml::element::ptr create_element (const litehtml::tchar_t* tag_name,
                                           const litehtml::string_map& attributes,
                                           const litehtml::document::ptr& doc);

private:

    WebLoader loader;

    js::Runtime jsRuntime;
    js::Context jsContext;
};

} // namespace juce_litehtml
