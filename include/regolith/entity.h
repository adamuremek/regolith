#ifndef REGOLITH_ENTITY_H
#define REGOLITH_ENTITY_H

class REGOLITH_API rEntity {
private:
    rEntityInfo entityInfo;
    rZone *p_parentZone;
public:
    Bedrock::EventCallback<void> ssFlushCallback;
    Bedrock::EventCallback<void> csFlushCallback;
    static inline Bedrock::Event<void> flushAllEntityMessages;

    rEntity() : p_parentZone{nullptr}, ssFlushCallback(this, &rEntity::ssFlushMessages),
                csFlushCallback(this, &rEntity::csFlushMessages) {}

    ~rEntity() = default;

    virtual void EngineHook_instantiateEntity() {}

    virtual void EngineHook_uninstantiateEntity() {}

//    void addComponent();

    inline EntityID getEntityID() const { return entityInfo.entityID; }

    inline EntityInstanceID getInstanceID() const { return entityInfo.instanceID; }

    inline PlayerID getOwner() const { return entityInfo.owner; }

    inline ZoneID getParentZoneID() const { return entityInfo.parentZone; }

    inline rZone *getParentZone() const { return p_parentZone; }

    inline rEntityInfo getEntityInfo() const { return entityInfo; }

    inline void setParentZone(rZone *zone) { p_parentZone = zone; }

    inline void setEntityInfo(const rEntityInfo &info) { entityInfo = info; }

    void ssFlushMessages();

    void csFlushMessages();

    bool has_ownership();
};


class rEntityRegistry {
private:
    std::unordered_map<EntityID, rEntity *> registry;

    rEntityRegistry() {}

public:
    // Delete copy constructor and assignment operator to prevent copying
    rEntityRegistry(const rEntityRegistry &) = delete;

    rEntityRegistry &operator=(const rEntityRegistry &) = delete;

    // Singleton accessor
    static rEntityRegistry &getInstance() {
        static rEntityRegistry instance;
        return instance;
    }
};


#endif //REGOLITH_ENTITY_H
