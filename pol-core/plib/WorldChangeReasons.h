#ifndef PLIB_WORLDCHANGEREASON_H
#define PLIB_WORLDCHANGEREASON_H

namespace Pol {
    namespace Plib {
        enum class WorldChangeReason {
            NpcLoad,
            NpcCreate,
            NpcDeath,

            PlayerLoad,
            PlayerEnter,
            PlayerExit,
            PlayerDeleted,

            Moved
        };
    }
}
#endif