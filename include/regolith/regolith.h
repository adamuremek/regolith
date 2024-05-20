#ifndef REGOLITH_H
#define REGOLITH_H

#ifdef _WIN32
    #ifdef BUILD_SHARED_LIB
        #ifdef REGOLITH_DLL
            #define REGOLITH_API __declspec(dllexport)
        #else
            #define REGOLITH_API __declspec(dllimport)
        #endif
    #else
        #define REGOLITH_API
    #endif
#else
    #define REGOLITH_API
#endif


#include <bedrock/bedrock.h>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <functional>

class rWorld;
class rZone;
class rEntity;
class rComponent;

#include "debug.h"
#include "types.h"
#include "messaging.h"
#include "id_generator.h"
#include "entity.h"
#include "component.h"
#include "player.h"
#include "zone.h"
#include "world.h"


#endif //REGOLITH_H
