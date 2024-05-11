#ifndef REGOLITH_ZONE_H
#define REGOLITH_ZONE_H

class rZone {

private:
    ZoneID zoneID{0};
    bool instantiated{false};

    virtual void EngineHook_instantiateZoneStart(){}
    virtual void EngineHook_instantiateZoneFinish(){}
    virtual void EngineHook_uninstantiateZone(){}
    virtual void EngineHook_playerJoinedZone(rPlayer* player){}
    virtual void EngineHook_playerLeftZone(rPlayer* player){}

public:
    std::unordered_map<PlayerID, rPlayer*> playersInZone;
    std::unordered_map<EntityInstanceID, rEntity*> entitiesInZone;

    // Zone events
    Bedrock::Event<void> onLoadedZone;
    Bedrock::Event<void, PlayerID> onPlayerLoadedZone;

    bool instantiateZone();
    void uninstantiateZone();

    void addPlayer(rPlayer* player);
    void removePlayer(rPlayer* player);

    void loadEntity(rEntityInfo& entityInfo);
    rEntity* createEntity(rEntityInfo& entityInfo);
    void destroyEntity(rEntity* entity);

    bool hasPlayer(PlayerID playerID);
    bool hasPlayer(rPlayer* player);
    inline bool isInstantiated() const { return instantiated; };

    inline ZoneID getZoneID() const { return zoneID; }
    rPlayer* getPlayer(const PlayerID& playerID) const;

    inline void setZoneID(const ZoneID& newZoneID) { zoneID = newZoneID; }
};

class rZoneRegistry{
private:
    std::unordered_map<ZoneID, rZone*> registry;

    rZoneRegistry(){}

public:
    // Delete copy constructor and assignment operator to prevent copying
    rZoneRegistry(const rZoneRegistry&) = delete;
    rZoneRegistry& operator=(const rZoneRegistry&) = delete;

    // Singleton accessor
    REGOLITH_API static rZoneRegistry& getInstance(){
        static rZoneRegistry instance;
        return instance;
    }

    REGOLITH_API rStatusCode registerZone(rZone* zone);
    REGOLITH_API rStatusCode unregisterZone(rZone* zone);
    REGOLITH_API rZone* getZoneByID(const ZoneID& zoneID);
};

#endif //REGOLITH_ZONE_H
