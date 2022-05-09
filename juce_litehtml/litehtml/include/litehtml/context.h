#ifndef LH_CONTEXT_H
#define LH_CONTEXT_H

#include <cassert>

extern "C" {
#	include "quickjs.h"
}

#include "stylesheet.h"

namespace litehtml
{
	class context
	{
	public:
		context();
		virtual ~context();

		/** Register JavaScript class. */
		template<class T>
		void js_register_class(const char* className)
		{
			if (T::jsClassID == 0)
				JS_NewClassID (&T::jsClassID);

			if (!JS_IsRegisteredClass(m_jsRuntime, T::jsClassID))
			{
				const JSClassDef def {
					className,
					[](JSRuntime*, JSValue value) {
                    	if (auto* ref { static_cast<typename T::js_object_ref*>(JS_GetOpaque (value, T::jsClassID)) })
						{
							delete ref;
						}
					},
					nullptr,
					nullptr,
					nullptr
				};

				[[maybe_unused]] const auto res = JS_NewClass(m_jsRuntime, T::jsClassID, &def);
				assert (res == 0);
			}

			auto proto { JS_NewObject(m_jsContext) };
			T::register_js_prototype(m_jsContext, proto);
			JS_SetClassProto(m_jsContext, T::jsClassID, proto);
		}

		/** Returns inner reference object. */
		template<class T>
		static typename T::js_object_ref* js_get_object_ref(JSValue obj)
		{
			void* opaque { nullptr };
			const JSClassID classID { JS_GetClassID(obj, &opaque) };
			
			if (classID == T::jsClassID && opaque != nullptr)
				return static_cast<typename T::js_object_ref*>(opaque);

			return nullptr;
		}

		/** Evaluate script. */
		JSValue js_eval(const litehtml::tstring& script);

		void			load_master_stylesheet(const tchar_t* str);
		litehtml::css&	master_css() { return m_master_css; }
		JSRuntime*		js_runtime() { return m_jsRuntime; }
		JSContext*		js_context() { return m_jsContext; }

		/** Register JS prototype method. */
    	static void js_register_method(JSContext* ctx, JSValue prototype, const tchar_t* name, JSCFunction func);

    	/** Register JS prototype constructor. */
    	static void js_register_constructor(JSContext* ctx, JSValue prototype, const tchar_t* name, JSCFunction func, int numArgs = 0);

		using JSGetter = JSValue(*)(JSContext*, JSValueConst);
    	using JSSetter = JSValue(*)(JSContext*, JSValueConst, JSValueConst);

    	/** Register JS protptype property. */
    	static void js_register_property(JSContext* ctx, JSValue prototype, const tchar_t* name, JSGetter getter, JSSetter setter = nullptr);

	private:

		litehtml::css	m_master_css;
		JSRuntime*		m_jsRuntime;
		JSContext*		m_jsContext;

		/** Register default classes. */
		void js_register_default_classes();
	};

}

#endif  // LH_CONTEXT_H
