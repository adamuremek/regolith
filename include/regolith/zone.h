#ifndef REGOLITH_ZONE_H
#define REGOLITH_ZONE_H

class rZone {

private:
    const static size_t ZONE_NAME_MAX_LEN = 20;

    ZoneID zoneID{0};
    char zoneName[ZONE_NAME_MAX_LEN]{};
    bool instantiated{false};

public:
    std::unordered_map<PlayerID, rPlayer*> playersInZone;
    std::unordered_map<EntityInstanceID, rEntity*> entitiesInZone;

    // Zone events
    Bedrock::Event<void> onLoadedZone;
    Bedrock::Event<void, PlayerID> onPlayerLoadedZone;

    rStatusCode instantiateZone();
    rStatusCode uninstantiateZone();

    // Engine hook implementation through composition
    std::function<rStatusCode()> EngineHook_instantiateZoneStartFunc;
    std::function<rStatusCode()> EngineHook_instantiateZoneFinishFunc;
    std::function<rStatusCode()> EngineHook_uninstantiateZoneFunc;

    // Engine hook implementation through polymorphism
    REGOLITH_API virtual rStatusCode EngineHook_instantiateZoneStart();
    REGOLITH_API virtual rStatusCode EngineHook_instantiateZoneFinish();
    REGOLITH_API virtual rStatusCode EngineHook_uninstantiateZone();

    void addPlayer(rPlayer* player);
    void removePlayer(rPlayer* player);

    void loadEntity(rEntityInfo& entityInfo);
    rEntity* createEntity(rEntityInfo& entityInfo);
    void destroyEntity(rEntity* entity);

    bool hasPlayer(PlayerID playerID);
    bool hasPlayer(rPlayer* player);
    inline bool isInstantiated() const { return instantiated; };

    inline ZoneID getZoneID() const { return zoneID; }
    const char* getZoneName() const { return zoneName; }
    rPlayer* getPlayer(const PlayerID& playerID) const;

    inline void setZoneID(const ZoneID& newZoneID) { zoneID = newZoneID; }
    void setZoneName(const char* name);
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
