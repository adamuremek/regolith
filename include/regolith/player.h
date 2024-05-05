#ifndef REGOLITH_PLAYER_INFO_H
#define REGOLITH_PLAYER_INFO_H

class rPlayer{
private:
    PlayerID playerID{0};
    Bedrock::ClientID clientID{0};
    rZone* p_currentZone{nullptr};
    bool flagLoadedInOtherPlayers{false};

    std::unordered_set<EntityInstanceID> entitiesWaitingForLoadAck;
    std::unordered_set<PlayerID> playersWaitingForLoadAck;

public:
    std::unordered_map<EntityInstanceID, rEntityInfo> ownedEntities;

    PlayerInfo();
    ~PlayerInfo();

    void loadEntitiesInCurrentZone();
    void loadEntity(rEntity* entity);

    void add_owned_entity(Ref<EntityInfo> associatedEntity);

    void loadPlayer(rPlayer* player);


    void confirm_player_load(PlayerID_t playerId);
    void confirm_entity_load(EntityNetworkID_t entityNetworkId);


    void clearZoneInfo();

    inline PlayerID getPlayerID() const { return playerID; }
    inline Bedrock::ClientID getClientID() const { return clientID; }
    inline rZone* getCurrentZone() const { return p_currentZone; }


    inline void setPlayerID(const PlayerID& newPlayerID) { playerID = newPlayerID; }
    inline void setClientID(const Bedrock::ClientID& newClientID) { clientID = newClientID; }
    inline void setCurrentZone(rZone* currentZone) {p_currentZone = currentZone; }
    inline void setFlagLoadedInOtherPlayers(const bool& flag) { flagLoadedInOtherPlayers = flag; }
};
#endif //REGOLITH_PLAYER_INFO_H
