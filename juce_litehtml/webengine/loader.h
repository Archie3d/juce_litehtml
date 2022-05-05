#pragma once

namespace juce_litehtml {

class Loader final
{
public:

    Loader();

    void setBaseURL (const juce::URL& url) { baseURL = url; }
    juce::URL getBaseURL() const { return baseURL; }

    juce::String loadText (const juce::URL& url);

private:
    juce::String loadTextFromResource (const juce::URL& url);

    juce::URL baseURL;
};

} // namespace juce_litehtml
