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


WebLoader::WebLoader()
{
}

String WebLoader::loadText (const URL& url)
{
    const auto scheme { url.getScheme() };

    if (scheme == "res")
        return loadTextFromResource (url.getFileName());

    return url.readEntireTextStream (false);
}

Image WebLoader::loadImage (const URL& url)
{
    const auto scheme { url.getScheme() };

    if (scheme == "res")
        return loadImageFromResource (url.getFileName());

    if (url.isLocalFile())
        return loadImageFromFile (url.getLocalFile());

    return {};
}

String WebLoader::loadTextFromResource (const String& resName)
{
#if JUCE_TARGET_HAS_BINARY_DATA
    if (auto* resourceName { getBinaryDataResourceNameForFile (resName) })
    {
        int size{};

        if (const auto* data { BinaryData::getNamedResource (resourceName, size) })
            return String::fromUTF8 (data, size);
    }
#endif

    return {};
}

Image WebLoader::loadImageFromResource (const String& resName)
{
#if JUCE_TARGET_HAS_BINARY_DATA
    if (auto* resourceName { getBinaryDataResourceNameForFile (resName) })
    {
        int size{};

        if (const auto* data { BinaryData::getNamedResource (resourceName, size) })
            return ImageCache::getFromMemory (data, size);
    }
#endif

    return {};
}

Image WebLoader::loadImageFromFile (const File& file)
{
    return ImageCache::getFromFile (file);
}

} // namespace juce_litehtml
