#ifndef REGOLITH_TYPES_H
#define REGOLITH_TYPES_H

using PlayerID = uint32_t;
using EntityInstanceID = uint32_t ;
using EntityID = uint32_t;
using ZoneID = uint32_t;
using PropertyID_t = uint32_t;

enum class MessageType : uint8_t {
    NONE,
    ASSIGN_PLAYER_ID,
    LOAD_ZONE_REQUEST,
    LOAD_ZONE_FAILED_INVALID_ID,
    LOAD_ZONE_ACKNOWLEDGE,
    PLAYER_LEFT_ZONE,
    CREATE_ENTITY_REQUEST,
    ALLOCATE_INCOMING_PLAYER
};


#endif //REGOLITH_TYPES_H
