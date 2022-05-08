#include "html.h"
#include "context.h"
#include "stylesheet.h"

litehtml::context::context()
{
	m_jsRuntime = JS_NewRuntime();
	m_jsContext = JS_NewContext(m_jsRuntime);
}

litehtml::context::~context()
{
	JS_FreeContext(m_jsContext);
	JS_FreeRuntime(m_jsRuntime);
}

void litehtml::context::register_js_classes()
{
	register_js_class<litehtml::element>("Element");
	register_js_class<litehtml::html_tag>("HTMLElement");
}

void litehtml::context::load_master_stylesheet( const tchar_t* str )
{
	media_query_list::ptr media;

	m_master_css.parse_stylesheet(str, nullptr, std::shared_ptr<litehtml::document>(), media_query_list::ptr());
	m_master_css.sort_selectors();
}
