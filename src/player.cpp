#include "regolith/regolith.h"

void rPlayer::clearZoneInfo() {
    p_currentZone = nullptr;
    ownedEntities.clear();
}

void rPlayer::addOwnedEntity(rEntity *ownedEntity) {
    ownedEntities[ownedEntity->getInstanceID()] = ownedEntity;
}

void rPlayer::removeOwnedEntity(rEntity *ownedEntity) {
    EntityInstanceID instanceID = ownedEntity->getInstanceID();
    if(ownedEntities.find(instanceID) != ownedEntities.end()){
        ownedEntities.erase(instanceID);
    }
}