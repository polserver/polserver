#ifndef PLIB_WORLDCHANGEREASON_H
#define PLIB_WORLDCHANGEREASON_H

namespace Pol {
    namespace Plib {
        enum class WorldChangeReason {
            ClientEnter,
            ClientExit,
            NpcCreate,
            NpcLoad,
            NpcDeath,
            Moved,
        };
    }
}
#endif