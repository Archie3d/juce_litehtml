namespace juce_litehtml::js {

/** JavaScript context.

    This class holds the JavaScript interpreter context.
*/
class Context final
{
public:

    /** Helper class to handle async callbacks and timers. */
    class TimerPool final
    {
    public:
        TimerPool();
        ~TimerPool();

        void callAfterDelay (int milliseconds, std::function<void()> f);

    private:

        struct Invoker;

        void removeInvoker (Invoker* ptr);

        std::list<std::shared_ptr<Invoker>> invokers{};
    };

    //------------------------------------------------------

    Context (Runtime& rt);
    ~Context();

    Runtime& getRuntime() noexcept { return runtime; }
    JSRuntime* getJSRuntime() noexcept { return runtime.getJSRuntime(); }
    JSContext* getJSContext() noexcept { return jsContext.get(); }

    void reset();

    /** Register JavaScript class (prototype) with this context.

        This method creates a new JavaScript and corresponding prototype.

        @note The template class must be inherited from @ref js::Object.
    */
    template<class T>
    void registerClass(juce::StringRef name)
    {
        runtime.registerClass<T>(name.text);

        auto* ctx = jsContext.get();
        auto proto = T::getProto(ctx);
        JS_SetClassProto(ctx, T::jsClassID, proto);
    }

    /** Create an empty (undefined) value associated with this context. */
    Value makeValue();

    /** Create value of a given type. */
    template <typename T>
    Value makeValue (T x)
    {
        Value val {makeValue()};
        val = x;
        return val;
    }

    /** Returns the global JavaScript scope object. */
    Value getGlobalObject();

    /** Evalute a script.

        This method executed the script provided and returns the
        result of the execution. In case of an error the returned
        value will be an exception.
    */
    Value evaluate (juce::StringRef script, juce::StringRef fileName = "");

    /** Evaluate a script with specified scope.

        This method sets the 'this' scope to the object provided and
        evaluates the script. Returned value is the result of the evaluation
        or an exception in case of an error.
    */
    Value evaluateThis (const Value& thisObj, juce::StringRef script, juce::StringRef fileName = "");

    /** Store native pointer in the global scope.

        This method is used to store native pointers in JavaScript's
        global scope. For example a pointer to this context is stored this
        way so that it can be retrieved from the JavaScript environment
        @ref getContextFromJSContext.
    */
    void setGlobalNative (juce::StringRef name, void* ptr);

    /** Returns native pointer stored in the global scope. */
    void* getGlobalNative (juce::StringRef name);

    /** Invoke a function after a delay.

        This method implements a function delayed invocation by
        starting a timer. This is used to implement the setTimeout
        global function.
    */
    void callAfterDelay (int milliseconds, std::function<void()> f);

    /** Print the current error (exception) along with the execution stack trace. */
    void dumpError();

    /** Retrieve the js::Context object from the JSContext. */
    static Context* getContextFromJSContext (JSContext* ctx);

private:

    void exposeGlobals();

    Runtime& runtime;
    std::unique_ptr<JSContext, void(*)(JSContext*)> jsContext;

    std::unique_ptr<TimerPool> timerPool;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Context)
};

} // namespace juce_litehtml::js
