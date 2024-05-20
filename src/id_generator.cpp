#include "regolith/regolith.h"

PlayerID IDGenerator::generatePlayerID() {
    unsigned int newID;

    if (!freePlayerIDs.empty()) {
        // If there exists a free player id, use the first one on the stack as the new id
        newID = freePlayerIDs.top();
        freePlayerIDs.pop();
    } else {
        // Keep incrementing the id counter until a suitible unused int is found. Add it to
        // the used list and return it.
        do {
            newID = s_playerIDCounter++;
        } while (usedPlayerIDs.find(newID) != usedPlayerIDs.end());
    }

    usedPlayerIDs.insert(newID);
    return newID;
}

EntityInstanceID IDGenerator::generateEntityInstanceID() {
    unsigned int newID;

    if (!freeNetworkEntityIDs.empty()) {
        // If there exists a free network entity id, use the first one on the stack as the new id
        newID = freeNetworkEntityIDs.top();
        freeNetworkEntityIDs.pop();
    } else {
        // Keep incrementing the id counter until a suitible unused int is found. Add it to
        // the used list and return it.
        do {
            newID = s_networkEntityIDCounter++;
        } while (usedNetworkEntityIDs.find(newID) != usedNetworkEntityIDs.end());
    }

    usedNetworkEntityIDs.insert(newID);
    return newID;
}

void IDGenerator::freePlayerID(const PlayerID &playerID) {
    // Move the id from used to free.
    usedPlayerIDs.erase(playerID);
    freePlayerIDs.push(playerID);
}

void IDGenerator::freeEntityInstanceID(const EntityInstanceID &entityInstanceId) {
    //Move the id from used to free.
    usedNetworkEntityIDs.erase(entityInstanceId);
    freeNetworkEntityIDs.push(entityInstanceId);
}

void IDGenerator::reclaimPlayerIDs() {
    // Clear stack with free player IDs
    while(!freePlayerIDs.empty()){
         freePlayerIDs.pop();
    }

    // Clear stack with used player IDs
    usedPlayerIDs.clear();

    // Reset player ID counter
    s_playerIDCounter = 1;
}