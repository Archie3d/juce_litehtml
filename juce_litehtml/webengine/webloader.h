#pragma once

namespace juce_litehtml {

class WebLoader final
{
public:

    WebLoader();

    void setBaseURL (const juce::URL& url);
    juce::URL getBaseURL() const { return baseURL; }

    juce::String loadText (const juce::URL& url);
    juce::Image loadImage (const juce::URL& url);

private:
    juce::URL fixUpURL (const juce::URL& url) const;

    static juce::String loadTextFromResource (const juce::String& resName);
    static juce::Image loadImageFromResource (const juce::String& resName);
    static juce::Image loadImageFromFile (const juce::File& file);

    juce::URL baseURL;
};

} // namespace juce_litehtml
