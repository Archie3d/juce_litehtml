namespace juce_litehtml::js {

juce::var JSValueToVar (JSContext* ctx, JSValueConst val);
JSValue varToJSValue (JSContext* ctx, const juce::var& val);

class Function final : public juce::ReferenceCountedObject
{
public:
    Function (JSContext* ctx, JSValueConst val);
    ~Function();

    template <typename ...Ts>
    juce::var call (Ts&&... args)
    {
        std::vector<JSValue> jsArgs;

        ([this, &jsArgs](auto& arg) {
            juce::var v (arg);
            jsArgs.push_back (varToJSValue (jsCtx, v));
        } (args), ...);

        auto ret {JS_Call (jsCtx, jsFunc, JS_NULL, int (jsArgs.size()), jsArgs.data())};
        juce::var retVar {JSValueToVar (jsCtx, ret)};
        JS_FreeValue (jsCtx, ret);

        for (auto& jsArg : jsArgs)
            JS_FreeValue (jsCtx, jsArg);

        return retVar;
    }

    template <typename ...Ts>
    juce::var callThis (JSValue thisObj, Ts&&... args)
    {
        std::vector<JSValue> jsArgs;

        ([this, &jsArgs](auto& arg) {
            juce::var v (arg);
            jsArgs.push_back (varToJSValue (jsCtx, v));
        } (args), ...);

        auto ret {JS_Call (jsCtx, jsFunc, thisObj, int (jsArgs.size()), jsArgs.data())};
        juce::var retVar {JSValueToVar (jsCtx, ret)};
        JS_FreeValue (jsCtx, ret);

        for (auto& jsArg : jsArgs)
            JS_FreeValue (jsCtx, jsArg);

        return retVar;
    }

    JSValue getJSValue() noexcept { return jsFunc; }

private:
    JSContext* jsCtx;
    JSValue jsFunc;
};

/** Wrapper around JSValue.

    This class incapsulates JSValue and provide
    some helpful methods to interact with it.
*/
class Value
{
public:

    Value (JSContext* ctx = nullptr);
    Value (JSContext* ctx, const JSValue& value);
    Value (const Value& other);
    Value (Value&& other) noexcept;
    Value& operator =(const Value& other);
    Value& operator =(int x);
    Value& operator =(int64_t x);
    Value& operator =(bool x);
    Value& operator =(float x);
    Value& operator =(double x);
    Value& operator =(const juce::String& str);
    ~Value();

    void reset();
    const JSValue& getJSValue() const noexcept { return jsValue; }
    JSValue releaseJSValue();

    bool isVoid() const;
    bool isUndefined() const;
    bool isError() const;
    bool isException() const;
    bool isInt() const;
    bool isBool() const;
    bool isDouble() const;
    bool isString() const;
    bool isArray() const;
    bool isFunction() const;
    bool isObject() const;

    operator int() const;
    operator int64_t() const;
    operator bool() const;
    operator float() const;
    operator double() const;
    operator juce::String() const;

    Value operator[](int index) const;
    Value operator[](juce::StringRef name) const;

    void setProperty (juce::StringRef name, const Value& value);
    Value getProperty (juce::StringRef name);

    juce::var toVar() const;

private:
    JSContext* jsContext;
    JSValue jsValue;
};

} // namespace juce_litehtml::ui
