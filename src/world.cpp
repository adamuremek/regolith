#include "regolith/regolith.h"

/* TODO 1: So when a player leaves the world, their player ID should be recycled for the next player when they join.
 * However, this comes with a pretty big caveat; all players that are currently in the world must acknowledge that
 * they have freed up the player from their cache. Otherwise, the server could reuse a player's id when one of the end
 * players didnt free up that id yet, and that new player will take on data from the old player on the clients' ends
 * that they shouldnt. TLDR: make sure all clients free up removed players and send an ack to the server.*/

/*========================== SHARED CALLBACKS =========================*/

void rWorld::removePlayer(const Bedrock::ClientID &clientID) {
    // Remove the player info from the connection keyed map and the id keyed map if they exist
    auto it = playerByClientID.find(clientID);
    if (it != playerByClientID.end()) {
        // Assign found value to a variable
        rPlayer *player = it->second;
        PlayerID playerID = player->getPlayerID();

        // Try to get te zone the player is currently loaded into:
        rZone *currentZone = player->getCurrentZone();

        // If the player is indeed in a zone, remove them from said zone
        if (currentZone) {
            ZoneID zoneID = currentZone->getZoneID();
            currentZone->removePlayer(player);

            // Create player unload message for end clients
            rControlMsg msg{};
            msg.msgType = rMessageType::PLAYER_UNLOADED_ZONE;
            msg.playerID = playerID;
            msg.zoneID = zoneID;

            // Tell remaining players in zone to remove the player locally
            for (const auto &playerInZone: currentZone->playersInZone) {
                // Skip the leaving player in case they are encountered
                if (playerInZone.first == playerID) {
                    continue;
                }

                // Send the message to the endpoint player in the zone
                Bedrock::ClientID playerEndpoint = playerInZone.second->getClientID();
                Bedrock::sendToClient(msg, playerEndpoint);
            }
        }

        // Remove the player from the player-clientID maps
        playerByClientID.erase(player->getClientID());
        playerByPlayerID.erase(playerID);

        //TODO: Maybe force close connection if you can? For cases where the server wants to force disconnect client

        // Fire player leave world event
        onWorldPlayerLeave.invoke(player->getPlayerID());

        // Free player memory allocation
        delete player;
    }
}

rStatusCode rWorld::loadZone(rZone* zone) {
    // Make sure the zone is not null
    if(zone == nullptr){
        return rStatusCode::NULL_ZONE_PROVIDED;
    }

    if(Bedrock::isRole(Bedrock::Role::ACTOR_CLIENT)){
        // Send a request to load the zone to the host when acting as client
        rControlMsg msg{};
        msg.msgType = rMessageType::LOAD_ZONE_REQUEST;
        msg.zoneID = zone->getZoneID();

        Bedrock::sendToHost(msg);
        return rStatusCode::SUCCESS;
    } else if(Bedrock::isRole(Bedrock::Role::ACTOR_SERVER)){
        // Try to instantiate the zone (if it hasn't already been) when acting as server
        return zone->instantiateZone();
    }

    return rStatusCode::LOAD_ZONE_FAILED;
}

rStatusCode rWorld::unloadZone(rZone *zone) {
    rDebug::log("A");
    // Make sure the zone is not null
    if(zone == nullptr){
        return rStatusCode::NULL_ZONE_PROVIDED;
    }
    rDebug::log("B");
    if(Bedrock::isRole(Bedrock::Role::ACTOR_CLIENT)){
        // Tell the server that the current player/client is unloading the specified zone
        rControlMsg msg{};
        msg.msgType = rMessageType::PLAYER_UNLOADED_ZONE;
        msg.zoneID = zone->getZoneID();
        msg.playerID = localPlayer->getPlayerID();
        Bedrock::sendToHost(msg);
        rDebug::log("C");

        // Uninstantiate the zone (client side)
        return zone->uninstantiateZone();
    } else if(Bedrock::isRole(Bedrock::Role::ACTOR_SERVER)){
        rDebug::log("E");
        // Uninstantiate the zone (server side)
        return zone->uninstantiateZone();
    }
    rDebug::log("F");
    return rStatusCode::UNLOAD_ZONE_FAILED;
}

