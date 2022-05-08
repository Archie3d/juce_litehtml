namespace juce_litehtml {

WebDOM::Element::Element (litehtml::element::ptr el, WebDOM::Ptr d)
    : element { el },
      dom { std::move (d) }
{
}

WebDOM::Element::~Element() = default;

String WebDOM::Element::getTagName() const
{
    if (element != nullptr)
        return juceString (element->get_tagName());

    return {};
}

String WebDOM::Element::getId() const
{
    if (element != nullptr)
        return juceString (element->get_attr("id"));

    return {};
}

void WebDOM::Element::setId (const String& id)
{
    if (element != nullptr )
    {
        element->set_attr ("id", to_tchar (id));

        // @todo Notify the DOM change
    }
}

std::unique_ptr<WebDOM::Element> WebDOM::Element::getParentElement() const
{
    if (element != nullptr && dom != nullptr)
    {
        if (auto parent { element->get_parent() })
            return std::make_unique<WebDOM::Element>(parent, dom);
    }

    return nullptr;
}

void WebDOM::Element::appendChild (Element& child)
{
    if (element != nullptr && dom != nullptr)
    {
        if (auto childEl { child.element })
        {
            element->appendChild (childEl);
            childEl->apply_stylesheet(dom->getDocument()->context()->master_css());
            childEl->apply_stylesheet (dom->getDocument()->get_styles());
            childEl->parse_attributes();
            childEl->parse_styles();
            childEl->init();

            // @todo Notify the DOM change
        }
    }
}

//----------------------------------------------------------

static JSValue js_Element_getTagName (JSContext* ctx, JSValueConst self)
{
    if (auto* element { js::Object<WebDOM::Element>::getNativeObject (self) })
    {
        const auto id { element->getTagName() };
        return JS_NewStringLen (ctx, id.toRawUTF8(), id.length());
    }

    return JS_NULL;
}


static JSValue js_Element_getId (JSContext* ctx, JSValueConst self)
{
    if (auto* element { js::Object<WebDOM::Element>::getNativeObject (self) })
    {
        const auto id { element->getId() };
        return JS_NewStringLen (ctx, id.toRawUTF8(), id.length());
    }

    return JS_NULL;
}

static JSValue js_Element_setId (JSContext* ctx, JSValueConst self, JSValueConst val)
{
    if (auto* element { js::Object<WebDOM::Element>::getNativeObject (self) })
    {
        if (JS_IsString (val))
        {
            const auto* str { JS_ToCString (ctx, val) };
            element->setId (juce::String::fromUTF8 (str));
            JS_FreeCString (ctx, str);
        }
    }

    return JS_UNDEFINED;
}

static JSValue js_Element_appendChild (JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc != 1)
        return JS_UNDEFINED;

    if (auto* element { js::Object<WebDOM::Element>::getNativeObject (self) })
    {
        if (auto classID { js::ObjectBase::getClassID(arg[0]) }; classID != 0)
        {
            if (element->getDOM()->getContext().isRegisteredElementClassID (classID))
            {
                if (auto* ptr { js::Object<WebDOM::Element>::getNativeObject (arg[0]) })
                    element->appendChild (*ptr);
            }
        }
    }

    return JS_UNDEFINED;
}

//----------------------------------------------------------

void WebDOM::Element::registerJSProto (JSContext* jsContext, JSValue proto)
{
    registerProperty (jsContext, proto, "tagName", &js_Element_getTagName);
    registerProperty (jsContext, proto, "id",      &js_Element_getId, &js_Element_setId);

    registerMethod (jsContext, proto, "appendChild", &js_Element_appendChild);
}

//==============================================================================

WebDOM::WebDOM (WebContext& ctx, litehtml::document::ptr doc)
    : context { ctx },
      document { std::move (doc) }
{
}

WebDOM::~WebDOM() = default;

std::unique_ptr<WebDOM::Element> WebDOM::createElement (const juce::String& tagName)
{
    if (document == nullptr)
        return nullptr;

    litehtml::string_map attrs{};
    auto el { document->create_element (to_tchar (tagName), attrs) };
    return std::make_unique<WebDOM::Element> (el, shared_from_this());
}

//----------------------------------------------------------

static JSValue js_Document_createElement (JSContext* ctx, JSValueConst self, int argc, JSValueConst* args)
{
    if (argc != 1)
        return JS_NULL;

    if (auto* doc { js::Object<WebDOM>::getNativeObject (self) })
    {
        const auto* str { JS_ToCString (ctx, args[0]) };
        const auto tag {juce::String::fromUTF8 (str)};
        JS_FreeCString (ctx, str);

        auto element { doc->createElement (tag) };

        // Element created from within the script lives
        // inside the script's context.
        if (auto* jsObject { dynamic_cast<js::ObjectBase*>(element.get()) })
        {
            jsObject->setOwnership (js::Ownership::Script);
            element.release(); // strip unique_ptr
            return jsObject->toJSValue (ctx);
        }
    }

    return JS_NULL;
}

//---------------------------------------------------------

void WebDOM::registerJSProto (JSContext* jsContext, JSValue proto)
{
    registerMethod (jsContext, proto, "createElement", js_Document_createElement);
}

} // namespace juce_litehtml
