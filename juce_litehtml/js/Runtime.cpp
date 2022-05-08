namespace juce_litehtml::js {

Runtime::Runtime()
    : jsRuntime (JS_NewRuntime(), JS_FreeRuntime)
{
}

Runtime::~Runtime() = default;

} // namespace juce_litehtml::js
