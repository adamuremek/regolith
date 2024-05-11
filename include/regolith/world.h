#ifndef REGOLITH_WORLD_H
#define REGOLITH_WORLD_H

class rWorld{
private:
    // Server side variables
    std::unordered_map<Bedrock::ClientID, rPlayer*> playerByClientID;
    std::unordered_map<PlayerID, rPlayer*> playerByPlayerID;

    // Client side variables
    rPlayer* localPlayer{nullptr};

    /*======================= SHARED CALLBACKS =======================*/
    void removePlayer(const Bedrock::ClientID& clientID);

    /*======================= SERVER SIDE CALLBACKS =======================*/
    void playerConnected(const Bedrock::ClientID& clientID);
    void playerDisconnected(const Bedrock::ClientID& clientID);
    void ssAllocatePlayerInstanceAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssPlayerUnloadedZone(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadZoneRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadZoneAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadEntityRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadEntityAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssHandleControlMsg(rControlMsg& inMsg, Bedrock::Message& outMsg);

    /*======================= CLIENT SIDE CALLBACKS =======================*/
    void csAssignPlayerID(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csAllocatePlayerInstance(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csPlayerUnloadedZone(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csLoadZoneRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csLoadZoneComplete(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csLoadEntityRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csHandleControlMsg(rControlMsg& inMsg, Bedrock::Message& outMsg);

public:
    // World Events
    Bedrock::Event<void> onJoinedWorld;
    Bedrock::Event<void, PlayerID> onPlayerJoinWorld;


    REGOLITH_API void startWorld(Port port);
    REGOLITH_API void stopWorld();

    REGOLITH_API void joinWorld(const char* world, Port port);
//    void leaveWorld();
//
//    bool load_zone_by_name(String zoneName);
//    bool load_zone_by_id(ZoneID_t zoneId);
//    void unload_zone();
//
//    //Both
//    bool player_exists(PlayerID_t playerId);
};
#endif //REGOLITH_WORLD_H