/*====================== SERVER SIDE CALLBACKS ======================*/
void rWorld::playerConnected(const Bedrock::ClientID &clientID) {
    // Make sure the connecting player isn't already connected (should be very rare, but just in case :D )
    if (playerByClientID.find(clientID) != playerByClientID.end()) {
        return;
    }

    // Allocate player info
    auto *newPlayer = new rPlayer;

    // Generate a network identifier for the player
    PlayerID newPlayerID = generatePlayerID();

    // Populate player info
    newPlayer->setClientID(clientID);
    newPlayer->setPlayerID(newPlayerID);

    // Send the player their ID
    rControlMsg msg{};
    msg.msgType = rMessageType::ASSIGN_PLAYER_ID;
    msg.playerID = newPlayerID;
    Bedrock::sendToClient(msg, clientID);

    // Add player to a map keyed by client id, and a map keyed by id
    playerByClientID[clientID] = newPlayer;
    playerByPlayerID[newPlayerID] = newPlayer;

    // Fire the player join world event (server side)
    onWorldPlayerJoin.invoke(newPlayerID);
}

void rWorld::playerDisconnected(const Bedrock::ClientID &clientID) {
    removePlayer(clientID);
}

void rWorld::ssAllocatePlayerInstanceAcknowledge(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the player who sent the acknowledgement
    PlayerID playerID = inMsg.playerID;
    rPlayer *player = playerByPlayerID[playerID];

    //Confirm that the player info has been created on the client's end
    player->confirmPlayerLoaded(inMsg.allocatedPlayer);
}

void rWorld::ssPlayerUnloadedZone(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    rDebug::log("AHHH PALYER UNLOADING");
    // Get the player and the zone that the player has left
    rZone *targetZone = rZoneRegistry::getInstance().getZoneByID(inMsg.zoneID);
    rPlayer *leavingPlayer = playerByPlayerID[inMsg.playerID];

    // Make sure the zone with the provided ID exists
    if (targetZone) {
        // Remove the player from the zone
        targetZone->removePlayer(leavingPlayer);
        rDebug::log("Gubba");

        //Tell remaining players in zone to remove the player locally
        for (const auto &pair: targetZone->playersInZone) {
            //Skip the leaving player in case they are encountered
            if (pair.second == leavingPlayer) {
                continue;
            }

            // Send the message to the endpoint player in the zone
            Bedrock::ClientID playerEndpoint = pair.second->getClientID();
            Bedrock::sendToClient(inMsg, playerEndpoint);
        }
    }
}

void rWorld::ssLoadZoneRequest(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the zone requested by the player
    ZoneID zoneID = inMsg.zoneID;
    rZone *zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Make sure the requested zone exists
    if (zone) {
        // Load the zone server side
        zone->instantiateZone();

        // Tell the player to load the zone locally on their end
        Bedrock::serializeType(inMsg, outMsg);
    } else {
        // Tell the player that the zone could not load due to an invalid ID.
        inMsg.msgType = rMessageType::LOAD_ZONE_FAILED_INVALID_ID;
        Bedrock::serializeType(inMsg, outMsg);
    }
}

void rWorld::ssLoadZoneAcknowledge(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the player who sent the acknowledgement and the zone they loaded in (by ID)
    rPlayer *player = playerByPlayerID[inMsg.playerID];
    ZoneID zoneID = inMsg.zoneID;
    rZone *zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Add player to the zone (adds them to the list of players in the zone).
    zone->addPlayer(player);

    // If the loading player is the only one in the zone, just advance to loading the entities.
    // Otherwise, load in the players currently in the zone first then load the entities.
    if (zone->playersInZone.size() == 1) {
        // IMPORTANT: mark that the player has loaded in all other players (since there are none in the zone).
        // Otherwise, the server will make this player load in all entities in the zone again
        // when another player loads into the zone.
        player->setFlagLoadedInOtherPlayers(true);

        // Now make the player load all entities that are currently in the zone
        player->loadEntitiesInCurrentZone();
    } else {
        //Tell all current players in the zone that a new player in loading in.
        //Also tell the new player to load in all other players in the zone.
        //The loading player should not be in the zone yet server side, so we don't have to worry
        //about checking that.
        for (const auto &pair: zone->playersInZone) {
            // Skip the playing being loaded since they (should have) already loaded themselves
            if (pair.second == player) {
                rDebug::log("skipping current player :)");
                continue;
            }

            // Tell the end client in the zone to allocate info for the new player
            pair.second->loadPlayer(player);

            // Make the new player allocate information for the end client
            player->loadPlayer(pair.second);
        }
    }
}

