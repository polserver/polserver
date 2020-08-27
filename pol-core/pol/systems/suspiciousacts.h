#ifndef H_SYSTEMS_SUSPICIOUSACTS
#define H_SYSTEMS_SUSPICIOUSACTS

#include "clib/rawtypes.h"

namespace Pol::Network
{
class Client;
}

// For now the reporting is done with static functions, so a namespace is fine. We can make this a class later if needed.
namespace Pol::SuspiciousActs
{
void UnexpectedGumpResponse( Pol::Network::Client* client, u32 gumpid, u32 buttonid );
void DropItemButNoneGotten( Network::Client* client, u32 item_serial );
void DropItemOtherThanGotten( Network::Client* client, u32 dropped_item_serial,
                                u32 gotten_item_serial );
void EquipItemButNoneGotten( Network::Client* client, u32 equipped_item_serial );
void EquipItemOtherThanGotten( Network::Client* client, u32 equipped_item_serial,
                               u32 gotten_item_serial );
void OutOfSequenceCursor( Network::Client* client );
}

#endif  // !H_SYSTEMS_SUSPICIOUSACTS
