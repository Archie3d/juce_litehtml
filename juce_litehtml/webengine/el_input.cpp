namespace juce_litehtml {

el_input::el_input (const litehtml::document::ptr& doc)
    : html_tag (doc)
{
}

void el_input::parse_attributes()
{
    const String type { get_attr("type") };

    const auto colour = webColour (get_color ("color", true, get_document()->get_def_color()));
    auto bgColour = webColour (get_color ("background-color", true, litehtml::web_color(255, 255, 255, 255)));

    if (auto* bg { get_background(true) })
        bgColour = webColour (bg->m_color);

    if (type.isEmpty() || type == "text" || type == "search")
    {
        textEditor = std::make_unique<TextEditor>();
        textEditor->setText (get_attr("value"));

        textEditor->setColour (TextEditor::textColourId, colour);
        textEditor->setColour (TextEditor::backgroundColourId, bgColour);

        component = textEditor.get();
    }
    else if (type == "submit")
    {
        textButton = std::make_unique<TextButton>(get_attr("value", "Submit"));

        textButton->setColour (TextButton::textColourOnId, colour);
        textButton->setColour (TextButton::buttonColourId, bgColour);

        component = textButton.get();
    }
    else if (type == "checkbox")
    {
        toggleButton = std::make_unique<ToggleButton>(get_attr("value"));

        component = toggleButton.get();
    }

    if (component != nullptr)
    {
        if (auto* ctx { dynamic_cast<WebContext*>(get_document()->context()) })
        {
            if (auto* view { ctx->getView() })
                view->addAndMakeVisible(component);
        }
    }

}

int el_input::render (int x, int y, int max_width, bool second_pass)
{
    return html_tag::render(x, y, max_width, second_pass);
}

void el_input::draw (litehtml::uint_ptr hdc, int x, int y, const litehtml::position* clip)
{
    if (component != nullptr)
    {
        position pos { m_pos };
        pos.x += x;
        pos.y += y;

        component->setBounds(pos.x, pos.y, pos.width, pos.height);
    }
}

} // namespace juce_litehtml
