namespace juce_litehtml::js {

/** Base class for JavaScript exposed objects. */
class ObjectBase
{
public:
    using Getter = JSValue(*)(JSContext*, JSValueConst);
    using Setter = JSValue(*)(JSContext*, JSValueConst, JSValueConst);

    ObjectBase();
    virtual ~ObjectBase() = default;

    virtual JSValue toJSValue (JSContext* jsCtx) = 0;

    Ownership getOwnership() const noexcept { return ownership; }
    void setOwnership (Ownership ownedBy) noexcept { ownership = ownedBy; }

    /** Returns class ID of a given object. */
    static JSClassID getClassID (JSValue obj)
    {
        return JS_GetClassID (obj, nullptr);
    }

    /** Returns the embedded opaque pointer if it is valid. */
    static void* getOpaquePtr (JSValue obj)
    {
        void* opaque { nullptr };

        if (0 != JS_GetClassID (obj, &opaque) && opaque != nullptr)
        {
            OpaqueReference* ref { static_cast<OpaqueReference*>(opaque) };

            if (*ref->valid)
                return ref->ptr;
        }

        return nullptr;
    }

    /** Register JS prototype method. */
    static void registerMethod (JSContext* jsContext, JSValue proto, juce::StringRef name, JSCFunction func);

    /** Register JS prototype constructor. */
    static void registerConstructor (JSContext* jsContext, JSValue proto, juce::StringRef name, JSCFunction func, int numArgs = 0);

    /** Register JS protptype property. */
    static void registerProperty (JSContext* jsContext, JSValue proto, juce::StringRef, Getter getter, Setter setter = nullptr);

private:
    Ownership ownership { Ownership::Native };
};

//==============================================================================

/** Specialized scriptable object. */
template<class T>
class Object : public ObjectBase
{
public:

    static JSClassID jsClassID;

    Object()
        : reference {static_cast<T*>(this), std::make_unique<bool>(true)}
    {
        // We must cast this pointer to the actual class pointer,
        // otherwise when retriving the void* from JS context casting it
        // to T* will fail.
    }

    ~Object()
    {
        // Setting the validity flag to false here. This will
        // indicate to all other opaque references that embedded pointer
        // is not valid anymore.
        *reference.valid = false;
    }

    JSValue toJSValue (JSContext* jsCtx) override
    {
        jsContext = jsCtx;

        auto obj {JS_NewObjectClass (jsContext, jsClassID)};

        JS_SetOpaque (obj, new OpaqueReference(reference));

        return obj;
    }

    static T* getNativeObject (JSValue obj)
    {
        if (auto* opaque {JS_GetOpaque (obj, jsClassID)})
        {
            auto* ref {static_cast<OpaqueReference*>(opaque)};

            if (*ref->valid)
                return static_cast<T*>(ref->ptr);
        }

        return nullptr;
    }

    static JSValue getProto (JSContext* jsContext)
    {
        auto proto {JS_NewObject (jsContext)};

        T::registerJSProto (jsContext, proto);

        return proto;
    }

private:
    JSContext* jsContext {nullptr};

    OpaqueReference reference;
};

template<class T>
JSClassID Object<T>::jsClassID = 0;

} // namespace juce_litehtml::js
