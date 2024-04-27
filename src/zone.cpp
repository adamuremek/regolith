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
    // Destory all entities in the zone
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
    auto playerIter = playersInZone.find(playerID);
    if( playerIter == playersInZone.end()){
        return;
    }

    //Iterate through the player's owned entities, and remove them
    //from the player's list and the zone if they exist in this zone
    for(const auto& pair : player->ownedEntities){
        auto entityIter = entitiesInZone.find(pair.first);
        if(entityIter != entitiesInZone.end()){
            destroyEntity(entityIter->second);
        }
    }
    player->ownedEntities.clear();

    //Remove player from zone's player list (this has to be done after the above bc
    //the "destroy_entity" method uses the player list to erase the owned entity from the player)
    playersInZone.erase(playerID);

    // Invoke engine specific code for when a player leaves the zone
    EngineHook_playerLeftZone(player);

    //Reset the player's information
    player->clearZoneInfo();
}

void rZone::loadEntity(const rEntityInfo &entityInfo) {
    //Make sure a world is being hosted or a world is connected to before trying to instantiate entities.
    if(Bedrock::BedrockMetadata::getInstance().isRole(ACTOR_NONE)){
        //ERR_PRINT("Cannot create an entity if not hosting or connected to a world!");
        return;
    }

    //Make sure zone has been instantiated before calling
    if(!m_instantiated){
        //ERR_PRINT("Cannot create entity in zone, zone has not been instantiated!");
        return;
    }

    //If an associated player is defined, make sure they are in the zone.
    if(entityInfo->get_owner_id() > 0 && !player_in_zone(entityInfo->get_owner_id())){
        //ERR_PRINT(vformat("Cannot associate entity with player \"ID: %d\". They are not in this zone!", entityInfo->get_owner_id()));
        return;
    }

    //Make sure all other user entered data is valid
    if(!entityInfo->verify_info()){
        //ERR_PRINT("Cannot create entity, entity info is not valid!");
        return;
    }

    //Set the entity's parent zone id
    entityInfo->m_entityInfo.parentZone = m_zoneId;

    if(GDNet::singleton->is_client()){
        //Serialize the entity info
        entityInfo->serialize_info();

        //Make the message a creation request and prepare the data
        entityInfo->m_entityInfo.dataBuffer.set(0, CREATE_ENTITY_REQUEST);
        const unsigned char* mssgData = entityInfo->m_entityInfo.dataBuffer.ptr();
        int mssgDataLen = entityInfo->m_entityInfo.dataBuffer.size();

        //Create and send the message
        SteamNetworkingMessage_t* mssg = allocate_message(mssgData, mssgDataLen, GDNet::singleton->world->m_worldConnection);
        send_message_reliable(mssg);
        print_line("Sent entity creation request! :)");
    }else if(GDNet::singleton->is_server()){
        //Assign a network id for the entity
        entityInfo->set_network_id(IDGenerator::generateNetworkIdentityID());
        //Create the entity
        create_entity(entityInfo);

        //Reserialize the entity info with the new data in it
        entityInfo->serialize_info();

        //Tell each player in the zone to also create this entity
        for (const KeyValue<PlayerID_t, Ref<PlayerInfo>> &player : m_playersInZone) {
            player.value->load_entity(entityInfo);
        }
    }
}

void rZone::destroyEntity(const rEntityInfo &entityInfo) {
    //Make sure the provided entity exists in this zone
    EntityInstanceID instanceID = entityInfo.instanceID;
    auto it = entitiesInZone.find(instanceID);
    if(it == entitiesInZone.end()){
        //ERR_PRINT(vformat("Entity with net id %d does not exist in this zone!", entityInfo->get_network_id()));
        return;
    }else{
        //Remove the entity reference stored in the zone
        entitiesInZone.erase(instanceID);
    }

    //Disconnect the entity from data transmission signals
    NetworkEntity* instanceAsEntity = entityInfo->m_entityInfo.entityInstance;
    if(GDNet::singleton->m_isServer){
        Callable transmitCallback = Callable(instanceAsEntity, "server_side_transmit_data");
        GDNet::singleton->world->disconnect("_server_side_transmit_entity_data", transmitCallback);
    }else{
        Callable transmitCallback = Callable(instanceAsEntity, "client_side_transmit_data");
        GDNet::singleton->world->disconnect("_client_side_transmit_entity_data", transmitCallback);
    }

    //Unlink the zone from the entity
    instanceAsEntity->m_parentZone = nullptr;

    //Delete the node instance
    instanceAsEntity->queue_free();

    //Remove the entity instance pointer reference from the entity info
    entityInfo->m_entityInfo.entityInstance = nullptr;

    print_line(vformat("Ref Count for entity %d: %d", networkId, entityInfo->get_reference_count()));

    print_line("Entity destoryed");
}

