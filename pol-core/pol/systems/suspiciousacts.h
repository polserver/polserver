#ifndef H_SYSTEMS_SUSPICIOUSACTS
#define H_SYSTEMS_SUSPICIOUSACTS

#include "pol/base/position.h"
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

// boat movement packet without being on a multi
void BoatMoveNoMulti( Network::Client* client );

// boat movement packet without being on a boat multi
void BoatMoveNotBoatMulti( Network::Client* client );

// boat movement packet sent while on a boat that character is not a pilot of
void BoatMoveNotPilot( Network::Client* client, u32 multi_serial );

// boat movement packet used packet with direction > 7 or speed > 2
void BoatMoveOutOfRangeParameters( Network::Client* client, u32 multi_serial, u8 direction,
                                   u8 speed );

// character attempted to move while piloting a boat
void CharacterMovementWhilePiloting( Network::Client* client );

// sell window answer sent with no sell window open
void SellAnswerWithoutWindow( Network::Client* client, u32 vendor_serial );

// sell window answer naming items the window did not list, or naming one more than once
void SellAnswerNotListed( Network::Client* client, u32 vendor_serial, unsigned count );

// sell window answer whose count claims more entries than its length holds
void SellAnswerOverflows( Network::Client* client, unsigned claimed, unsigned held );
}  // namespace Pol::SuspiciousActs

#endif  // !H_SYSTEMS_SUSPICIOUSACTS
