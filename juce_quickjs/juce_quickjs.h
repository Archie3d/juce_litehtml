/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            juce_quickjs
    vendor:        Arthur Benilov
    version:       1.0.0
    name:          JUCE JavaScript engine
    description:   JavaScript engine based on QuickJS
    website:       https://github.com/Archie3d/juce_litehtml
    license:       GPL/BSD-3

    dependencies:  juce_core, juce_data_structures

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
*/

#pragma once
#define JUCE_QUICKJS_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

extern "C" {
#  include "quickjs.h"
#  include "quickjs-libc.h"
}
