#ifndef REGOLITH_WORLD_H
#define REGOLITH_WORLD_H

class rWorld{
private:
    std::unordered_map<Bedrock::ClientID, rPlayer*> playerByClientID;
    std::unordered_map<PlayerID, rPlayer*> playerByPlayerID;

    // World Events
    Bedrock::Event<> onJoinedWorld;
    Bedrock::Event<PlayerID> onPlayerJoinWorld;

    //server callbacks?
    void playerConnected(const Bedrock::ClientID& clientID);
    void playerDisconnected(const Bedrock::ClientID& clientID);

    void removePlayer(const Bedrock::ClientID& clientID);

    void ssAllocatePlayerInstanceAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadZoneRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadZoneAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void ssLoadEntityAcknowledge(rControlMsg& inMsg, Bedrock::Message& outMsg);


    void SERVER_SIDE_load_entity_request(const unsigned char *mssgData);
    void SERVER_SIDE_handle_entity_update(const unsigned char *mssgData, const int mssgLen);
    void SERVER_SIDE_player_left_zone(const unsigned char *mssgData);

    void SERVER_SIDE_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *pInfo);
    void SERVER_SIDE_poll_incoming_messages();
    void server_listen_loop();
    void server_tick_loop();

    //Client Side
    rPlayer* localPlayer{nullptr};

    void csAssignPlayerID(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csAllocatePlayerInstance(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csLoadZoneRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);
    void csLoadEntityRequest(rControlMsg& inMsg, Bedrock::Message& outMsg);


    void CLIENT_SIDE_zone_load_complete(const unsigned char *mssgData);
    void CLIENT_SIDE_handle_entity_update(const unsigned char *mssgData, const int mssgLen);
    void CLIENT_SIDE_player_left_zone(const unsigned char *mssgData);

    void CLIENT_SIDE_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *pInfo);
    void CLIENT_SIDE_poll_incoming_messages();
    void client_listen_loop();
    void client_tick_loop();

public:
    World();
    ~World();
    void cleanup();



    //Server side
    void start_world(int port);
    void stop_world();

    //Client side
    HSteamNetConnection m_worldConnection;

    PlayerID_t get_player_id();

    void joinWorld(const char* world, int port);
    void leaveWorld();

    bool load_zone_by_name(String zoneName);
    bool load_zone_by_id(ZoneID_t zoneId);
    void unload_zone();

    //Both
    bool player_exists(PlayerID_t playerId);
};
#endif //REGOLITH_WORLD_H
