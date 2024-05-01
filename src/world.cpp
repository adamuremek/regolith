#include "regolith/regolith.h"


void rWorld::playerConnected(const Bedrock::ClientID &clientID) {
    //Make sure the connecting player isn't already connected (should be very rare, but just in case :D )
    if (playerByClientID.find(clientID) != playerByClientID.end()) {
        return;
    }

    //Allocate player info
    auto *newPlayer = new rPlayer;

    //Generate a network identifier for the player
    PlayerID newPlayerID = generatePlayerID();

    //Populate player info
    newPlayer->setClientID(clientID);
    newPlayer->setPlayerID(newPlayerID);

    //Send the player their ID
    rControlMsg msg{};
    msg.msgType = MessageType::ASSIGN_PLAYER_ID;
    msg.playerID = newPlayerID;

    Bedrock::sendToClient(msg, clientID);

    //Add player to a map keyed by client id, and a map keyed by id
    playerByClientID[clientID] = newPlayer;
    playerByPlayerID[newPlayerID] = newPlayer;
}

void rWorld::playerDisconnected(const Bedrock::ClientID &clientID) {
    removePlayer(clientID);
}

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

            // Tell remaining players in zone to remove the player locally
            for (const auto &playerInZone: currentZone->playersInZone) {
                // Skip the leaving player in case they are encountered
                if (playerInZone.first == playerID) {
                    continue;
                }


                // Send the message to the endpoint player in the zone
                Bedrock::ClientID playerEndpoint = playerInZone.second->getClientID();

                rControlMsg msg{};
                msg.msgType = MessageType::PLAYER_LEFT_ZONE;
                msg.playerID = playerID;
                msg.zoneID = zoneID;

                Bedrock::sendToClient(msg, playerEndpoint);
            }
        }

        // Remove the player from the player-clientID maps
        playerByClientID.erase(player->getClientID());
        playerByPlayerID.erase(playerID);

        //TODO: Maybe force close connection if you can? For cases where the server wants to force disconnect client

        // Free player memory allocation
        delete player;
    }
}


void rWorld::ssLoadZoneRequest(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Get the zone requested by the player
    ZoneID zoneID = inMsg.zoneID;
    rZone *zone = rZoneRegistry::getInstance().getZoneByID(zoneID);

    // Try to instantiate the zone (if it hasn't already been)
    zone->instantiateZone();

    // Tell the player to load the zone locally on their end
    Bedrock::serializeType(inMsg, outMsg);
}


void rWorld::csAssignPlayerID(rControlMsg &inMsg, Bedrock::Message &outMsg) {
    // Assign playerID to local player
    PlayerID playerID = inMsg.playerID;
    localPlayer->setPlayerID(playerID);

    //Locally add the local player to the world's list of players in the world (by ID only)
    playerByPlayerID[playerID] = localPlayer;

    //Since this entire loop is running in a different thread from the main thread/game loop,
    //the signal has to be queued to be emitted at the next game loop call.
    call_deferred("emit_signal", "joined_world");
}


void rWorld::start_world(int port) {
    //TODO: maybe init bedrock here?
    if (!Bedrock::isInitialized) {
        return;
    }

    Bedrock::startDedicatedHost(port);
}

void rWorld::stop_world() {
    //TODO: need a way to shutdown the host without shutting down bedrock
}

void rWorld::joinWorld(const char *world, int port) {
    //Set client callbacks
    //TODO

    //Connect to world
    if (!Bedrock::startClient(port, world)) {
        return;
    }

    //Create local player info object to store info in
    localPlayer = new rPlayer;
}

void rWorld::leaveWorld() {
    if (Bedrock::BedrockMetadata::getInstance().isRole(ACTOR_NONE) ||
        Bedrock::BedrockMetadata::getInstance().isRole(ACTOR_SERVER)) {
        return;
    }

    //Check for a zone to unload, and unload it if there is one
    rZone *loadedZone = localPlayer->getCurrentZone();
    if(loadedZone){
        //Remove the player from the zone locally
        loadedZone->remove_player(m_localPlayer);

        //Destroy the zone locally (for now)
        loadedZone->uninstantiate_zone();
    }

    m_clientRunLoop = false;

    //Stop the listen loop
    if (m_clientListenThread.joinable()){
        m_clientListenThread.join();
    }

    //Stop the tick loop
    if(m_clientTickThread.joinable()){
        m_clientTickThread.join();
    }

    //Stop world connection
    SteamNetworkingSockets()->CloseConnection(m_worldConnection, 0, nullptr, false);
    m_worldConnection = k_HSteamNetConnection_Invalid;

    GDNet::singleton->m_isClient = false;

    //Inform signal connections that client has left the world
    emit_signal("left_world");
}
