namespace juce_litehtml {

/** Custom <script> tag. */
class el_script :public litehtml::html_tag
{
public:
    el_script (const litehtml::document::ptr& doc);

    void setScript (const juce::String& s);

    void parse_attributes() override;
    void set_data (const litehtml::tchar_t* data) override;

private:

    juce::String script;
    WebContext* context{};
};

} // namespace juce_litehtml
