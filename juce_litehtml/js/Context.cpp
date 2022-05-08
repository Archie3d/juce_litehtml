namespace juce_litehtml::js {

// This literal is used to inject the js::Context pointer to the
// JavaScript global scope so that it can be retrieved then.
const char* const contextInternalLiteral {"__contetx__"};

//==============================================================================

static JSValue js_log (JSContext *ctx, JSValueConst this_val,
                       int argc, JSValueConst *argv)
{
    ignoreUnused(this_val);

    String juceString;

    int i;
    const char *str;
    size_t len;

    for(i = 0; i < argc; i++) {
        if (i != 0)
            juceString += ' ';
        str = JS_ToCStringLen(ctx, &len, argv[i]);
        if (!str)
            return JS_EXCEPTION;

        juceString += String::fromUTF8(str, (int) len);
        JS_FreeCString(ctx, str);
    }

    DBG (juceString);

    return JS_UNDEFINED;
}

static JSValue js_setTimeout (JSContext* ctx, JSValueConst this_val,
                              int argc, JSValueConst* argv)
{
    ignoreUnused (this_val);

    if (argc != 2)
        return JS_EXCEPTION;

    if (! JS_IsFunction (ctx, argv[0]))
        return JS_EXCEPTION;

    if (JS_VALUE_GET_TAG (argv[1]) != JS_TAG_INT)
        return JS_EXCEPTION;

    auto* jsContext {Context::getContextFromJSContext (ctx)};

    if (jsContext == nullptr)
        return JS_EXCEPTION;

    int delay{};
    JS_ToInt32 (ctx, &delay, argv[1]);

    auto func {std::make_shared<js::Function>(ctx, argv[0])};

    auto callback = [f = func]() {
        f->call();
    };

    jsContext->callAfterDelay (delay, callback);

    return JS_UNDEFINED;
}

//==============================================================================


struct Context::TimerPool::Invoker : private Timer
{
    Invoker (Context::TimerPool& pool, int milliseconds, std::function<void()> f)
        : timerPool {pool}
        , func {f}
    {
        if (func)
            startTimer (milliseconds);
    }

    ~Invoker()
    {
        stopTimer();
    }

    void timerCallback() override
    {
        func();
        timerPool.removeInvoker (this);
        // this is now deleted!
    }

    Context::TimerPool& timerPool;
    std::function<void()> func;


    JUCE_DECLARE_NON_COPYABLE (Invoker)
};

//------------------------------------------------------

Context::TimerPool::TimerPool()
{
}

Context::TimerPool::~TimerPool() = default;

void Context::TimerPool::callAfterDelay (int milliseconds, std::function<void()> f)
{
    auto inv {std::make_shared<Invoker>(*this, milliseconds, f)};
    invokers.push_back (inv);
}

void Context::TimerPool::removeInvoker (Invoker* ptr)
{
    auto it = invokers.begin();
    while (it != invokers.end())
    {
        if (it->get() == ptr)
        {
            it = invokers.erase (it);
            return;
        }
        ++it;
    }
}

//==============================================================================

Context::Context(Runtime& rt)
    : runtime {rt}
    , jsContext (JS_NewContext (rt.getJSRuntime()), JS_FreeContext)
    , timerPool {std::make_unique<TimerPool>()}
{
    JS_SetModuleLoaderFunc(rt.getJSRuntime(), nullptr, js_module_loader, nullptr);
    exposeGlobals();
}

Context::~Context()
{
    // Delete all the pending timers first
    timerPool.reset();
}

void Context::reset()
{
    timerPool.reset();
    jsContext.reset(JS_NewContext (runtime.getJSRuntime()));
    timerPool = std::make_unique<TimerPool>();
    exposeGlobals();
}

Value Context::makeValue()
{
    return Value (jsContext.get());
}

Value Context::getGlobalObject()
{
    return Value (jsContext.get(), JS_GetGlobalObject (jsContext.get()));
}

Value Context::evaluate (StringRef script, StringRef fileName)
{
    const int evalFlags {JS_DetectModule (script, script.length()) ? JS_EVAL_TYPE_MODULE : JS_EVAL_TYPE_GLOBAL};

    if ((evalFlags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE)
    {
        JSValue val {JS_Eval (jsContext.get(), script, script.length(), fileName, evalFlags | JS_EVAL_FLAG_COMPILE_ONLY)};

        if (!JS_IsException (val))
        {
            constexpr int qjsTRUE {1};
            js_module_set_import_meta (jsContext.get(), val, qjsTRUE, qjsTRUE);
            val = JS_EvalFunction (jsContext.get(), val);
        }

        return Value (jsContext.get(), val);
    }

    return Value (
        jsContext.get(),
        JS_Eval (jsContext.get(), script, script.length(), fileName, evalFlags)
    );
}

Value Context::evaluateThis (const Value& thisObj, StringRef script, StringRef fileName)
{
    const int evalFlags {JS_EVAL_TYPE_GLOBAL};

    return Value (
        jsContext.get(),
        JS_EvalThis (jsContext.get(), thisObj.getJSValue(), script, script.length(), fileName, evalFlags)
    );
}

void Context::setGlobalNative (StringRef name, void* ptr)
{
    auto* ctx {jsContext.get()};
    auto obj {JS_NewObject (ctx)};
    JS_SetOpaque (obj, ptr);

    auto globalObj {JS_GetGlobalObject (ctx)};
    JS_SetPropertyStr (ctx, globalObj, name, obj);

    JS_FreeValue (ctx, globalObj);
}

void* Context::getGlobalNative (juce::StringRef name)
{
    void* ptr {nullptr};
    auto* ctx {jsContext.get()};
    auto globalObj {JS_GetGlobalObject (ctx)};
    auto obj {JS_GetPropertyStr (ctx, globalObj, name)};

    if (JS_VALUE_GET_TAG (obj) == JS_TAG_OBJECT)
    {
        JS_GetClassID (obj, &ptr);
    }

    JS_FreeValue (ctx, obj);
    JS_FreeValue (ctx, globalObj);

    return ptr;
}

static void js_dump_obj(JSContext* ctx, JSValueConst val)
{
    if (const char* str = JS_ToCString(ctx, val)) {
        std::cerr << str << '\n';
        JS_FreeCString(ctx, str);
    } else {
        std::cerr << "[exception]\n";
    }
}

static void js_dump_error(JSContext* ctx, JSValueConst exception)
{
    if (JS_IsError(ctx, exception)) {
        auto message = JS_GetPropertyStr(ctx, exception, "message");
        if (!JS_IsUndefined(message)) {
            if (auto* str = JS_ToCString(ctx, message)) {
                std::cerr << str << '\n';
                JS_FreeCString(ctx, str);
            }
        }
        JS_FreeValue(ctx, message);

        auto stack = JS_GetPropertyStr(ctx, exception, "stack");
        if (!JS_IsUndefined(stack))
            js_dump_obj(ctx, stack);

        JS_FreeValue(ctx, stack);
    }
}

void Context::callAfterDelay (int milliseconds, std::function<void()> f)
{
    jassert (timerPool != nullptr);
    timerPool->callAfterDelay (milliseconds, f);
}

Context* Context::getContextFromJSContext (JSContext* ctx)
{
    Context* contextPtr {nullptr};
    auto global_obj {JS_GetGlobalObject(ctx)};
    auto obj {JS_GetPropertyStr (ctx, global_obj, contextInternalLiteral)};

    void* opaque {nullptr};
    JS_GetClassID (obj, &opaque);
    JS_FreeValue (ctx, obj);

    if (opaque != nullptr)
        contextPtr = reinterpret_cast<Context*>(opaque);

    JS_FreeValue (ctx, global_obj);

    return contextPtr;
}


void Context::dumpError()
{
    auto* ctx = jsContext.get();
    JSValue exception = JS_GetException(ctx);
    js_dump_error(ctx, exception);
    JS_FreeValue(ctx, exception);
}

void Context::exposeGlobals()
{
    auto* ctx {jsContext.get()};
    auto global_obj {JS_GetGlobalObject(ctx)};

    /* __context__ */
    auto context {JS_NewObject(ctx)};
    JS_SetOpaque(context, this);
    JS_SetPropertyStr (ctx, global_obj, contextInternalLiteral, context);

    /* console.log */
    auto console {JS_NewObject(ctx)};
    JS_SetPropertyStr (ctx, console, "log",
                       JS_NewCFunction (ctx, js_log, "log", 1));
    JS_SetPropertyStr (ctx, global_obj, "console", console);

    /* setTimeout */
    auto setTimeout {JS_NewCFunction (ctx, js_setTimeout, "setTimeout", 2)};
    JS_SetPropertyStr (ctx, global_obj, "setTimeout", setTimeout);

    JS_FreeValue (ctx, global_obj);
}

} // namespace juce_litehtml::js
