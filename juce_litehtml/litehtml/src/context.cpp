#include "html.h"
#include "context.h"
#include "stylesheet.h"
#include "document.h"

extern "C" {
#	include "quickjs-libc.h"
}

litehtml::context::context()
{
	m_jsRuntime = JS_NewRuntime();
	m_jsContext = JS_NewContext(m_jsRuntime);

	js_register_default_classes();
}

litehtml::context::~context()
{
	JS_FreeContext(m_jsContext);
	JS_FreeRuntime(m_jsRuntime);
}

JSValue litehtml::context::js_eval(const litehtml::tstring& script)
{
	const int evalFlags { JS_DetectModule (script.c_str(), script.length()) ? JS_EVAL_TYPE_MODULE : JS_EVAL_TYPE_GLOBAL };

	if ((evalFlags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE)
	{
		JSValue val { JS_Eval (m_jsContext, script.c_str(), script.length(), "", evalFlags | JS_EVAL_FLAG_COMPILE_ONLY) };

		if (!JS_IsException(val))
		{
			constexpr int use_realpath = 1;
			constexpr int is_main = 1;
			js_module_set_import_meta(m_jsContext, val, use_realpath, is_main);
			val = JS_EvalFunction(m_jsContext, val);
		}

		return val;
	}

	return JS_Eval(m_jsContext, script.c_str(), script.length(), "", evalFlags);
}

void litehtml::context::load_master_stylesheet( const tchar_t* str )
{
	media_query_list::ptr media;

	m_master_css.parse_stylesheet(str, nullptr, std::shared_ptr<litehtml::document>(), media_query_list::ptr());
	m_master_css.sort_selectors();
}

void litehtml::context::js_register_method(JSContext* ctx, JSValue prototype, const tchar_t* name, JSCFunction func)
{
    JS_SetPropertyStr(ctx, prototype, name, JS_NewCFunction(ctx, func, name, 0));
}

void litehtml::context::js_register_constructor(JSContext* ctx, JSValue prototype, const tchar_t* name, JSCFunction func, int numArgs)
{
    JSValue clazz { JS_NewCFunction2(ctx, func, name, numArgs, JS_CFUNC_constructor, 0) };
    auto global { JS_GetGlobalObject(ctx) };
    JS_DefinePropertyValueStr(ctx, global, name, JS_DupValue(ctx, clazz), JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetConstructor(ctx, clazz, prototype);
    JS_FreeValue(ctx, global);
    JS_FreeValue(ctx, clazz);
}

void litehtml::context::js_register_property(JSContext* ctx, JSValue prototype, const tchar_t* name, JSGetter getter, JSSetter setter)
{
    auto aProperty { JS_NewAtom(ctx, name) };
    int flags { JS_PROP_CONFIGURABLE };

    if (getter != nullptr)
        flags |= JS_PROP_ENUMERABLE;

    if (setter != nullptr)
        flags |= JS_PROP_WRITABLE;

    auto get { getter != nullptr ? JS_NewCFunction2(ctx, (JSCFunction*)getter, "<get>", 0, JS_CFUNC_getter, 0) : JS_UNDEFINED };
    auto set { setter != nullptr ? JS_NewCFunction2(ctx, (JSCFunction*)setter, "<set>", 1, JS_CFUNC_setter, 0) : JS_UNDEFINED };

    JS_DefinePropertyGetSet(ctx, prototype, aProperty, get, set, flags);
    JS_FreeAtom(ctx, aProperty);
}

void litehtml::context::js_register_default_classes()
{	
	js_register_class<litehtml::document>("Document");
	js_register_class<litehtml::element>("Element");
	js_register_class<litehtml::html_tag>("HTMLElement");
}
