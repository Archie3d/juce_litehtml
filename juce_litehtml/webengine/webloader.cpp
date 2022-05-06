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

//==============================================================================

WebLoader::WebLoader()
    : cacheLifetime (43200.0), // [s]
      valid { std::make_shared<bool>(true) }
{
    setCachePath (File::getSpecialLocation (File::tempDirectory).getChildFile ("juce_litehtml"));
}

WebLoader::~WebLoader()
{
    *valid = false;
}

void WebLoader::setCachePath (const File& cacheFolder)
{
    cachePath = cacheFolder;
    assureCachePathExists();
}

void WebLoader::setCacheLifetime (int seconds)
{
    cacheLifetime = RelativeTime ((double) seconds);
}

void WebLoader::purgeCache()
{
    cachePath.deleteRecursively();
    assureCachePathExists();
}

void WebLoader::setBaseURL (const URL& url)
{
    if (url.getFileName().isEmpty())
        baseURL = url;
    else
        baseURL = url.getParentURL();
}

bool WebLoader::loadLocal (const URL& url, String& text)
{
    const auto fixedUrl { fixUpURL (url) };
    const auto scheme { fixedUrl.getScheme() };

    if (scheme == "res")
    {
        text = loadTextFromResource (fixedUrl.getFileName());
        return true;
    }

    if (fixedUrl.isLocalFile())
    {
        text = fixedUrl.readEntireTextStream (false);
        return true;
    }

    return false;
}

bool WebLoader::loadLocal (const URL& url, Image& image)
{
    const auto fixedUrl { fixUpURL (url) };
    const auto scheme { fixedUrl.getScheme() };

    if (scheme == "res")
    {
        image = loadImageFromResource (fixedUrl.getFileName());
        return true;
    }

    if (fixedUrl.isLocalFile())
    {
        image = loadImageFromFile (fixedUrl.getLocalFile());
        return true;
    }

    return false;
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

void WebLoader::assureCachePathExists()
{
    if (! cachePath.exists())
        cachePath.createDirectory();
}

File WebLoader::getCachedResource (const URL& url)
{
    const auto hash { url.toString (true).hash() };
    return cachePath.getChildFile (String::toHexString (hash));
}

bool WebLoader::isCachedResourceValid (const File& file)
{
    if (! file.existsAsFile())
        return false;

    // We consider empty files as invalid in order to
    // avoid caching potentially failed downloads.
    if (file.getSize() == 0)
    {
        file.deleteFile();
        return false;
    }

    RelativeTime fileAge { Time::getCurrentTime() - file.getCreationTime() };

    if (fileAge.inMilliseconds() > cacheLifetime.inMilliseconds())
    {
        // Cached resource is expired
        file.deleteFile();
        return false;
    }

    return true;
}

bool WebLoader::loadFromCache (const URL& url, String& text, bool validate)
{
    const auto file { getCachedResource (url) };

    if (validate && (! isCachedResourceValid (file)))
        return false;

    text = file.loadFileAsString();

    return true;
}

bool WebLoader::loadFromCache (const URL& url, juce::Image& image, bool validate)
{
    const auto file { getCachedResource (url) };

    if (validate && (! isCachedResourceValid (file)))
        return false;

    image = ImageFileFormat::loadFrom (file);

    return true;
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

void WebLoader::finished (URL::DownloadTask* task, bool success)
{
    jassert (task != nullptr);

    MessageManager::callAsync ([objValid = std::weak_ptr<bool>(valid), this, task, success]() {
        if (objValid.expired())
            return;

        auto it = downloadTaskCallbackMap.find (task);

        if (it != downloadTaskCallbackMap.end())
        {
            if (success)
            {
                it->second();
            }
            else
            {
                auto targetFile {task->getTargetLocation()};
                targetFile.deleteFile();
            }

            downloadTaskCallbackMap.erase (it);
        }

        downloadTasks.removeObject (task, true);
    });

}

void WebLoader::progress (URL::DownloadTask*, int64, int64)
{
}

} // namespace juce_litehtml
