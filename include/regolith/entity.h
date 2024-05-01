#ifndef REGOLITH_ENTITY_H
#define REGOLITH_ENTITY_H

class rEntity{
private:
    EntityID entityID{0};
    EntityInstanceID instanceID{0};
    PlayerID owner{0};

    rZone* p_parentZone;
public:
    static inline Bedrock::Event<> flushAllEntityMessages;

    virtual void EngineHook_instantiateEntity(){}
    virtual void EngineHook_uninstantiateEntity(){}

    void addComponent();

    void ssFlushMessages();
    void csFlushMessages();

    inline EntityID getEntityID() const { return entityID; }
    inline EntityInstanceID getInstanceID() const { return instanceID; }
    inline PlayerID getOwner() const { return owner; }
    inline rZone* getParentZone() const { return p_parentZone; }

    inline void setParentZone(rZone* zone) { p_parentZone = zone; }
    inline void setEntityInfo(const rEntityInfo& entityInfo){
        entityID = entityInfo.entityID;
        instanceID = entityInfo.instanceID;
        owner = entityInfo.owner;
    }

    bool has_ownership();
    void SERVER_SIDE_tick();
    void SERVER_SIDE_recieve_data(EntityUpdateInfo_t updateInfo);
    //void SERVER_SIDE_transmit_data();
    void CLINET_SIDE_tick();
    void CLIENT_SIDE_recieve_data(EntityUpdateInfo_t updateInfo);
    //void CLIENT_SIDE_transmit_data();

    Ref<Transform3DSync> get_transform3d_sync();
    Ref<Transform2DSync> get_transform2d_sync();
    Ref<EntityInfo> get_entity_info();

    void set_transform3d_sync(Ref<Transform3DSync> transform3DSync);
    void set_transform2d_sync(Ref<Transform2DSync> transform2DSync);
};


class rEntityRegistry{
private:
    std::unordered_map<EntityID, rEntity*> registry;

    rEntityRegistry(){}

public:
    // Delete copy constructor and assignment operator to prevent copying
    rEntityRegistry(const rEntityRegistry&) = delete;
    rEntityRegistry& operator=(const rEntityRegistry&) = delete;

    // Singleton accessor
    static rEntityRegistry& getInstance(){
        static rEntityRegistry instance;
        return instance;
    }
};


#endif //REGOLITH_ENTITY_H
