namespace juce_litehtml {

struct WebPage::Impl
{
    litehtml::document::ptr document { nullptr };
};

//==============================================================================

WebPage::WebPage()
    : d { std::make_unique<Impl>() }
{
}

WebPage::~WebPage() = default;

} // namespace juce_litehtml
