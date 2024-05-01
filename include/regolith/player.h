#ifndef REGOLITH_PLAYER_INFO_H
#define REGOLITH_PLAYER_INFO_H

class rPlayer{
private:
    PlayerID playerID{0};
    Bedrock::ClientID clientID{0};
    rZone* p_currentZone{nullptr};


public:
    std::unordered_map<EntityInstanceID, rEntity*> ownedEntities;

    void addOwnedEntity(rEntity* ownedEntity);
    void removeOwnedEntity(rEntity* ownedEntity);

    void load_player(Ref<PlayerInfo> playerInfo);
    void load_entity(Ref<EntityInfo> entityInfo);
    void load_entities_in_current_zone();
    void confirm_player_load(PlayerID_t playerId);
    void confirm_entity_load(EntityNetworkID_t entityNetworkId);


    void clearZoneInfo();

    inline PlayerID getPlayerID() const { return playerID; }
    inline Bedrock::ClientID getClientID() const { return clientID; }
    inline rZone* getCurrentZone() const { return p_currentZone; }


    inline void setPlayerID(const PlayerID& newPlayerID) { playerID = newPlayerID; }
    inline void setClientID(const Bedrock::ClientID& newClientID) { clientID = newClientID; }
    inline void setCurrentZone(rZone* currentZone) {p_currentZone = currentZone; }
};
#endif //REGOLITH_PLAYER_INFO_H
