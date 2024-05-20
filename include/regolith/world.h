#ifndef REGOLITH_WORLD_H
#define REGOLITH_WORLD_H

class rWorld{
private:
    std::unordered_map<PlayerID, std::unordered_set<PlayerID>> awaitingPlayerDeallocation;

    /*========================== SHARED CALLBACKS =========================*/
//    void removePlayer(const Bedrock::ClientID& clientID);
    void removePlayerFromWorld(rPlayer* player);
    void sendWorldPlayerJoinMessage(PlayerID playerID);
    rStatusCode loadZone(rZone* zone);
    rStatusCode unloadZone(rZone* zone);

    /*======================= SERVER SIDE CALLBACKS =======================*/
    void playerConnected(const Bedrock::ClientID& clientID);
    void playerDisconnected(const Bedrock::ClientID& clientID);
    void ssAssignPlayerIDAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssAllocatePlayerAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssRemovePlayerFromWorldAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssPlayerUnloadedZone(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadZoneRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadZoneAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadEntityRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadEntityAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssHandleControlMsg(rControlMsg& inMsg, Bedrock::Message& outMsg);

    /*======================= CLIENT SIDE CALLBACKS =======================*/
    void csAssignPlayerID(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csAllocatePlayer(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csWorldJoinComplete(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csRemovePlayerFromWorld(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csWorldLeaveComplete(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csPlayerUnloadedZone(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csLoadZoneRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csLoadZoneComplete(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csLoadEntityRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csHandleControlMsg(rControlMsg& inMsg, Bedrock::Message& outMsg);

public:
    // Server side variables
    std::unordered_map<int, rPlayer*> playerByClientID;
    std::unordered_map<PlayerID, rPlayer*> playerByPlayerID;

    // Client side variables
    rPlayer* localPlayer{nullptr};

    // World Events
    Bedrock::Event<void> onWorldStart;
    Bedrock::Event<void> onWorldStop;
    Bedrock::Event<void> onWorldJoin;
    Bedrock::Event<void, PlayerID> onWorldPlayerJoin;
    Bedrock::Event<void> onWorldLeave;
    Bedrock::Event<void, PlayerID> onWorldPlayerLeave;


    void startWorld(Port port);
    void stopWorld();

    void joinWorld(const char* world, Port port);
    void leaveWorld();

    rStatusCode loadZone(const char* zoneName);
    rStatusCode loadZone(ZoneID zoneID);
    rStatusCode unloadZone(const char* zoneName);
    rStatusCode unloadZone(ZoneID zoneID);

//
//    //Both
//    bool player_exists(PlayerID_t playerId);
};
#endif //REGOLITH_WORLD_H
