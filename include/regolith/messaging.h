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
    //TODO implement this shit
        return true;
    }
};

BedrockMsgDatatype(rControlMsg){
    rMessageType msgType{rMessageType::NONE};
    PlayerID playerID{0};
    PlayerID allocatedPlayer{0};
    PlayerID removedPlayer{0};
    PlayerID zonePlayerAdded{0};
    ZoneID zoneID{0};
    rEntityInfo entityInfo;


    rControlMsg(){
        registerMember(&msgType);
        registerMember(&playerID);
        registerMember(&allocatedPlayer);
        registerMember(&removedPlayer);
        registerMember(&zonePlayerAdded);
        registerMember(&zoneID);
        registerBedrockMsgDatatypeMember(&entityInfo);
    }
};




#endif //REGOLITH_MESSAGING_H
