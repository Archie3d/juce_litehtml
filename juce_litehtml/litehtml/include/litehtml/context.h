#ifndef LH_CONTEXT_H
#define LH_CONTEXT_H

#include <cassert>

extern "C" {
#	include "quickjs.h"
}

#include "stylesheet.h"

namespace litehtml
{
	struct js_object_ref
	{
		virtual ~js_object_ref() = default;
		virtual void release_js_object() {};
	};

	class context
	{
		litehtml::css	m_master_css;
		JSRuntime*		m_jsRuntime;
		JSContext*		m_jsContext;

	public:
		context();
		virtual ~context();

		template<class T>
		void register_js_class(const char* className)
		{
			if (T::jsClassID == 0)
				JS_NewClassID (&T::jsClassID);

			if (!JS_IsRegisteredClass(m_jsRuntime, T::jsClassID))
			{
				const JSClassDef def {
					className,
					[](JSRuntime*, JSValue value) {
                    	if (auto* ref { static_cast<T::element_js_object_ref*>(JS_GetOpaque (value, T::jsClassID)) })
						{
							ref->release_js_object();
							delete ref;
						}
					},
					nullptr,	// JSClassGCMark
					nullptr,	// JSClassCall
					nullptr		// JSClassExoticMethods
				};

				[[maybe_unused]] const auto res = JS_NewClass(m_jsRuntime, T::jsClassID, &def);
				assert (res == 0);
			}

			auto proto { JS_NewObject(m_jsContext) };
			T::register_js_prototype(m_jsContext, proto);
			JS_SetClassProto(m_jsContext, T::jsClassID, proto);
		}

		void register_js_classes();

		void			load_master_stylesheet(const tchar_t* str);
		litehtml::css&	master_css() { return m_master_css; }
		JSRuntime*		js_runtime() { return m_jsRuntime; }
		JSContext*		js_context() { return m_jsContext; }
	};

}

#endif  // LH_CONTEXT_H
