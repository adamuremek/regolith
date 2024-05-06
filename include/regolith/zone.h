#ifndef REGOLITH_ZONE_H
#define REGOLITH_ZONE_H

class rZone {

private:
    ZoneID zoneID{0};
    bool instantiated{false};

public:
    std::unordered_map<PlayerID, rPlayer*> playersInZone;
    std::unordered_map<EntityInstanceID, rEntity*> entitiesInZone;

    // Zone events
    Bedrock::Event<> onLoadedZone;
    Bedrock::Event<PlayerID> onPlayerLoadedZone;

    virtual void EngineHook_instantiateZoneStart(){}
    virtual void EngineHook_instantiateZoneFinish(){}
    virtual void EngineHook_uninstantiateZone(){}
    virtual void EngineHook_playerJoinedZone(rPlayer* player){}
    virtual void EngineHook_playerLeftZone(rPlayer* player){}

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
    static rZoneRegistry& getInstance(){
        static rZoneRegistry instance;
        return instance;
    }

    inline rZone* getZoneByID(const ZoneID& zoneID){
        auto it = registry.find(zoneID);
        if(it != registry.end()){
            return it->second;
        }else{
            return nullptr;
        }
    }
};

#endif //REGOLITH_ZONE_H
