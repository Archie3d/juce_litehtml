namespace juce_litehtml {

el_script::el_script (const litehtml::document::ptr& doc)
    : html_tag (doc),
      script{},
      context{ dynamic_cast<WebContext*>(doc->context()) }
{
}

void el_script::setScript (const String& s)
{
    script = s;
    //context->eval(to_tstring(script));
}

void el_script::parse_attributes()
{
    if (const auto* src { get_attr ("src") })
    {
        document::ptr doc = get_document();

        litehtml::tstring script;
        doc->container()->import_script (script, src);

        if (!script.empty())
            setScript (juceString (script));
    }
}

void el_script::set_data (const litehtml::tchar_t* data)
{
    if (data != nullptr)
        script = String::fromUTF8 (data);
}

} // namespace juce_litehtml
