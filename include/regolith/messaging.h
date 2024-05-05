#ifndef REGOLITH_MESSAGING_H
#define REGOLITH_MESSAGING_H


BedrockMsgDatatype(rEntityInfo){
    EntityID entityID{0};
    EntityInstanceID instanceID{0};
    PlayerID owner{0};
    ZoneID parentZone{0};

    rEntityInfo(){
        registerMember(&entityID);
        registerMember(&instanceID);
        registerMember(&owner);
        registerMember(&parentZone);
    }

    bool verifyDataIntegrity(){
    }
};

BedrockMsgDatatype(rControlMsg){
    MessageType msgType{MessageType::NONE};
    PlayerID playerID{0};
    PlayerID allocatedPlayer{0};
    ZoneID zoneID{0};
    rEntityInfo entityInfo;


    rControlMsg(){
        registerMember(&msgType);
        registerMember(&playerID);
        registerMember(&zoneID);
        registerBedrockMsgDatatypeMember(&entityInfo);
    }
};




#endif //REGOLITH_MESSAGING_H
