namespace juce_litehtml {

class el_input : public litehtml::html_tag
{
public:
    el_input (const litehtml::document::ptr& doc);

    void parse_attributes() override;
    int render (int x, int y, int max_width, bool second_pass = false) override;
    void draw (litehtml::uint_ptr hdc, int x, int y, const litehtml::position* clip) override;

private:
    std::unique_ptr<juce::TextEditor> textEditor { nullptr };
};

} // namespace juce_litehtml
