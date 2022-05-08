namespace juce_litehtml::js {

/** Objects ownership.

    Freshly created objects exposed to JavaScript environment
    have a Native ownership by default. When moved to the Script
    ownership the object will be deleted by the JavaScript garbage
    collector.
*/
enum class Ownership
{
    Native, // Object is owned by the native context.
    Script  // Object is owned by the JS context.
};

/** Opaque pointer wrapper.

    This is a helper structure that wraps a native pointer
    along with its validity flag. When an object gets deleted
    it will set the validity flag to false. This will indicate that
    the embedded native pointer is no longer valid and cannot not be
    used or deleted.
*/
struct OpaqueReference
{
    void* ptr;
    std::shared_ptr<bool> valid;
};

/** JavaScript runtime.

    This is a wrapper of the JSRuntime with some helper functions.
*/
class Runtime final
{
public:

    /** JavaScript modules loader interface. */
    class ModuleLoader
    {
    public:
        virtual ~ModuleLoader() = default;
        virtual bool canLoadModule (const juce::String& moduleName) = 0;
        virtual juce::String loadModule (const juce::String& moduleName) = 0;
    };

    Runtime();
    ~Runtime();

    JSRuntime* getJSRuntime() noexcept { return jsRuntime.get(); }

        /** Register a JavaScript class with this runtime.

        This method is called by the @ref Context::registerClass and normally
        should not be called directly. Registered class will attach a custom
        destructor that track the object scope (native or script) and deletes
        the object if it is owned by JavaScript and gets reclaimed by the garbage collector.

        @note The template class must be inherited from js::Object.
    */
    template<class T>
    void registerClass (juce::StringRef name)
    {
        if (T::jsClassID == 0)
            JS_NewClassID (&T::jsClassID);

        auto rt {jsRuntime.get()};

        if (! JS_IsRegisteredClass (rt, T::jsClassID))
        {
            const JSClassDef def {
                name.text,
                [](JSRuntime*, JSValue value) {
                    if (auto* ref { static_cast<OpaqueReference*>(JS_GetOpaque (value, T::jsClassID)) })
                    {
                        T* obj { static_cast<T*>(ref->ptr) };

                        if (*ref->valid && obj->getOwnership() == Ownership::Script)
                        {
                            // If the script owns this object we delete it here,
                            // unless it has been already deleted, which is indicated
                            // by the validity flag stored in the reference.
                            delete obj;
                        }

                        delete ref;
                    }
                },
                nullptr, // JSClassGCMark
                nullptr, // JSClassCall
                nullptr, // JSClassExoticMethods
            };

            [[maybe_unused]] const auto res { JS_NewClass (rt, T::jsClassID, &def) };
            jassert (res == 0);
        }
    }

private:
    std::unique_ptr<JSRuntime, void(*)(JSRuntime*)> jsRuntime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Runtime)
};

} // namespace juce_litehtml::js
