namespace juce_litehtml {

el_input::el_input (const litehtml::document::ptr& doc)
    : html_tag (doc)
{
}

void el_input::parse_attributes()
{
    DBG("<input>");
    for (auto&& [key, value] : m_attrs)
    {
        DBG("    " << String(key) << " = " << String(value));
    }
}

int el_input::render (int x, int y, int max_width, bool second_pass)
{
    if (textEditor == nullptr)
    {
        textEditor = std::make_unique<TextEditor>();

        if (auto* ctx { dynamic_cast<WebContext*>(get_document()->context()) })
        {
            if (auto* view { ctx->getView() })
                view->addAndMakeVisible(textEditor.get());
        }
    }

    return html_tag::render(x, y, max_width, second_pass);
}

void el_input::draw (litehtml::uint_ptr hdc, int x, int y, const litehtml::position* clip)
{
    if (textEditor != nullptr)
    {
        position pos { m_pos };
        pos.x += x;
        pos.y += y;

        textEditor->setBounds(pos.x, pos.y, pos.width, pos.height);
    }
}

} // namespace juce_litehtml
