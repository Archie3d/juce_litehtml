#pragma once

#include "litehtml.h"

namespace juce_litehtml {

/** A single web page.

    This class represents a single web page. It contains
    the HTML document model, which cal be loaded from a
    string or a local or remote resource.

    In order to be loaded and visualized, a page must be associated with a view.

    @note litehtml does not separate clearlt the DOM, layout, and rendering.
          Which is why in order to load a page it must be associated
          with a view (as the view injects methods required for the page
          resources to be loaded).

          A page can be associated with a single view only.

    @see WebView
*/
class WebPage final
{
public:

    /** Page client.

        Page client gets incerted into the page's events chain
        and can be used to drive the behavior of the page.
     */
    class Client
    {
    public:
        virtual ~Client() = default;

        /** Follow the anchor link.

            This method gets called when user clicks an anchor link,
            but before the new URL is loaded. If this method
            returns true, the page continues with the load process,
            otherwise the link following wil be skipped and the page
            stays on the current document.
        */
        virtual bool followLink (const juce::URL& url) = 0;
    };

    //==========================================================================

    WebPage();
    ~WebPage();

    /** Returns web context owned by this page. */
    WebContext& getContext();

    /** Load document from URL.

        This method loads the page from given URL without
        consulting the client's followLink() method.

        @note The page will be loaded asynchronously, however
              some of the resources referenced by the page may
              be further loaded synchronously blocking the message thread.
     */
    void loadFromURL (const juce::URL& url);

    /** Load the page from an HTML string. */
    void loadFromHTML (const juce::String& html);

    /** Reloag the current page. */
    void reload();

    /** Follow the URL.

        This methos is similar to the loadFromURL(),
        however before loading, the assigend client's followLink()
        method will becalled. The client may block the link, in which
        case it won't be loaded.

        @see WebPage::Client
     */
    void followLink (const juce::URL& url);

    juce::URL getURL() const;

    /** Returns the document handled by this page. */
    litehtml::document::ptr getDocument();

    /** Returns the loader used to load resources of this page. */
    WebLoader& getLoader();

    /** Inject page's client.

        Originally the page has no client assigned.

        @note This method cal also be used to unassign a client
              when called with nulltr argument.
     */
    void setClient (Client* client);

    /** Returns currently assigned client. */
    Client* getClient();

private:

    friend class WebView;

    void setRenderer (litehtml::document_container* renderer);

    /** @internal
     *
     * This interface is used to notify the view about the
     * documen't loading process, so that the view could be repainted
     * correctly once the document has been loaded.
     */
    class ViewClient
    {
    public:
        virtual ~ViewClient() = default;
        virtual void documentAboutToBeReloaded() = 0;
        virtual void documentLoaded() = 0;
        virtual WebView* getView() = 0;
    };

    void setViewClient (ViewClient* view);

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebPage)
};

} // namespace juce_litehtml
