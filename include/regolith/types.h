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
    PLAYER_LEFT_ZONE
};


//This struct is used for server side data storage only
struct PlayerInfo {
    PlayerID id;
    Zone* currentLoadedZone;
    std::unordered_map<EntityInstanceID, >
    HashMap<EntityNetworkID_t, Ref<EntityInfo>> ownedEntities;

    //Server-side relavent info only
    HSteamNetConnection playerConnection; // Connection Handle
    bool loadedPlayersInZone;
    bool loadedEntitiesInZone;
    List<EntityNetworkID_t> entitiesWaitingForLoadAck;
    List<PlayerID_t> playersWaitingForLoadAck;
};

//struct NetworkEntityInfo_t {
//    EntityID_t id;
//    String name;
//    Ref<PackedScene> scene;
//};
//
//struct ZoneInfo_t {
//    ZoneID_t id;
//    String name;
//    Zone *zone;
//};
//
//struct EntityInfo_t {
//    String entityName;
//    String parentRelativePath;
//    ZoneID_t parentZone;
//    EntityID_t entityId;
//    EntityNetworkID_t networkId;
//    PlayerID_t owner;
//    Vector3 initialPosition3D;
//    Vector2 initialPosition2D;
//    Vector<unsigned char> dataBuffer;
//
//    NetworkEntity *entityInstance;
//};
//
//struct EntityUpdateInfo_t{
//    ZoneID_t parentZone;
//    EntityNetworkID_t networkId;
//    unsigned char updateType;
//    Vector<unsigned char> dataBuffer;
//};
#endif //REGOLITH_TYPES_H
