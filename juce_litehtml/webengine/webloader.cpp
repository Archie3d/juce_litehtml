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

void WebLoader::setBaseURL (const URL& url)
{
    if (url.getFileName().isEmpty())
        baseURL = url;
    else
        baseURL = url.getParentURL();
}

String WebLoader::loadText (const URL& url)
{
    const auto fixedUrl { fixUpURL (url) };
    const auto scheme { fixedUrl.getScheme() };

    if (scheme == "res")
        return loadTextFromResource (fixedUrl.getFileName());

    return fixedUrl.readEntireTextStream (false);
}

Image WebLoader::loadImage (const URL& url)
{
    const auto fixedUrl { fixUpURL (url) };
    const auto scheme { fixedUrl.getScheme() };

    if (scheme == "res")
        return loadImageFromResource (fixedUrl.getFileName());

    if (fixedUrl.isLocalFile())
        return loadImageFromFile (fixedUrl.getLocalFile());

    return {};
}

URL WebLoader::fixUpURL (const URL& url) const
{
    if (url.getScheme().isEmpty())
    {
        if (const auto subPath { url.getSubPath() }; subPath.isNotEmpty())
            return baseURL.getChildURL (subPath);

        return baseURL.getChildURL (url.getFileName());
    }

    return url;
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
