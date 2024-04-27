#ifndef REGOLITH_MESSAGING_H
#define REGOLITH_MESSAGING_H

BedrockMsgDatatype(ControlMsg){
    MessageType msgType{MessageType::NONE};
    PlayerID playerID{0};
    ZoneID zoneID{0};


    ControlMsg(){
        registerMember(&msgType);
        registerMember(&playerID);
        registerMember(&zoneID);
    }
};

BedrockMsgDatatype(rEntityInfo){
    EntityInstanceID instanceID;
};


#endif //REGOLITH_MESSAGING_H
