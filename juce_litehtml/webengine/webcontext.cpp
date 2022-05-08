namespace juce_litehtml {

WebContext::WebContext()
    : loader(),
      jsRuntime(),
      jsContext (jsRuntime)
{
    // Load default embedded stylesheet
    load_master_stylesheet (juce_litehtml_master_css);
}

WebContext::~WebContext() = default;

litehtml::element::ptr WebContext::create_element (const litehtml::tchar_t* tag_name,
                                                   const litehtml::string_map& attributes,
                                                   const litehtml::document::ptr& doc)
{
    const auto tag { juceString (tag_name).toLowerCase() };

    litehtml::element::ptr element{};

    if (tag == "script")
        element = std::make_shared<el_script>(doc);

    return element;
}

} // namespace juce_litehtml