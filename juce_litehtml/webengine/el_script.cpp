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
}

void el_script::parse_attributes()
{
    if (const auto* src { get_attr ("src") })
    {
        auto& loader { context->getLoader() };
        const URL url (juceString (src));

        loader.loadAsync<String>(url, [self = std::weak_ptr<litehtml::element>(shared_from_this())](bool ok, const String& content) {
            if (ok)
            {
                if (auto el { self.lock() })
                {
                    if (auto* el_this { dynamic_cast<juce_litehtml::el_script*>(el.get()) })
                        el_this->setScript (content);
                }
            }
        });
    }
}

void el_script::set_data (const litehtml::tchar_t* data)
{
    if (data != nullptr)
        script = String::fromUTF8 (data);
}

} // namespace juce_litehtml
