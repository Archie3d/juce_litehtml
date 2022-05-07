/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            juce_litehtml
    vendor:        Arthur Benilov
    version:       1.0.0
    name:          litehtml
    description:   HTML rendering engine
    website:       https://github.com/Archie3d/juce_litehtml
    license:       GPL/BSD-3

    dependencies:  juce_gui_basics, juce_gui_extra

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
*/

#pragma once
#define JUCE_LITEHTML_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "litehtml.h"

#include "webengine/webloader.h"
#include "webengine/webpage.h"
#include "webengine/webview.h"

