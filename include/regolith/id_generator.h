#ifndef REGOLITH_ID_GENERATOR_H
#define REGOLITH_ID_GENERATOR_H

class IDGenerator {
private:
    IDGenerator(){}

    std::stack<unsigned int> freePlayerIDs;
    std::stack<unsigned int> freeNetworkEntityIDs;
    std::unordered_set<unsigned int> usedPlayerIDs;
    std::unordered_set<unsigned int> usedNetworkEntityIDs;

    unsigned int s_playerIDCounter{1};
    unsigned int s_networkEntityIDCounter{1};

public:
    // Delete copy constructor and assignment operator to prevent copying
    IDGenerator(const IDGenerator&) = delete;
    IDGenerator& operator=(const IDGenerator&) = delete;

    // Singleton accessor
    static IDGenerator& getInstance(){
        static IDGenerator instance;
        return instance;
    }

    PlayerID generatePlayerID();
    EntityInstanceID generateEntityInstanceID();
    void freePlayerID(const PlayerID& playerID);
    void freeEntityInstanceID(const EntityInstanceID& entityInstanceId);
};


inline PlayerID generatePlayerID() { return IDGenerator::getInstance().generatePlayerID(); }
inline EntityInstanceID generateEntityInstanceID() { return IDGenerator::getInstance().generateEntityInstanceID(); }
inline void freePlayerID(const PlayerID& playerID) { IDGenerator::getInstance().freePlayerID(playerID); }
inline void freeEntityInstanceID(const EntityInstanceID& entityInstanceID) {
    IDGenerator::getInstance().freeEntityInstanceID(entityInstanceID);
}

#endif //REGOLITH_ID_GENERATOR_H
