#include "regolith/regolith.h"


void rWorld::player_connected(Bedrock::ClientID playerConnection) {

}


void rWorld::start_world(int port) {
    //TODO: maybe init bedrock here?
    if(!Bedrock::isInitialized){
        return;
    }

    Bedrock::startDedicatedHost(port);
}

void rWorld::stop_world() {
    //TODO: need a way to shutdown the host without shutting down bedrock
}