void rWorld::ssLoadEntityRequest(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    rDebug::log("Create entity request recieved!");
    // Get the zone where the entity needs to be loaded into
    ZoneID parentZoneID = inMsg.entityInfo.parentZone;
    rZone *parentZone = rZoneRegistry::getInstance().getZoneByID(parentZoneID);

    if (parentZone) {
        parentZone->loadEntity(inMsg.entityInfo);
    }

    rDebug::log("Entity load complete!");
}

void rWorld::ssLoadEntityAcknowledge(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the player who sent the acknowledgement
    rPlayer *player = playerByPlayerID[inMsg.playerID];

    // Confirm that the end client has loaded this entity (remove from ACK buffer)
    player->confirmEntityLoaded(inMsg.entityInfo.instanceID);
}

void rWorld::ssHandleControlMsg(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    switch (inMsg.msgType) {
        case rMessageType::LOAD_ZONE_REQUEST:
            ssLoadZoneRequest(inMsg, outMsg);
            break;
        case rMessageType::LOAD_ZONE_ACKNOWLEDGE:
            ssLoadZoneAcknowledge(inMsg, outMsg);
            break;
        case rMessageType::PLAYER_UNLOADED_ZONE:
            ssPlayerUnloadedZone(inMsg, outMsg);
            break;
        case rMessageType::CREATE_ENTITY_REQUEST:
            ssLoadEntityRequest(inMsg, outMsg);
            break;
        case rMessageType::CREATE_ENTITY_ACKNOWLEDGE:
            ssLoadEntityAcknowledge(inMsg, outMsg);
            break;
        case rMessageType::ALLOCATE_INCOMING_PLAYER_ACK:
            ssAllocatePlayerInstanceAcknowledge(inMsg, outMsg);
            break;
        default:
            break;
    }
}

/*====================== CLIENT SIDE CALLBACKS ======================*/
void rWorld::csAssignPlayerID(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Assign playerID to local player
    PlayerID playerID = inMsg.playerID;
    localPlayer->setPlayerID(playerID);

    //Locally add the local player to the world's list of players in the world (by ID only)
    playerByPlayerID[playerID] = localPlayer;

    // Fire the join world event for this local client only.
    onWorldJoin.invoke();

    // Fire the player join world event (client side)
    onWorldPlayerJoin.invoke(playerID);
}

void rWorld::csAllocatePlayerInstance(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the player ID for the player that needs a player object instance to be allocated
    PlayerID playerID = inMsg.playerID;

    // Create a new player object instance
    auto player = new rPlayer;
    player->setPlayerID(playerID);

    //Add the new player info to the zone's list players
    localPlayer->getCurrentZone()->addPlayer(player);

    //Send a player object allocation acknowledgement back to the server
    inMsg.msgType = rMessageType::ALLOCATE_INCOMING_PLAYER_ACK;
    inMsg.playerID = localPlayer->getPlayerID();
    inMsg.allocatedPlayer = player->getPlayerID();
    Bedrock::serializeType(inMsg, outMsg);
}

void rWorld::csPlayerUnloadedZone(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the zone object being left and the leaving player
    rZone *targetZone = rZoneRegistry::getInstance().getZoneByID(inMsg.zoneID);
    rPlayer *leavingPlayer = playerByPlayerID[inMsg.playerID];

    // Remove the player from the zone
    targetZone->removePlayer(leavingPlayer);
}

