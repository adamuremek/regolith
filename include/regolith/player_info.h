#ifndef REGOLITH_PLAYER_INFO_H
#define REGOLITH_PLAYER_INFO_H

class rPlayer{

public:
    PlayerInfo_t m_playerInfo;

    PlayerInfo();
    ~PlayerInfo();

    void load_player(Ref<PlayerInfo> playerInfo);
    void load_entity(Ref<EntityInfo> entityInfo);
    void load_entities_in_current_zone();
    void add_owned_entity(Ref<EntityInfo> associatedEntity);
    void confirm_player_load(PlayerID_t playerId);
    void confirm_entity_load(EntityNetworkID_t entityNetworkId);
    void reset_zone_info();

    PlayerID_t get_player_id();
    HSteamNetConnection get_player_conn();
    Zone* get_current_loaded_zone();

    void set_player_id(PlayerID_t playerId);
    void set_player_conn(HSteamNetConnection playerConnection);
    void set_current_loaded_zone(Zone* zone);
};
#endif //REGOLITH_PLAYER_INFO_H
