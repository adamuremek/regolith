#ifndef REGOLITH_ZONE_H
#define REGOLITH_ZONE_H

class rZone {

private:
    ZoneID zoneID{0};
    bool instantiated{false};

protected:
    static void _bind_methods();
    void _notification(int n_type);

public:
    std::unordered_map<PlayerID, rPlayer*> playersInZone;
    std::unordered_map<EntityInstanceID, rEntityInfo> entitiesInZone;

    virtual void EngineHook_instantiateZoneStart(){}
    virtual void EngineHook_instantiateZoneFinish(){}
    virtual void EngineHook_uninstantiateZone(){}
    virtual void EngineHook_playerJoinedZone(rPlayer* player){}
    virtual void EngineHook_playerLeftZone(rPlayer* player){}

    bool instantiateZone();
    void uninstantiateZone();


    void addPlayer(rPlayer* player);
    void removePlayer(rPlayer* player);


    void loadEntity(const rEntityInfo& entityInfo);
    void createEntity(Ref<EntityInfo> entityInfo);
    void destroyEntity(const rEntityInfo& entityInfo);

    void player_loaded_callback(Ref<PlayerInfo> playerInfo);

    bool player_in_zone(PlayerID_t player);
    bool is_instantiated();



    inline ZoneID getZoneID() const { return zoneID; }

    inline void setZoneID(const ZoneID& newZoneID) { zoneID = newZoneID; }


    Ref<PackedScene> get_zone_scene() const;
    Ref<PlayerInfo> get_player(PlayerID_t playerId) const;

    void set_zone_scene(const Ref<PackedScene> &zoneScene);
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
