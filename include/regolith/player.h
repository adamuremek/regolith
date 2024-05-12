#ifndef REGOLITH_PLAYER_INFO_H
#define REGOLITH_PLAYER_INFO_H

class rPlayer{
private:
    PlayerID playerID{0};
    Bedrock::ClientID clientID{0};
    rZone* p_currentZone{nullptr};
    bool flagLoadedInOtherPlayers{false};
    bool flagLoadedAllEntitiesInZone{false};

    std::unordered_set<EntityInstanceID> entitiesWaitingForLoadAck;
    std::unordered_set<PlayerID> playersWaitingForLoadAck;

public:
    std::unordered_map<EntityInstanceID, rEntity*> ownedEntities;

    inline void addOwnedEntity(rEntity* entity) { ownedEntities[entity->getInstanceID()] = entity; }
    inline void removeOwnedEntity(rEntity* entity) { ownedEntities.erase(entity->getInstanceID()); }
    void loadEntitiesInCurrentZone();
    void loadEntity(rEntity* entity);
    void confirmEntityLoaded(EntityInstanceID entityInstanceID);

    void loadPlayer(rPlayer* player);
    void confirmPlayerLoaded(PlayerID playerID);


    void clearZoneInfo();

    [[nodiscard]] inline PlayerID getPlayerID() const { return playerID; }
    [[nodiscard]] inline Bedrock::ClientID getClientID() const { return clientID; }
    [[nodiscard]] inline rZone* getCurrentZone() const { return p_currentZone; }

    inline void setPlayerID(const PlayerID& newPlayerID) { playerID = newPlayerID; }
    inline void setClientID(const Bedrock::ClientID& newClientID) { clientID = newClientID; }
    inline void setCurrentZone(rZone* currentZone) {p_currentZone = currentZone; }
    inline void setFlagLoadedInOtherPlayers(const bool& flag) { flagLoadedInOtherPlayers = flag; }
};
#endif //REGOLITH_PLAYER_INFO_H
