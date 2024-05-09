#include "regolith/regolith.h"


bool rZone::instantiateZone() {
    // Just in case this method is called when the zone is already instantiated
    if (instantiated) {
        return false;
    }

    // Instantiate the zone game object in the game engine
    EngineHook_instantiateZoneStart();

    // Indicate that the zone has been instantiated through the boolean flag
    // Gotta call this before adding the instance to the scene otherwise if any entities are
    // created, it will think that the zone doesn't exist and throw an error.
    instantiated = true;

    // Invoke engine specific code for when the zone has been instantiated.
    EngineHook_instantiateZoneFinish();

    return true;
}

void rZone::uninstantiateZone() {
    // Destroy all entities in the zone
    for(const auto& pair : entitiesInZone){
        destroyEntity(pair.second);
    }

    // Clear all players from the zone
    playersInZone.clear();

    // Invoke engine specific code to clean up the zone game object
    EngineHook_uninstantiateZone();

    // Mark that the zone is no longer instantiated
    instantiated = false;
}

void rZone::addPlayer(rPlayer *player) {
    // Store the player info within the zone
    playersInZone[player->getPlayerID()] = player;
    // Add this zone to the player's list of loaded zones
    player->setCurrentZone(this);
    // Invoke engine specific code for when a player joins
    EngineHook_playerJoinedZone(player);
}

void rZone::removePlayer(rPlayer *player) {
    PlayerID playerID = player->getPlayerID();

    // Return from the function if the specified player isn't in the zone
    if(!hasPlayer(playerID)){
        return;
    }

    //Iterate through the player's owned entities, and remove them
    //from the player's list and the zone if they exist in this zone
    for(const auto& pair : player->ownedEntities){
        destroyEntity(pair.second);
    }

    player->ownedEntities.clear();

    //Remove player from zone's player list (this has to be done after the above bc
    //the "destroy_entity" method uses the player list to erase the owned entity from the player)
    playersInZone.erase(playerID);

    // Invoke engine specific code for when a player leaves the zone
    EngineHook_playerLeftZone(player);

    //Reset the player's zone information
    player->clearZoneInfo();
}

void rZone::loadEntity(rEntityInfo &entityInfo) {
    //Make sure a world is being hosted or a world is connected to before trying to instantiate entities.
    if(Bedrock::isRole(Bedrock::Role::ACTOR_NONE)){
        rDebug::err("Cannot create an entity if not hosting or connected to a world!");
        return;
    }

    //Make sure zone has been instantiated before calling
    if(!instantiated){
        rDebug::err("Cannot create entity in zone, zone has not been instantiated!");
        return;
    }

    //If an associated player is defined, make sure they are in the zone.
    if(entityInfo.owner > 0 && !hasPlayer(entityInfo.owner)){
        rDebug::err("Cannot associate entity with player. They are not in this zone!");
        return;
    }

    //Make sure all other user entered data is valid
    //TODO: Figure out how to do this
//    if(!entityInfo->verify_info()){
//        //ERR_PRINT("Cannot create entity, entity info is not valid!");
//        return;
//    }

    //Set the entity's parent zone id
    entityInfo.parentZone = zoneID;

    if(Bedrock::isRole(Bedrock::Role::ACTOR_CLIENT)){
        // Send the entity creation request
        rControlMsg req{};
        req.msgType = MessageType::CREATE_ENTITY_REQUEST;
        req.entityInfo = entityInfo;

        Bedrock::sendToHost(req);
    }else if(Bedrock::isRole(Bedrock::Role::ACTOR_SERVER)){
        //Assign a network id for the entity
        entityInfo.instanceID = generateEntityInstanceID();

        //Create the entity on the server's side
        rEntity* entity = createEntity(entityInfo);

        //Tell each player in the zone to also create this entity
        for (const auto& player : playersInZone) {
            player.second->loadEntity(entity);
        }
    }
}

rEntity* rZone::createEntity(rEntityInfo &entityInfo) {
    auto* entity = new rEntity;

    // Assign entity info to the entity
    entity->setEntityInfo(entityInfo);

    // Instantiate the entity via engine hook
    entity->EngineHook_instantiateEntity();

    // Add the entity to list of known entities in zone
    entitiesInZone[entityInfo.instanceID] = entity;

    // Store the parent zone instance in the entity
    entity->setParentZone(this);

    // Associate the entity with a player (if such a player was specified)
    PlayerID ownerID = entityInfo.owner;
    if(ownerID != 0){
        playersInZone[ownerID]->addOwnedEntity(entity);
    }

    // Connect the entity to data transmission events.
    // These events will be called ever network tick to send information around.
    if(Bedrock::isRole(Bedrock::Role::ACTOR_SERVER)){
        rEntity::flushAllEntityMessages.subscribe(entity->ssFlushCallback);
    }else if(Bedrock::isRole(Bedrock::Role::ACTOR_CLIENT)){
        rEntity::flushAllEntityMessages.subscribe(entity->csFlushCallback);
    }

    return entity;
}

void rZone::destroyEntity(rEntity* entity) {
    //Make sure the provided entity exists in this zone
    EntityInstanceID instanceID = entity->getInstanceID();
    auto it = entitiesInZone.find(instanceID);
    if(it == entitiesInZone.end()){
        //ERR_PRINT(vformat("Entity with net id %d does not exist in this zone!", entityInfo->get_network_id()));
        return;
    }else{
        //Remove the entity reference stored in the zone
        entitiesInZone.erase(instanceID);
    }

    // If the entity has an owner, remove it from that owner list
    PlayerID ownerID = entity->getOwner();
    if(ownerID != 0){
        playersInZone[ownerID]->removeOwnedEntity(entity);
    }

    //Disconnect the entity from data transmission signals
    if(Bedrock::isRole(Bedrock::Role::ACTOR_SERVER)){
        rEntity::flushAllEntityMessages.unsubscribe(entity->ssFlushCallback);
    }else if(Bedrock::isRole(Bedrock::Role::ACTOR_CLIENT)){
        rEntity::flushAllEntityMessages.unsubscribe(entity->csFlushCallback);
    }

    //Unlink the zone from the entity
    entity->setParentZone(nullptr);

    // Destroy the entity via engine hook
    entity->EngineHook_uninstantiateEntity();

    delete entity;
}

bool rZone::hasPlayer(PlayerID playerID) {
    auto playerIter = playersInZone.find(playerID);
    return playerIter != playersInZone.end();
}

bool rZone::hasPlayer(rPlayer *player) {
    auto playerIter = playersInZone.find(player->getPlayerID());
    return playerIter != playersInZone.end();
}

rPlayer *rZone::getPlayer(const PlayerID &playerID) const {
    auto it = playersInZone.find(playerID);
    if(it != playersInZone.end()){
        return it->second;
    }

    return nullptr;
}

