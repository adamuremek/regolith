#ifndef REGOLITH_WORLD_H
#define REGOLITH_WORLD_H

class rWorld{
private:
    HashMap<HSteamNetConnection, Ref<PlayerInfo>> m_worldPlayerInfoByConnection;
    HashMap<PlayerID_t, Ref<PlayerInfo>> m_worldPlayerInfoById;


    void player_connected(Bedrock::ClientID playerConnection);
    void player_disconnected(HSteamNetConnection playerConnection);
    void remove_player(HSteamNetConnection hConn);

    void SERVER_SIDE_load_zone_request(const unsigned char *mssgData, HSteamNetConnection sourceConn);
    void SERVER_SIDE_load_zone_acknowledge(const unsigned char *mssgData, HSteamNetConnection sourceConn);
    void SERVER_SIDE_create_zone_player_info_acknowledge(const unsigned char *mssgData, HSteamNetConnection sourceConn);
    void SERVER_SIDE_load_entity_request(const unsigned char *mssgData);
    void SERVER_SIDE_load_entity_acknowledge(const unsigned char *mssgData, HSteamNetConnection sourceConn);
    void SERVER_SIDE_handle_entity_update(const unsigned char *mssgData, const int mssgLen);
    void SERVER_SIDE_player_left_zone(const unsigned char *mssgData);

    void SERVER_SIDE_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *pInfo);
    void SERVER_SIDE_poll_incoming_messages();
    void server_listen_loop();
    void server_tick_loop();

    //Client Side
    Ref<PlayerInfo> m_localPlayer;
    bool m_clientRunLoop;
    std::thread m_clientListenThread;
    std::thread m_clientTickThread;

    static void CLIENT_SIDE_CONN_CHANGE(SteamNetConnectionStatusChangedCallback_t *pInfo);

    void CLIENT_SIDE_assign_player_id(const unsigned char *mssgData);
    void CLIENT_SIDE_zone_load_complete(const unsigned char *mssgData);
    void CLIENT_SIDE_load_zone_request(const unsigned char *mssgData);
    void CLIENT_SIDE_process_create_zone_player_info_request(const unsigned char *mssgData);
    void CLIENT_SIDE_load_entity_request(const unsigned char *mssgData);
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

    bool CLIENT_SIDE_instantiate_zone(ZoneID_t zoneId);

    PlayerID_t get_player_id();
    void join_world(String world, int port);
    void leave_world();

    bool load_zone_by_name(String zoneName);
    bool load_zone_by_id(ZoneID_t zoneId);
    void unload_zone();

    //Both
    bool player_exists(PlayerID_t playerId);
};
#endif //REGOLITH_WORLD_H
