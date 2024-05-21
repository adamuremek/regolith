#include "regolith/regolith.h"

void rPlayer::clearZoneInfo() {
    p_currentZone = nullptr;
    ownedEntities.clear();
}

void rPlayer::loadEntitiesInCurrentZone() {
    // Make the player start loading all entities in the zone by initiating the first entity load requests.
    // This will start a chain of requests that eventually loads all entities on the player's end.
    for(const auto& pair : p_currentZone->entitiesInZone){
        loadEntity(pair.second);
    }
}

void rPlayer::loadEntity(rEntity *entity) {
    // Create a message to request the creation of the entity on this player's end
    rControlMsg msg;
    msg.msgType = rMessageType::CREATE_ENTITY_REQUEST;
    msg.entityInfo = entity->getEntityInfo();

    // Add the entity (instance ID) to the ACK waiting buffer
    awaitingEntityLoad.insert(entity->getInstanceID());

    // Send the message to the end client
    Bedrock::sendToClient(msg, clientID);
}

void rPlayer::confirmEntityLoaded(EntityInstanceID entityInstanceID) {
    // Remove the entity from the ACK buffer
    awaitingEntityLoad.erase(entityInstanceID);

    // Check if all the entities in the zone have been loaded for this player
    if(awaitingEntityLoad.empty() && !flagLoadedAllEntitiesInZone){
        // Mark that this player has loaded all entities in the zone
        flagLoadedAllEntitiesInZone = true;
    }
}

void rPlayer::addAllZonePlayers() {
    for(const auto& pair : p_currentZone->playersInZone){
        addZonePlayer(pair.second);
    }
}

void rPlayer::addZonePlayer(rPlayer *player) {
    // Create a message that tells this player to add the requested player to the zone
    rControlMsg msg;
    msg.msgType = rMessageType::ADD_ZONE_PLAYER;
    msg.playerID = player->getPlayerID();

    // Add the requested player to the ACK waiting buffer
    awaitingZonePlayerAdd.insert(player->getPlayerID());

    // Send the message to this player
    Bedrock::sendToClient(msg, clientID);
}

void rPlayer::confirmZonePlayerAdd(PlayerID addedZonePlayerID) {
    // Remove the added player from buffer
    awaitingZonePlayerAdd.erase(addedZonePlayerID);

    if(awaitingZonePlayerAdd.empty() && !flagAddedZonePlayers){
        // Mark that all players that are in the zone have been added locally
        flagAddedZonePlayers = true;
    }
}

void rPlayer::allocatePlayer(rPlayer *player) {
    // Create a message to tell this end client/player to allocate information for the incoming player
    rControlMsg msg;
    msg.msgType = rMessageType::ALLOCATE_PLAYER;
    msg.playerID = player->getPlayerID();

    // Add incoming player (player ID) to the ACK waiting buffer
    awaitingPlayerAllocation.insert(player->getPlayerID());

    // Send message to the end client
    Bedrock::sendToClient(msg, clientID);
}

void rPlayer::confirmPlayerAllocation(PlayerID allocatedPlayerID) {
    // Remove the player from the buffer
    awaitingPlayerAllocation.erase(allocatedPlayerID);

    // Once the ACK buffer is empty and the player has made player object allocations for all other
    // players in the world, flag that this player has loaded everyone in.
    if(awaitingPlayerAllocation.empty() && !flagAllocatedPlayersInWorld){
        // Mark that the player has loaded all other players in the zone locally
        flagAllocatedPlayersInWorld = true;
    }
}


