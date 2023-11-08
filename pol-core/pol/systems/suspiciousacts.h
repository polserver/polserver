#ifndef H_SYSTEMS_SUSPICIOUSACTS
#define H_SYSTEMS_SUSPICIOUSACTS

#include "base/position.h"
#include "clib/rawtypes.h"

namespace Pol::Network
{
class Client;
}

// For now the reporting is done with static functions, so a namespace is fine. We can make this a
// class later if needed.
namespace Pol::SuspiciousActs
{
void GumpResponseWasUnexpected( Pol::Network::Client* client, u32 gumpid, u32 buttonid );
void GumpResponseHasTooManyInts( Network::Client* client );
void GumpResponseHasTooManyIntsOrStrings( Network::Client* client );
void GumpResponseOverflows( Network::Client* client );

void DropItemButNoneGotten( Network::Client* client, u32 item_serial );
void DropItemOutOfRange( Network::Client* client, u32 item_serial );
void DropItemOutAtBlockedLocation( Network::Client* client, u32 item_serial,
                                   const Core::Pos3d& pos );

void DropItemOtherThanGotten( Network::Client* client, u32 dropped_item_serial,
                              u32 gotten_item_serial );

void EquipItemButNoneGotten( Network::Client* client, u32 equipped_item_serial );
void EquipItemOtherThanGotten( Network::Client* client, u32 equipped_item_serial,
                               u32 gotten_item_serial );

void OutOfSequenceCursor( Network::Client* client );
}  // namespace Pol::SuspiciousActs

#endif  // !H_SYSTEMS_SUSPICIOUSACTS
