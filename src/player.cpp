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
    msg.msgType = MessageType::CREATE_ENTITY_REQUEST;
    msg.entityInfo = entity->getEntityInfo();

    // Add the entity (instance ID) to the ACK waiting buffer
    entitiesWaitingForLoadAck.insert(entity->getInstanceID());

    // Send the message to the end client
    Bedrock::sendToClient(msg, clientID);
}

void rPlayer::loadPlayer(rPlayer *player) {
    // Create a message to tell this end client/player to allocate information for the incoming player
    rControlMsg msg;
    msg.msgType = MessageType::ALLOCATE_INCOMING_PLAYER;
    msg.playerID = player->getPlayerID();

    // Add incoming player (player ID) to the ACK waiting buffer
    playersWaitingForLoadAck.insert(player->getPlayerID());

    // Send message to the end client
    Bedrock::sendToClient(msg, clientID);
}