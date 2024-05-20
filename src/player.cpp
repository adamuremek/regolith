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
    entitiesWaitingForLoadAck.insert(entity->getInstanceID());

    // Send the message to the end client
    Bedrock::sendToClient(msg, clientID);
}

void rPlayer::confirmEntityLoaded(EntityInstanceID entityInstanceID) {
    // Remove the entity from the ACK buffer
    entitiesWaitingForLoadAck.erase(entityInstanceID);

    // Check if all the entities in the zone have been loaded for this player
    if(entitiesWaitingForLoadAck.empty() && !flagLoadedAllEntitiesInZone){
        // Mark that this player has loaded all entities in the zone
        flagLoadedAllEntitiesInZone = true;

        // Inform all players in the zone that this player has fully loaded into the zone
        rControlMsg msg;
        msg.msgType = rMessageType::LOAD_ZONE_COMPLETE;
        msg.playerID = playerID;
        msg.zoneID = p_currentZone->getZoneID();

        for (const auto& pair : p_currentZone->playersInZone) {
            Bedrock::sendToClient(msg, pair.second->getClientID());
        }

        // Fire the zone's player loaded event server side
        p_currentZone->onPlayerLoadedZone.invoke(playerID);
    }
}

void rPlayer::loadPlayer(rPlayer *player) {
    // Create a message to tell this end client/player to allocate information for the incoming player
    rControlMsg msg;
    msg.msgType = rMessageType::ALLOCATE_INCOMING_PLAYER;
    msg.playerID = player->getPlayerID();

    // Add incoming player (player ID) to the ACK waiting buffer
    playersWaitingForLoadAck.insert(player->getPlayerID());

    // Send message to the end client
    Bedrock::sendToClient(msg, clientID);
}

void rPlayer::confirmPlayerLoaded(PlayerID loadedPlayerID) {
    // Remove the player from the ACK buffer
    playersWaitingForLoadAck.erase(loadedPlayerID);

    // Once the ACK buffer is empty and the player has made player info allocations for all other
    // players in the zone, start loading in all the entities in the zone.
    if(playersWaitingForLoadAck.empty() && !flagLoadedInOtherPlayers){
        // Mark that the player has loaded all other players in the zone locally
        flagLoadedInOtherPlayers = true;

        // Load all entities in the zone
        loadEntitiesInCurrentZone();
    }
}


