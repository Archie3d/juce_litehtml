#pragma once

namespace juce_litehtml {

class WebLoader final : private juce::URL::DownloadTask::Listener
{
public:

    WebLoader();
    ~WebLoader();

    void setCachePath (const juce::File& cacheFolder);
    juce::File getCachePath() const { return cachePath; }
    void setCacheLifetime (int seconds);
    void purgeCache();

    void setBaseURL (const juce::URL& url);
    juce::URL getBaseURL() const { return baseURL; }

    bool loadLocal (const juce::URL& url, juce::String& text);
    bool loadLocal (const juce::URL& url, juce::Image& image);

    /** Load resource asynchronously.

        This is a generic method to load resources asynchronously.
        If there is a cached resource that can be delivered immediately, this method
        will be performed synchronously. Otherwise the resource will be downloaded and cached.
        Once loaded, the passed callback function gets called.

        The Boolean argument in the callback tells whether the reource has been
        loaded successfully or not.
    */
    template <typename T>
    void loadAsync (const juce::URL& url, const std::function<void (bool, const T&)>& callback, const juce::String& headers = "")
    {
        auto fixedUrl { fixUpURL (url) };

        {
            T content{};

           // Reading local and binary resources
            if (loadLocal (fixedUrl, content))
            {
                callback (true, content);
                return;
            }

            // Reading from the cache
            if (loadFromCache (fixedUrl, content))
            {
                callback (true, content);
                return;
            }
        }

        const auto file { getCachedResource (fixedUrl) };

        const auto downloadOptions = juce::URL::DownloadTaskOptions()
            .withExtraHeaders (headers)
            .withListener (this);

        if (auto task { fixedUrl.downloadToFile (file, downloadOptions) })
        {
            auto* taskPtr {task.get()};
            downloadTasks.add (task.release());

            downloadTaskCallbackMap[taskPtr] = [objValid = std::weak_ptr<bool>(valid), this, fixedUrl, callback]() -> void {
                if (objValid.expired())
                    return;

                T content{};

                if (loadFromCache (fixedUrl, content, false))
                    callback (true, content);
                else
                    callback (false, content);
            };
        }
        else
        {
            // Failed to load
            callback (false, {});
        }
    }

    /** Load text resource synchronously.

        This will attempt to load the requested resource locally or from
        cache, and if failed will load it from the network on the current thread.
        The loaded content will be cached.
     */
    juce::String loadTextSync (const juce::URL& url);

    /** Load local or cached resource synchronously.

        This method delivers local reources (local files or embedded binary),
        or already cached resources loaded from network. The load is performed
        synchronously.
     */
    template <typename T>
    bool loadLocalOrCached (const juce::URL& url, T& content)
    {
        const auto fixedUrl { fixUpURL (url) };

        // Reading local and binary resources
        if (loadLocal (fixedUrl, content))
            return true;

        // Reading from the cache
        if (loadFromCache (fixedUrl, content))
            return true;

        return false;
    }

    juce::URL fixUpURL (const juce::URL& url) const;

private:
    void assureCachePathExists();
    juce::File getCachedResource (const juce::URL& url);

    bool isCachedResourceValid (const juce::File& file);

    bool loadFromCache (const juce::URL& url, juce::String& text, bool validate = true);
    bool loadFromCache (const juce::URL& url, juce::Image& image, bool validate = true);

    static juce::String loadTextFromResource (const juce::String& resName);
    static juce::Image loadImageFromResource (const juce::String& resName);
    static juce::Image loadImageFromFile (const juce::File& file);

    // juce::URL::DownloadTask::Listener
    void finished (juce::URL::DownloadTask* task, bool success) override;
    void progress (juce::URL::DownloadTask* task, juce::int64 bytesDownloaded, juce::int64 totalLength) override;

    juce::URL baseURL;
    juce::File cachePath;
    juce::RelativeTime cacheLifetime;

    juce::OwnedArray<juce::URL::DownloadTask> downloadTasks;
    std::map<juce::URL::DownloadTask*, std::function<void()>> downloadTaskCallbackMap;

    /// Validity flag used to track this object deletion when in callbacks.
    std::shared_ptr<bool> valid;
};

} // namespace juce_litehtml