void rWorld::csLoadZoneRequest(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the zone id that the server wants instantiated
    ZoneID zoneID = inMsg.zoneID;
    rZone *zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Make sure requested zone exists
    if (zone) {
        // Add the local player to the zone
        zone->addPlayer(localPlayer);

        // Try to instantiate the zone
        zone->instantiateZone();

        // Acknowledge that the zone has been loaded
        inMsg.msgType = rMessageType::LOAD_ZONE_ACKNOWLEDGE;
        inMsg.playerID = localPlayer->getPlayerID();
        Bedrock::serializeType(inMsg, outMsg);
    } else {
        rDebug::err("[ERR] Could not locate the requested zone by given ID!");
    }
}

void rWorld::csLoadZoneComplete(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the specified zone that was loaded
    rZone *zone = rZoneRegistry::getInstance().getZoneByID(inMsg.zoneID);

    // Fire the zone's player loaded event
    zone->onPlayerLoadedZone.invoke(inMsg.playerID);

    // If the player that loaded in was this local player, then fire the local version of the event
    if (inMsg.playerID == localPlayer->getPlayerID()) {
        zone->onLoadedZone.invoke();
        rDebug::log("Loaded into zone!");
    }
}

void rWorld::csLoadEntityRequest(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    rDebug::log("Create entity rquest recieved!");
    // Get the zone where the entity needs to be loaded into
    ZoneID parentZoneID = inMsg.entityInfo.parentZone;
    rZone *parentZone = rZoneRegistry::getInstance().getZoneByID(parentZoneID);

    // Make sure the relevant zone exists
    if (parentZone) {
        //Create the entity
        parentZone->createEntity(inMsg.entityInfo);

        //Send entity creation acknowledgement to server
        inMsg.msgType = rMessageType::CREATE_ENTITY_ACKNOWLEDGE;
        inMsg.playerID = localPlayer->getPlayerID();
        Bedrock::serializeType(inMsg, outMsg);
    } else {
        rDebug::err("Parent zone ID does not exist when creating entity!");
    }
}

void rWorld::csHandleControlMsg(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    switch (inMsg.msgType) {
        case rMessageType::ASSIGN_PLAYER_ID:
            csAssignPlayerID(inMsg, outMsg);
            break;
        case rMessageType::LOAD_ZONE_REQUEST:
            csLoadZoneRequest(inMsg, outMsg);
            break;
        case rMessageType::LOAD_ZONE_FAILED_INVALID_ID:
            rDebug::err("Sent wrong zone id (client side)");
            break;
        case rMessageType::LOAD_ZONE_COMPLETE:
            csLoadZoneComplete(inMsg, outMsg);
            break;
        case rMessageType::PLAYER_UNLOADED_ZONE:
            csPlayerUnloadedZone(inMsg, outMsg);
            break;
        case rMessageType::CREATE_ENTITY_REQUEST:
            csLoadEntityRequest(inMsg, outMsg);
            break;
        case rMessageType::ALLOCATE_INCOMING_PLAYER:
            csAllocatePlayerInstance(inMsg, outMsg);
            break;
        default:
            break;
    }
}

/*====================== PUBLIC METHODS ======================*/
void rWorld::startWorld(Port port) {
    if (Bedrock::isInitialized) {
        return;
    }

    auto callbacks = [this](rControlMsg &inMsg, Bedrock::Message &outMsg) {
        this->ssHandleControlMsg(inMsg, outMsg);
    };
    // Register server side callbacks
    Bedrock::MessageCallbackRegistry::singleton().registerCallback<rControlMsg>(callbacks);
    Bedrock::EventCallback<void, const Bedrock::ClientID &> connectCallback(this, &rWorld::playerConnected);
    Bedrock::EventCallback<void, const Bedrock::ClientID &> disconnectCallback(this, &rWorld::playerDisconnected);

    Bedrock::onClientConnect.subscribe(connectCallback);
    Bedrock::onClientDisconnect.subscribe(disconnectCallback);

    Bedrock::init();
    Bedrock::startDedicatedHost(port);

    onWorldStart.invoke();
}

