namespace juce_litehtml {

static Colour webColour (const litehtml::web_color& c)
{
    return Colour (c.red, c.green, c.blue, c.alpha);
}

static String juceString (const litehtml::tchar_t* s)
{
#ifdef LITEHTML_UTF8
    return String::fromUTF8 (s);
#else
    return String (s);
#endif
}

static String juceString (const litehtml::tstring& s)
{
    return String (s.c_str());
}

static litehtml::tstring to_tstring (const String& s)
{
#ifdef LITEHTML_UTF8
    return s.toStdString();
#else
    return tstring (s.toWideCharPointer());
#endif
}

static const litehtml::tchar_t* to_tchar (const String& s)
{
#ifdef LITEHTML_UTF8
    return s.toRawUTF8();
#else
    return s.toWideCharPointer();
#endif
}

} // namespace juce_litehtml
