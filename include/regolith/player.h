#ifndef REGOLITH_PLAYER_INFO_H
#define REGOLITH_PLAYER_INFO_H

class rPlayer{
private:
    PlayerID playerID{0};
    Bedrock::ClientID clientID{0};
    rWorld* p_currentWorld{nullptr};
    rZone* p_currentZone{nullptr};
    bool flagAllocatedPlayersInWorld{false};
    bool flagLoadedAllEntitiesInZone{false};

    std::unordered_set<EntityInstanceID> entitiesWaitingForLoadAck;
    std::unordered_set<PlayerID> awaitingPlayerAllocation;

public:
    std::unordered_map<EntityInstanceID, rEntity*> ownedEntities;

    inline void addOwnedEntity(rEntity* entity) { ownedEntities[entity->getInstanceID()] = entity; }
    inline void removeOwnedEntity(rEntity* entity) { ownedEntities.erase(entity->getInstanceID()); }
    void loadEntitiesInCurrentZone();
    void loadEntity(rEntity* entity);
    void confirmEntityLoaded(EntityInstanceID entityInstanceID);

    // Called server side only
    void allocatePlayer(rPlayer* player);
    void confirmPlayerAllocation(PlayerID allocatedPlayerID);


    void clearZoneInfo();

    [[nodiscard]] inline PlayerID getPlayerID() const { return playerID; }
    [[nodiscard]] inline Bedrock::ClientID getClientID() const { return clientID; }
    [[nodiscard]] inline rZone* getCurrentZone() const { return p_currentZone; }
    inline bool getFlagAllocatedPlayersInWorld() const { return flagAllocatedPlayersInWorld; }

    inline void setPlayerID(const PlayerID& newPlayerID) { playerID = newPlayerID; }
    inline void setClientID(const Bedrock::ClientID& newClientID) { clientID = newClientID; }
    inline void setCurrentWorld(rWorld* currentWorld) { p_currentWorld = currentWorld; }
    inline void setCurrentZone(rZone* currentZone) { p_currentZone = currentZone; }
    inline void setFlagAllocatedPlayersInWorld(const bool& flag) { flagAllocatedPlayersInWorld = flag; }
};
#endif //REGOLITH_PLAYER_INFO_H
