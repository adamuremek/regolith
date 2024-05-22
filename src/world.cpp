#include "regolith/regolith.h"

//void rWorld::removePlayer(const Bedrock::ClientID &clientID) {
//    // Check to see if the player exists (by connection)
//    auto it = playerByClientID.find(clientID);
//    if (it == playerByClientID.end()){
//        rDebug::err("Player by client ID %d does not exist!", clientID);
//        return;
//    }
//
//    // Assign found value to a variable
//    rPlayer *player = it->second;
//    PlayerID playerID = player->getPlayerID();
//
////    // Try to get the zone the player is currently loaded into:
////    rZone *currentZone = player->getCurrentZone();
////
////    // If the player is in a zone, remove them from said zone
////    if (currentZone) {
////        ZoneID zoneID = currentZone->getZoneID();
////        currentZone->removePlayer(player);
////
////        // Create player unload message for end clients
////        rControlMsg msg{};
////        msg.msgType = rMessageType::PLAYER_UNLOADED_ZONE;
////        msg.playerID = playerID;
////        msg.zoneID = zoneID;
////
////        // Tell remaining players in zone to remove the player locally
////        for (const auto &playerInZone: currentZone->playersInZone) {
////            // Skip the leaving player in case they are encountered
////            if (playerInZone.first == playerID) {
////                continue;
////            }
////
////            // Send the message to the endpoint player in the zone
////            Bedrock::ClientID playerEndpoint = playerInZone.second->getClientID();
////            Bedrock::sendToClient(msg, playerEndpoint);
////        }
////    }
//
//    // Remove the player from the player-clientID maps
//    playerByClientID.erase(player->getClientID());
//    playerByPlayerID.erase(playerID);
//
//    //TODO: Maybe force close connection if you can? For cases where the server wants to force disconnect client
//
//    // Fire player leave world event
//    onWorldPlayerLeave.invoke(player->getPlayerID());
//
//    // Free player memory allocation
//    delete player;
//
//}

void rWorld::removePlayerFromWorld(rPlayer *player) {
    // Remove the player from the player-clientID maps
    playerByClientID.erase(player->getClientID());
    playerByPlayerID.erase(player->getPlayerID());

    // Free player memory allocation
    delete player;
}

void rWorld::sendWorldPlayerJoinMessage(PlayerID playerID) {
    // Fire the player join world event (server side)
    onWorldPlayerJoin.invoke(playerID);

    // Tell every other player in the server that "this" player has joined the world (including themselves)
    rControlMsg msg;
    msg.msgType = rMessageType::WORLD_JOIN_COMPLETE;
    msg.playerID = playerID;

    for(const auto& pair : playerByClientID){
        Bedrock::sendToClient(msg, pair.first);
    }
}

void rWorld::sendWorldPlayerLeaveMessage(PlayerID playerID) {
    // Fire player leave event (server side)
    onWorldPlayerLeave.invoke(playerID);

    // Tell every other player in the world that "this" player has left the world
    rControlMsg msg;
    msg.msgType = rMessageType::WORLD_LEAVE_COMPLETE;
    msg.playerID = playerID;

    for(const auto& pair : playerByClientID){
        Bedrock::sendToClient(msg, pair.first);
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
        rDebug::log("C");
        // Tell the server that the current player/client is unloading the specified zone
        rControlMsg msg{};
        msg.msgType = rMessageType::PLAYER_UNLOADED_ZONE;
        msg.zoneID = zone->getZoneID();
        msg.playerID = localPlayer->getPlayerID();
        Bedrock::sendToHost(msg);
        rDebug::log("D");
        // Uninstantiate the zone (client side)
        rStatusCode code = zone->uninstantiateZone();
        rDebug::log("E");
        if(code == rStatusCode::SUCCESS){
            // Reset the player's current zone
            localPlayer->setCurrentZone(nullptr);
            onZoneUnload.invoke();
        }
        rDebug::log("F");
        return code;
    } else if(Bedrock::isRole(Bedrock::Role::ACTOR_SERVER)){
        rDebug::log("G");
        // Uninstantiate the zone (server side)
        return zone->uninstantiateZone();
    }
    rDebug::log("H");
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
    newPlayer->setCurrentWorld(this);

    // Send the player their ID
    rControlMsg msg{};
    msg.msgType = rMessageType::ASSIGN_PLAYER_ID;
    msg.playerID = newPlayerID;
    Bedrock::sendToClient(msg, clientID);

    // Add player to a map keyed by client id, and a map keyed by id
    playerByClientID[clientID] = newPlayer;
    playerByPlayerID[newPlayerID] = newPlayer;
}

