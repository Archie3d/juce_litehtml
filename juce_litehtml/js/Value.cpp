namespace juce_litehtml::js {

var JSValueToVar (JSContext* ctx, JSValueConst val)
{
    const auto tag {JS_VALUE_GET_TAG (val)};

    switch (tag)
    {
        case JS_TAG_NULL:
            return {};
        case JS_TAG_UNDEFINED:
            return var::undefined();
        case JS_TAG_BOOL:
            return JS_ToBool (ctx, val) != 0;
        case JS_TAG_INT:
        {
            int64_t x;
            JS_ToInt64 (ctx, &x, val);
            return x;
        }
        case JS_TAG_FLOAT64:
        {
            double x;
            JS_ToFloat64 (ctx, &x, val);
            return x;
        }
        case JS_TAG_STRING:
        {
            if (const char* s { JS_ToCString (ctx, val) })
            {
                const auto str {juce::String::fromUTF8 (s)};
                JS_FreeCString (ctx, s);
                return str;
            }
            break;
        }
        case JS_TAG_OBJECT:
        {
            if (JS_IsFunction (ctx, val))
            {
                return var (new Function (ctx, val));
            }
            else if (JS_IsArray (ctx, val))
            {
                Array<var> arr;
                int length{};

                JS_ToInt32 (ctx, &length, JS_GetPropertyStr (ctx, val, "length"));

                for (int i = 0; i < length; ++i)
                {
                    auto item { JS_GetPropertyUint32 (ctx, val, static_cast<uint32_t>(i)) };
                    arr.add (JSValueToVar (ctx, item));
                    JS_FreeValue (ctx, item);
                }

                return arr;
            }
            else
            {
                auto* obj {new DynamicObject()};

                JSPropertyEnum* props;
                uint32_t numProps;

                if (JS_GetOwnPropertyNames (ctx, &props, &numProps, val, JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) == 0)
                {
                    for (uint32_t i = 0; i < numProps; ++i)
                    {
                        auto propKey { JS_AtomToCString (ctx, props[i].atom) };
                        auto propVal { JS_GetProperty (ctx, val, props[i].atom) };
                        obj->setProperty (propKey, JSValueToVar (ctx, propVal));

                        JS_FreeValue (ctx, propVal);
                        JS_FreeCString (ctx, propKey);
                    }

                    js_free_prop_enum (ctx, props, numProps);
                }

                return obj;
            }
        }
        default:
            break;
    }


    return {};
}

//==============================================================================

JSValue varToJSValue (JSContext* ctx, const var& val)
{
    if (val.isVoid())
        return JS_NULL;
    if (val.isUndefined())
        return JS_UNDEFINED;
    if (val.isBool())
        return (bool)val ? JS_TRUE : JS_FALSE;
    if (val.isInt())
        return JS_NewInt32 (ctx, int32_t (val));
    if (val.isInt64())
        return JS_NewInt64 (ctx, int64_t (val));
    if (val.isDouble())
        return JS_NewFloat64 (ctx, double (val));
    if (val.isString())
        return JS_NewString (ctx, val.toString().toRawUTF8());
    if (val.isArray())
    {
        auto jsArr {JS_NewArray (ctx)};
        int i{};

        for (auto& item : *val.getArray())
            JS_SetPropertyUint32 (ctx, jsArr, i++, varToJSValue (ctx, item));

        return jsArr;
    }

    if (val.isObject())
    {
        if (auto* func { dynamic_cast<Function*>(val.getObject()) })
        {
            return JS_DupValue (ctx, func->getJSValue());
        }

        if (auto* obj {val.getDynamicObject()})
        {
            auto jsObj {JS_NewObject (ctx)};

            for (auto& prop : obj->getProperties())
                JS_SetPropertyStr (ctx, jsObj, prop.name.toString().toRawUTF8(), varToJSValue (ctx, prop.value));

            return jsObj;
        }
    }

    return JS_UNDEFINED;
}

//==============================================================================

Function::Function (JSContext* ctx, JSValueConst val)
    : jsCtx {ctx}
    , jsFunc {JS_DupValue (ctx, val)}
{
    jassert (JS_IsFunction (ctx, val));
}

Function::~Function()
{
    JS_FreeValue (jsCtx, jsFunc);
}

//==============================================================================

Value::Value (JSContext* ctx)
    : jsContext {ctx}
    , jsValue (JS_UNINITIALIZED)
{
}

Value::Value (JSContext* ctx, const JSValue& value)
    : jsContext {ctx}
    , jsValue {value}
{
}

Value::Value (const Value& other)
    : jsContext {other.jsContext}
    , jsValue {JS_DupValue (jsContext, other.jsValue)}
{
}

Value::Value (Value&& other) noexcept
    : jsContext {other.jsContext}
    , jsValue {other.releaseJSValue()}
{
}

Value& Value::operator =(const Value& other)
{
    if (this != &other)
    {
        reset();
        jsContext = other.jsContext;

        if (jsContext != nullptr && ! other.isVoid())
            jsValue = JS_DupValue (jsContext, other.jsValue);
        else
            jsValue = JS_UNINITIALIZED;
    }

    return *this;
}

Value& Value::operator =(int x)
{
    reset();

    if (jsContext != nullptr)
        jsValue = JS_NewInt32 (jsContext, x);

    return *this;
}

Value& Value::operator =(int64_t x)
{
    reset();

    if (jsContext != nullptr)
        jsValue = JS_NewInt64 (jsContext, x);

    return *this;
}

Value& Value::operator =(bool x)
{
    reset();

    if (jsContext != nullptr)
        jsValue = JS_NewBool (jsContext, x);

    return *this;
}

Value& Value::operator =(float x)
{
    reset();

    if (jsContext != nullptr)
        jsValue = JS_NewFloat64 (jsContext, double (x));

    return *this;
}

Value& Value::operator =(double x)
{
    reset();

    if (jsContext != nullptr)
        jsValue = JS_NewFloat64 (jsContext, x);

    return *this;
}

Value& Value::operator =(const String& str)
{
    reset();

    if (jsContext != nullptr)
        jsValue = JS_NewString (jsContext, str.toRawUTF8());

    return *this;
}

Value::~Value()
{
    if (jsContext != nullptr && ! isVoid())
        JS_FreeValue (jsContext, jsValue);
}

void Value::reset()
{
    if (jsContext != nullptr && ! isVoid())
    {
        JS_FreeValue (jsContext, jsValue);
        jsValue = JS_UNINITIALIZED;
    }
}

JSValue Value::releaseJSValue()
{
    auto tmp {jsValue};
    reset();
    return tmp;
}

bool Value::isVoid() const { return JS_VALUE_GET_TAG (jsValue) == JS_TAG_UNINITIALIZED; }
bool Value::isUndefined() const { return JS_VALUE_GET_TAG (jsValue) == JS_TAG_UNDEFINED; }
bool Value::isError() const { return JS_IsError (jsContext, jsValue); }
bool Value::isException() const { return JS_IsException (jsValue); }
bool Value::isInt() const { return JS_VALUE_GET_TAG (jsValue) == JS_TAG_INT; }
bool Value::isBool() const { return JS_VALUE_GET_TAG (jsValue) == JS_TAG_BOOL; }
bool Value::isDouble() const { return JS_VALUE_GET_TAG (jsValue) == JS_TAG_FLOAT64; }
bool Value::isString() const { return JS_VALUE_GET_TAG (jsValue) == JS_TAG_STRING; }

bool Value::isArray() const
{
    return JS_VALUE_GET_TAG (jsValue) == JS_TAG_OBJECT
        && JS_IsArray (jsContext, jsValue);
}

bool Value::isFunction() const
{
    return JS_VALUE_GET_TAG (jsValue) == JS_TAG_OBJECT
        && JS_IsFunction (jsContext, jsValue);
}

bool Value::isObject() const
{
    if (JS_VALUE_GET_TAG (jsValue) != JS_TAG_OBJECT)
        return false;

    if (JS_IsArray (jsContext, jsValue))
        return false;

    if (JS_IsFunction (jsContext, jsValue))
        return false;

    return true;
}

Value::operator int() const
{
    if (isInt()) {
        int x;
        JS_ToInt32 (jsContext, &x, jsValue);
        return x;
    }

    if (isDouble()) {
        double x;
        JS_ToFloat64 (jsContext, &x, jsValue);
        return int(x);
    }

    return {};
}

Value::operator int64_t() const
{
    if (isInt()) {
        juce::int64 x;
        JS_ToInt64 (jsContext, &x, jsValue);
        return x;
    }

    if (isDouble()) {
        double x;
        JS_ToFloat64 (jsContext, &x, jsValue);
        return juce::int64(x);
    }

    return {};
}

Value::operator bool() const
{
    return (1 == JS_ToBool (jsContext, jsValue));
}

Value::operator float() const
{
    if (isDouble()) {
        double x;
        JS_ToFloat64 (jsContext, &x, jsValue);
        return float (x);
    } else if (isInt()) {
        int64_t x;
        JS_ToInt64 (jsContext, &x, jsValue);
        return float (x);
    }

    return {};
}

Value::operator double() const
{
    if (isDouble()) {
        double x;
        JS_ToFloat64 (jsContext, &x, jsValue);
        return x;
    } else if (isInt()) {
        int64_t x;
        JS_ToInt64 (jsContext, &x, jsValue);
        return double (x);
    }

    return {};
}
Value::operator String() const
{
    if (isString())
    {
        if (const char* s = JS_ToCString (jsContext, jsValue))
            return juce::String::fromUTF8 (s);
    }

    return {};
}

Value Value::operator[](int index) const
{
    if (jsContext != nullptr && isArray() && index >= 0)
    {
        auto val {JS_GetPropertyUint32 (jsContext, jsValue, (uint32_t) index)};
        return Value (jsContext, val);
    }

    return {};
}

Value Value::operator[](StringRef name) const
{
    if (jsContext == nullptr || ! isObject())
        return {};

    return Value (jsContext, JS_GetPropertyStr (jsContext, jsValue, name));
}

void Value::setProperty (StringRef name, const Value& value)
{
    if (isObject())
        JS_SetPropertyStr (jsContext, jsValue, name, JS_DupValue (jsContext, value.getJSValue()));
}

Value Value::getProperty (StringRef name)
{
    if (jsContext == nullptr || ! isObject())
        return {};

    return Value (jsContext, JS_GetPropertyStr (jsContext, jsValue, name));
}

var Value::toVar() const
{
    return JSValueToVar (jsContext, jsValue);
}

} // namespace juce_litehtml::js
