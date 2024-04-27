#include "regolith/regolith.h"

void rPlayer::clearZoneInfo() {
    p_currentZone = nullptr;
    ownedEntities.clear();
}