void rWorld::playerDisconnected(const Bedrock::ClientID &clientID) {
    // Check to see if the player exists (by connection)
    auto it = playerByClientID.find(clientID);
    if (it == playerByClientID.end()){
        rDebug::err("Player by client ID %d does not exist!", clientID);
        return;
    }

    // Store player stuff in this scope
    rPlayer *player = it->second;
    PlayerID playerID = player->getPlayerID();

    // Remove player from world maps and from any zone they are in
    removePlayerFromWorld(player);

    // Add remaining players to the deallocation waiting buffer.
    // The buffer is used to confirm what players removed the disconnected player locally.
    for(const auto& pair : playerByPlayerID){
        awaitingPlayerDeallocation[playerID].insert(pair.first);
    }

    // Tell all remaining players to remove the disconnected player locally
    if(playerByPlayerID.empty()){
        // If there are no players left in the world, just fire the player leave event server side
        sendWorldPlayerLeaveMessage(playerID);

        // Clear the deallocation wait buffer in case a bunch of people leave at the same time, causing some
        // dirty data to be stored in the buffer
        awaitingPlayerDeallocation.clear();

        // Reclaim any leaked player IDs
        reclaimPlayerIDs();

    } else{
        // Send the message to all remaining players
        rControlMsg msg;
        msg.msgType = rMessageType::REMOVE_PLAYER_FROM_WORLD;
        msg.playerID = playerID;

        for(const auto& pair : playerByClientID){
            Bedrock::sendToClient(msg, pair.first);
        }
    }

}

void rWorld::ssAssignPlayerIDAcknowledge(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the player who sent the acknowledgement
    PlayerID playerID = inMsg.playerID;
    rPlayer* player = playerByPlayerID[playerID];

    // Since the first player will be the only one in the zone,
    // just send them the player join message for themselves (there are no other players to allocate).
    if(playerByPlayerID.size() == 1){
        sendWorldPlayerJoinMessage(playerID);
        return;
    }

    // Tell the joining player to allocate memory for all other players in the world
    for(const auto& pair : playerByPlayerID){
        if(pair.second == player){
            continue;
        }

        player->allocatePlayer(pair.second);
    }
}

void rWorld::ssAllocatePlayerAcknowledge(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the player who sent the acknowledgement
    PlayerID playerID = inMsg.playerID;
    rPlayer *player = playerByPlayerID[playerID];

    //Confirm that the player info has been created on the client's end
    player->confirmPlayerAllocation(inMsg.allocatedPlayer);

    if(player->getFlagAllocatedPlayersInWorld()){
        sendWorldPlayerJoinMessage(playerID);
    }
}

