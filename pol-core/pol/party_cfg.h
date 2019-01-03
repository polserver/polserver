/** @file
 *
 * @par History
 */


#ifndef PARTY_CFG_H
#define PARTY_CFG_H

#include "network/pktdef.h"
#include <string>

namespace Pol
{
namespace Core
{
class ExportedFunction;

struct Party_Cfg
{
  struct
  {
    unsigned short MaxPartyMembers;
    bool TreatNoAsPrivate;
    unsigned short DeclineTimeout;
    std::string PrivateMsgPrefix;
    bool RemoveMemberOnLogoff;
    bool RejoinPartyOnLogon;
  } General;

  struct
  {
    ExportedFunction* CanAddToParty;      // (leader,member)
    ExportedFunction* CanLeaveParty;      // (member
    ExportedFunction* CanRemoveMember;    // (leader,member)
    ExportedFunction* ChangePrivateChat;  // (member,tomember,uctext)
    ExportedFunction* ChangePublicChat;   // (member,uctext)
    ExportedFunction* OnAddToParty;       // (who)
    ExportedFunction* OnDecline;          // (who)
    ExportedFunction* OnDisband;          // (partyref)
    ExportedFunction* OnLeaveParty;       // (who,fromwho)
    ExportedFunction* OnLootPermChange;   // (who)
    ExportedFunction* OnPartyCreate;      // (party)
    ExportedFunction* OnPrivateChat;      // (member,tomember,uctext)
    ExportedFunction* OnPublicChat;       // (member,uctext)
  } Hooks;
};

const int CLP_Rejoined = 1005437;            // You have rejoined the party.
const int CLP_Add_Yourself = 1005439;        // You cannot add yourself to a party.
const int CLP_Already_Your_Party = 1005440;  // This person is already in your party!
const int CLP_Already_in_a_Party = 1005441;  // This person is already in a party!
const int CLP_Add_Living = 1005442;          // You may only add living things to your party!
const int CLP_Ignore_Offer = 1005444;        // The creature ignores your offer.
const int CLP_Added = 1005445;               // You have been added to the party.
const int CLP_Cannot_Remove_Self =
    1005446;  // You may only remove yourself from a party if you are not the leader.
const int CLP_Allow_Loot = 1005447;  // You have chosen to allow your party to loot your corpse.
const int CLP_Prevent_Loot =
    1005448;  // You have chosen to prevent your party from looting your corpse.
const int CLP_Disbanded = 1005449;       // Your party has disbanded.
const int CLP_Last_Person = 1005450;     // The last person has left the party...
const int CLP_Removed = 1005451;         // You have been removed from the party.
const int CLP_Player_Removed = 1005452;  // A player has been removed from your party.
const int CLP_Add_No_Leader =
    1005453;                         // You may only add members to the party if you are the leader.
const int CLP_Who_To_Add = 1005454;  // Who would you like to add to your party?
const int CLP_Who_to_Remove = 1005455;    // Who would you like to remove from your party?
const int CLP_Player_Rejoined = 1008087;  // : rejoined the party.
const int CLP_Invite = 1008089;   // : You are invited to join the party. Type /accept to join or
                                  // /decline to decline the offer.
const int CLP_Invited = 1008090;  // You have invited them to join the party.
const int CLP_Notify_Decline = 1008091;  // : Does not wish to join the party.
const int CLP_Decline = 1008092;         // You notify them that you do not wish to join the party.
const int CLP_Joined = 1008094;          // : joined the party.
const int CLP_Max_Size = 1008095;  // You may only have 10 in your party (this includes candidates).
const int CLP_No_Party = 3000211;  // You are not in a party.
const int CLP_No_Invite = 3000222;  // No one has invited you to be in a party.
const int CLP_Too_Late_Decline =
    3000223;  // Too late to decline, you are already in the party. Try /quit.
}  // namespace Core
}  // namespace Pol
#endif