void rWorld::stopWorld() {
    Bedrock::shutdown();
    Bedrock::clearEventCallbacks();
    Bedrock::clearMsgCallbacks();

    onWorldStop.invoke();
}

void rWorld::joinWorld(const char *world, Port port) {
    if (Bedrock::isInitialized || Bedrock::isRole(Bedrock::Role::ACTOR_SERVER)) {
        return;
    }

    auto callbacks = [this](rControlMsg &inMsg, Bedrock::Message &outMsg) {
        this->csHandleControlMsg(inMsg, outMsg);
    };

    // Register client side callbacks
    Bedrock::MessageCallbackRegistry::singleton().registerCallback<rControlMsg>(callbacks);

    //Create local player info object to store info in
    localPlayer = new rPlayer;

    //Connect to world
    Bedrock::init();
    if (Bedrock::startClient(port, world) != Bedrock::StatusCode::SUCCESS) {
        return;
    }
}

void rWorld::leaveWorld() {
    if (!Bedrock::isInitialized || !Bedrock::isRole(Bedrock::Role::ACTOR_CLIENT)) {
        return;
    }

    //Check for a zone to unload, and unload it if there is one
    rZone *loadedZone = localPlayer->getCurrentZone();
    if(loadedZone){
        //Remove the player from the zone locally
        loadedZone->removePlayer(localPlayer);

        //Destroy the zone locally (for now)
        loadedZone->uninstantiateZone();

        //Inform host
        rControlMsg msg{};
        msg.msgType = rMessageType::PLAYER_UNLOADED_ZONE;
        msg.playerID = localPlayer->getPlayerID();
        msg.zoneID = localPlayer->getCurrentZone()->getZoneID();

        Bedrock::sendToHost(msg);
    }

    delete localPlayer;
    Bedrock::shutdown();
    Bedrock::clearEventCallbacks();
    Bedrock::clearMsgCallbacks();

    onWorldLeave.invoke();
}

rStatusCode rWorld::loadZone(const char *zoneName) {
    // Try to get the zone by name
    rZone* zone = rZoneRegistry::getInstance().getZoneByName(zoneName);

    // Proceed to load the zone if it was found from the registry, otherwise return appropriate error
    if(zone){
        return loadZone(zone);
    }else{
        rDebug::err("Zone could not be found to load by requested zone name!");
        return rStatusCode::ZONE_WITH_PROVIDED_NAME_NOT_FOUND;
    }
}

rStatusCode rWorld::loadZone(ZoneID zoneID) {
    // Try to get the zone by ID
    rZone* zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Proceed to load the zone if it was found from the registry, otherwise return appropriate error
    if(zone){
        return loadZone(zone);
    }else{
        rDebug::err("Zone could not be found to load by requested zone ID!");
        return rStatusCode::ZONE_WITH_PROVIDED_ID_NOT_FOUND;
    }
}

rStatusCode rWorld::unloadZone(const char *zoneName) {
    // Try to get the zone by name
    rZone* zone = rZoneRegistry::getInstance().getZoneByName(zoneName);

    // Proceed to unload the zone if it was found from the registry, otherwise return appropriate error
    if(zone){
        return unloadZone(zone);
    }else{
        rDebug::err("Zone could not be found to unload by requested zone name!");
        return rStatusCode::ZONE_WITH_PROVIDED_NAME_NOT_FOUND;
    }
}

rStatusCode rWorld::unloadZone(ZoneID zoneID) {
    // Try to get the zone by ID
    rZone* zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Proceed to unload the zone if it was found from the registry, otherwise return appropriate error
    if(zone){
        rStatusCode code = unloadZone(zone);
        rDebug::log("D");
        rDebug::log("%d",(int)code);
        return code;
    }else{
        rDebug::err("Zone could not be found to unload by requested zone ID!");
        return rStatusCode::ZONE_WITH_PROVIDED_ID_NOT_FOUND;
    }
}

