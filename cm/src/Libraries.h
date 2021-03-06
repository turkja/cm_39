#ifndef CM_LIBRARIES_H
#define CM_LIBRARIES_H

//----//
//JUCE//
//----//

#define DONT_SET_USING_JUCE_NAMESPACE 1

#include "AppConfig.h"
#include "modules/juce_audio_basics/juce_audio_basics.h"
#include "modules/juce_audio_devices/juce_audio_devices.h"
#include "modules/juce_audio_formats/juce_audio_formats.h"
#include "modules/juce_audio_processors/juce_audio_processors.h"
#include "modules/juce_audio_utils/juce_audio_utils.h"
#include "modules/juce_core/juce_core.h"
#include "modules/juce_cryptography/juce_cryptography.h"
#include "modules/juce_data_structures/juce_data_structures.h"
#include "modules/juce_events/juce_events.h"
#include "modules/juce_graphics/juce_graphics.h"
#include "modules/juce_gui_basics/juce_gui_basics.h"
#include "modules/juce_gui_extra/juce_gui_extra.h"
#include "modules/juce_opengl/juce_opengl.h"
#include "modules/juce_video/juce_video.h"

//-----------//
// S7 Scheme //
//-----------//

#include "s7.h"

//---------------//
//Other Libraries//
//---------------//

#ifdef WITH_SDIF
#include "sdif.h"
#endif

#ifdef WITH_LIBLO
#include "lo/lo.h"
#endif

#ifdef WITH_FOMUS
#define FOMUS_TYPESONLY
#include <fomus/fomusapi.h>
#include <fomus/infoapi.h>
#endif


//-----------------//
//Platform-Specific//
//-----------------//


//----------------//
//Standard library//
//----------------//
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

//These should be eventually changed to <cmath>, <cstring>, etc.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

//----------//
//Namespaces//
//----------//
//using namespace juce;
#endif
