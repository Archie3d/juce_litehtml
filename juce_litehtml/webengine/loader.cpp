#if JUCE_TARGET_HAS_BINARY_DATA
#   include "BinaryData.h"
#endif

namespace juce_litehtml {

#if JUCE_TARGET_HAS_BINARY_DATA

static const char* getBinaryDataResourceNameForFile (const String& filename)
{
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        if (filename == BinaryData::originalFilenames[i])
            return BinaryData::namedResourceList[i];
    }

    return nullptr;
}

#endif // JUCE_TARGET_HAS_BINARY_DATA


Loader::Loader()
{
}

String Loader::loadText (const URL& url)
{
    const auto scheme { url.getScheme() };

    if (scheme == "res")
        return loadTextFromResource (url);

    return url.readEntireTextStream (false);
}

String Loader::loadTextFromResource (const URL& url)
{
#if JUCE_TARGET_HAS_BINARY_DATA
    if (auto* resourceName { getBinaryDataResourceNameForFile (url.getFileName()) })
    {
        int size{};

        if (const auto* data { BinaryData::getNamedResource (resourceName, size) })
            return String::fromUTF8 (data, size);
    }
#endif

    return {};
}

} // namespace juce_litehtml
