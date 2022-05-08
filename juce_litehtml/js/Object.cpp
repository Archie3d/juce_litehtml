namespace juce_litehtml::js {

ObjectBase::ObjectBase()
    : ownership { Ownership::Native }
{
}

void ObjectBase::registerMethod (JSContext* jsContext, JSValue proto, StringRef name, JSCFunction func)
{
    JS_SetPropertyStr (jsContext, proto, name.text, JS_NewCFunction (jsContext, func, name, 0));
}

void ObjectBase::registerConstructor (JSContext* jsContext, JSValue proto, StringRef name, JSCFunction func, int numArgs)
{
    JSValue clazz = JS_NewCFunction2 (jsContext, func, name.text, numArgs, JS_CFUNC_constructor, 0);

    auto global {JS_GetGlobalObject (jsContext)};
    JS_DefinePropertyValueStr (jsContext, global, name.text, JS_DupValue (jsContext, clazz),
                               JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetConstructor (jsContext, clazz, proto);
    JS_FreeValue (jsContext, global);
    JS_FreeValue (jsContext, clazz);
}

void ObjectBase::registerProperty (JSContext* jsContext, JSValue proto, StringRef name, Getter getter, Setter setter)
{
    auto aProperty = JS_NewAtom(jsContext, name.text);
    int flags = JS_PROP_CONFIGURABLE;

    if (getter != nullptr)
        flags |= JS_PROP_ENUMERABLE;

    if (setter != nullptr)
        flags |= JS_PROP_WRITABLE;

    auto get = getter != nullptr ? JS_NewCFunction2(jsContext, (JSCFunction*)getter, "<get>", 0, JS_CFUNC_getter, 0) : JS_UNDEFINED;
    auto set = setter != nullptr ? JS_NewCFunction2(jsContext, (JSCFunction*)setter, "<set>", 1, JS_CFUNC_setter, 0) : JS_UNDEFINED;

    JS_DefinePropertyGetSet(jsContext, proto, aProperty,
        get,
        set,
        flags
    );
    JS_FreeAtom(jsContext, aProperty);
}


} // namespace juce_litehtml::js
