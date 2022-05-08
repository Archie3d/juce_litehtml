namespace juce_litehtml {

WebDOM::Element::Element (litehtml::element::ptr el, WebDOM::Ptr d)
    : element { el },
      dom { d }
{
}

WebDOM::Element::~Element() = default;

String WebDOM::Element::getId() const
{
    if (auto el { element.lock() })
        return juceString (el->get_attr("id"));

    return {};
}

namespace Element {

static JSValue js_getId (JSContext* ctx, JSValueConst self, int argc, JSValueConst* args)
{
    if (auto* element { js::Object<WebDOM::Element>::getNativeObject (self) })
    {
        const auto id { element->getId() };
        return JS_NewStringLen (ctx, id.toRawUTF8(), id.length());
    }

    return JS_NULL;
}

} // namespace Element

void WebDOM::Element::registerJSProto (JSContext* jsContext, JSValue proto)
{
}

//==============================================================================

WebDOM::WebDOM (WebContext& ctx, litehtml::document::ptr doc)
    : context { ctx },
      document { std::move (doc) }
{
}

WebDOM::~WebDOM() = default;

namespace Document {

static JSValue js_createElement (JSContext* ctx, JSValueConst self, int argc, JSValueConst* args)
{
    return JS_NULL;
}

} // namespace Document

void WebDOM::registerJSProto (JSContext* jsContext, JSValue proto)
{
    registerMethod (jsContext, proto, "createElement", &Document::js_createElement);
}


} // namespace juce_litehtml