void rWorld::ssRemovePlayerFromWorldAcknowledge(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    PlayerID removedPlayerID = inMsg.removedPlayer;

    // Remove the acknowledging player from the waiting buffer
    awaitingPlayerDeallocation[removedPlayerID].erase(inMsg.playerID);

    // Once every other player has confirmed that they removed the disconnected player locally,
    // send the remaining players confirmation that the removal was complete. The disconnected
    // player fully left the world.
    if(awaitingPlayerDeallocation[removedPlayerID].empty()){
        // Fire the player leave event and tell other players that the specified player has successfuly left.
        sendWorldPlayerLeaveMessage(removedPlayerID);

        // Remove the removed player's key from the waiting buffer
        awaitingPlayerDeallocation.erase(removedPlayerID);

        // Free the removed player's ID
        freePlayerID(removedPlayerID);
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

    // Give the player a reference to what zone they are in so they can load stuff in
    player->setCurrentZone(zone);

    if(zone->playersInZone.empty()){
        // If there are no players in the zone, add the loading player and
        // move on to loading entities
        zone->addPlayer(player);

        // Tell player to add themselves to their zone's player list.
        rControlMsg msg{};
        msg.msgType = rMessageType::ADD_ZONE_PLAYERS_COMPLETE;
        msg.playerID = player->getClientID();
        Bedrock::sendToClient(msg, player->getClientID());
    }else{
        // Make player locally populate their zone's player list with the server's copy
        player->addAllZonePlayers();
    }
}

void rWorld::ssAddZonePlayerAcknowledge(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Confirm that the loading player added the previously requested player to their local zone
    rPlayer* loadingPlayer = playerByPlayerID[inMsg.playerID];
    loadingPlayer->confirmZonePlayerAdd(inMsg.zonePlayerAdded);

    // Once the loading player adds all other players to the zone's player list, add the loading player
    // as well and tell them to do the same locally
    if(loadingPlayer->getFlagAddedZonePlayers()){
        // Add loading player to zone player list
        rZone* targetZone = loadingPlayer->getCurrentZone();
        targetZone->addPlayer(loadingPlayer);

        // Tell player to add themselves to their zone's player list.
        rControlMsg msg{};
        msg.msgType = rMessageType::ADD_ZONE_PLAYERS_COMPLETE;
        Bedrock::sendToClient(msg, loadingPlayer->getClientID());
    }
}

void rWorld::ssAddZonePlayersCompleteAcknowledge(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Now start loading entities that are in the zone for this player
    rPlayer* player = playerByPlayerID[inMsg.playerID];
    rZone* targetZone = player->getCurrentZone();

    if(targetZone->entitiesInZone.empty()){
        // If there are no entities to load, skip to completing the player load
        targetZone->sendZonePlayerLoadMessage(player->getPlayerID());
    }else{
        player->loadEntitiesInCurrentZone();
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

void rWorld::ssPlayerUnloadedZone(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the player and the zone that the player has left
    rZone *targetZone = rZoneRegistry::getInstance().getZoneByID(inMsg.zoneID);
    rPlayer *leavingPlayer = playerByPlayerID[inMsg.playerID];

    // Make sure the zone with the provided ID exists
    if (targetZone) {
        // Remove the player from the zone
        targetZone->removePlayer(leavingPlayer);

        // If there are no more players (after removing the specified one), then just uninstantiate the zone
        if(targetZone->playersInZone.empty()){
            targetZone->uninstantiateZone();
            return;
        }

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
        case rMessageType::ALLOCATE_PLAYER_ACKNOWLEDGE:
            ssAllocatePlayerAcknowledge(inMsg, outMsg);
            break;
        case rMessageType::ASSIGN_PLAYER_ID_ACKNOWLEDGE:
            ssAssignPlayerIDAcknowledge(inMsg, outMsg);
            break;
        case rMessageType::REMOVE_PLAYER_FROM_WORLD_ACKNOWLEDGE:
            ssRemovePlayerFromWorldAcknowledge(inMsg, outMsg);
            break;
        case rMessageType::ADD_ZONE_PLAYER_ACKNOWLEDGE:
            ssAddZonePlayerAcknowledge(inMsg, outMsg);
            break;
        case rMessageType::ADD_ZONE_PLAYERS_COMPLETE_ACKNOWLEDGE:
            ssAddZonePlayersCompleteAcknowledge(inMsg, outMsg);
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

    // Locally add the local player to the world's list of players in the world (by ID only)
    playerByPlayerID[playerID] = localPlayer;

    // Send ID assignment acknowledgement to the server
    inMsg.msgType = rMessageType::ASSIGN_PLAYER_ID_ACKNOWLEDGE;
    Bedrock::serializeType(inMsg, outMsg);
}

void rWorld::csAllocatePlayer(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the player ID for the player that needs a player object instance to be allocated
    PlayerID playerID = inMsg.playerID;

    // Create a new player object instance
    auto player = new rPlayer;
    player->setPlayerID(playerID);
    player->setCurrentWorld(this);

    // Store this player
    playerByPlayerID[playerID] = player;

    //Add the new player info to the zone's list players
    // TODO move localPlayer->getCurrentZone()->addPlayer(player);

    //Send a player object allocation acknowledgement back to the server
    inMsg.msgType = rMessageType::ALLOCATE_PLAYER_ACKNOWLEDGE;
    inMsg.playerID = localPlayer->getPlayerID();
    inMsg.allocatedPlayer = player->getPlayerID();
    Bedrock::serializeType(inMsg, outMsg);
}

void rWorld::csWorldJoinComplete(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    PlayerID playerID = inMsg.playerID; // Player ID that of player that joined the world

    if(localPlayer->getPlayerID() == playerID){
        // Fire the join world event for this local client only.
        onWorldJoin.invoke();
    }else{
        // Locally allocate the new player that has joined the world
        // The new player has already allocated all other players, but every other player is
        // just finding out about the new player right here
        auto player = new rPlayer;
        player->setPlayerID(playerID);
        player->setCurrentWorld(this);

        // Store the new player
        playerByPlayerID[playerID] = player;
    }

    // Fire the player join world event with the new player (should be fired for all clients)
    onWorldPlayerJoin.invoke(playerID);
}

void rWorld::csRemovePlayerFromWorld(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    PlayerID playerID = inMsg.playerID;
    rPlayer* player = playerByPlayerID[playerID];

    // Remove the player
    removePlayerFromWorld(player);

    // Send player removal ack to the server
    inMsg.msgType = rMessageType::REMOVE_PLAYER_FROM_WORLD_ACKNOWLEDGE;
    inMsg.playerID = localPlayer->getPlayerID();
    inMsg.removedPlayer = playerID;
    Bedrock::serializeType(inMsg, outMsg);
}

void rWorld::csWorldLeaveComplete(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Fire the player left world event
    onWorldPlayerLeave.invoke(inMsg.playerID);
}

void rWorld::csLoadZoneRequest(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the zone id that the server wants instantiated
    ZoneID zoneID = inMsg.zoneID;
    rZone *zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Make sure requested zone exists
    if (zone) {
        // Try to instantiate the zone
        zone->instantiateZone();

        // Give the player a reference to what zone they are in so they can load stuff in
        localPlayer->setCurrentZone(zone);

        // Acknowledge that the zone has been loaded
        inMsg.msgType = rMessageType::LOAD_ZONE_ACKNOWLEDGE;
        inMsg.playerID = localPlayer->getPlayerID();
        Bedrock::serializeType(inMsg, outMsg);
    } else {
        rDebug::err("[ERR] Could not locate the requested zone by given ID!");
    }
}

void rWorld::csAddZonePlayer(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Add the requested player to the zone'
    PlayerID requestedPlayerID = inMsg.playerID;
    rPlayer* playerToAdd = playerByPlayerID[requestedPlayerID];
    localPlayer->getCurrentZone()->addPlayer(playerToAdd);

    // Send acknowledgement to the server
    inMsg.msgType = rMessageType::ADD_ZONE_PLAYER_ACKNOWLEDGE;
    inMsg.playerID = localPlayer->getPlayerID();
    inMsg.zonePlayerAdded = requestedPlayerID;
    Bedrock::serializeType(inMsg, outMsg);
}

void rWorld::csAddZonePlayersComplete(rControlMsg &inMsg, Bedrock::Message &outMsg) {

    // Add local player to their current zone's player list
    localPlayer->getCurrentZone()->addPlayer(localPlayer);

    // Send acknowledgment to server
    inMsg.msgType = rMessageType::ADD_ZONE_PLAYERS_COMPLETE_ACKNOWLEDGE;
    inMsg.playerID = localPlayer->getPlayerID();
    Bedrock::serializeType(inMsg, outMsg);
}

void rWorld::csLoadEntityRequest(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    rDebug::log("Create entity request recieved!");
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

void rWorld::csLoadZoneComplete(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the specified zone that was loaded
    rZone* targetZone = localPlayer->getCurrentZone();

    // Fire the zone's player loaded event
    targetZone->onZonePlayerLoad.invoke(inMsg.playerID);

    // If the player that loaded in was this local player, then fire the local version of the event
    if (inMsg.playerID == localPlayer->getPlayerID()) {
        onZoneLoad.invoke();
        rDebug::log("Loaded into zone!");
    }
}

void rWorld::csPlayerUnloadedZone(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the zone object being left and the leaving player
    rZone *targetZone = rZoneRegistry::getInstance().getZoneByID(inMsg.zoneID);
    rPlayer *leavingPlayer = playerByPlayerID[inMsg.playerID];

    rDebug::log("Is zone null? %p", targetZone);
    rDebug::log("Is player null? %p", leavingPlayer);

    // Remove the player from the zone
    targetZone->removePlayer(leavingPlayer);
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
        case rMessageType::ALLOCATE_PLAYER:
            csAllocatePlayer(inMsg, outMsg);
            break;
        case rMessageType::WORLD_JOIN_COMPLETE:
            csWorldJoinComplete(inMsg, outMsg);
            break;
        case rMessageType::REMOVE_PLAYER_FROM_WORLD:
            csRemovePlayerFromWorld(inMsg, outMsg);
            break;
        case rMessageType::WORLD_LEAVE_COMPLETE:
            csWorldLeaveComplete(inMsg, outMsg);
            break;
        case rMessageType::ADD_ZONE_PLAYER:
            csAddZonePlayer(inMsg, outMsg);
            break;
        case rMessageType::ADD_ZONE_PLAYERS_COMPLETE:
            csAddZonePlayersComplete(inMsg, outMsg);
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
    localPlayer->setCurrentWorld(this);

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

//    //Check for a zone to unload, and unload it if there is one
//    rZone *loadedZone = localPlayer->getCurrentZone();
//    if(loadedZone){
//        //Remove the player from the zone locally
//        loadedZone->removePlayer(localPlayer);
//
//        //Destroy the zone locally (for now)
//        loadedZone->uninstantiateZone();
//
//        //Inform host
//        rControlMsg msg{};
//        msg.msgType = rMessageType::PLAYER_UNLOADED_ZONE;
//        msg.playerID = localPlayer->getPlayerID();
//        msg.zoneID = localPlayer->getCurrentZone()->getZoneID();
//
//        Bedrock::sendToHost(msg);
//    }

    // Clear map that stores players by their client ID
    playerByClientID.clear();

    // Deallocate all stored players (THIS INCLUDES THE LOCAL PLAYER!! don't double delete :p)
    for(auto& pair : playerByPlayerID){
        delete pair.second;
    }
    playerByPlayerID.clear();

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
    rDebug::log("GRRRRR TELL ME WHYYYY");
    // Try to get the zone by ID
    rZone* zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Proceed to load the zone if it was found from the registry, otherwise return appropriate error
    if(zone){
        rDebug::log("GRRRRR TELL ME WHYYYY == END");
        return loadZone(zone);
    }else{
        rDebug::err("Zone could not be found to load by requested zone ID!");
        rDebug::log("GRRRRR TELL ME WHYYYY == END");
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
    rDebug::log("GAHHHHH STOP");
    // Try to get the zone by ID
    rZone* zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Proceed to unload the zone if it was found from the registry, otherwise return appropriate error
    if(zone){
        rStatusCode code = unloadZone(zone);
        rDebug::log("AHWIDUABWDIWUAHB %d",(int)code);
        rDebug::log("GAHHHHH STOP == END");
        return code;
    }else{
        rDebug::err("Zone could not be found to unload by requested zone ID!");
        rDebug::log("GAHHHHH STOP == END");
        return rStatusCode::ZONE_WITH_PROVIDED_ID_NOT_FOUND;
    }


}

