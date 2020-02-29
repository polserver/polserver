/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib: Initial creation.
 *
 * @note This file is for packet structs who sent by both client and server.
 */


#ifndef __PKTBOTHIDH
#define __PKTBOTHIDH
namespace Pol
{
namespace Core
{
enum PKTBOTHID : unsigned char
{
  PKTBI_22_APPROVED_ID = 0x22,  // Move Approved/Resync Request.
  PKTBI_22_SYNC_ID = 0x22,      // Move Approved/Resync Request.
  PKTBI_2C_ID = 0x2C,           // Ressurection Menu

  PKTBI_3A_ID = 0x3A,
  PKTBI_3A_VALUES_ID = 0x3A,  // Send Skills
  PKTBI_3A_LOCKS_ID = 0x3A,   // Send Skills
  PKTBI_3A_CAPS_ID = 0x3A,    // Send Skills
  PKTBI_3B_ID = 0x3B,         // Buy Items

  PKTBI_56_ID = 0x56,  // Map Pin

  PKTBI_66_ID = 0x66,  // Old Book Packet
  PKTBI_6C_ID = 0x6C,  // Target Cursor
  PKTBI_6F_ID = 0x6F,  // Secure Trading Window

  PKTBI_71_ID = 0x71,  // Bulliten Board Messages
  PKTBI_72_ID = 0x72,  // War mode
  PKTBI_73_ID = 0x73,  // Ping Message

  PKTBI_93_ID = 0x93,      // Open Book
  PKTBI_95_ID = 0x95,      // Dye/Color Selection
  PKTBI_98_IN_ID = 0x98,   // All Names Macro FIXME: This is a nasty kludge due to message handler
                           // using struct name+_id to get this. Boooooooo.
  PKTBI_98_OUT_ID = 0x98,  // All Names Macro FIXME: This is a nasty kludge due to message handler
                           // using struct name+_id to get this. Boooooooo.
  PKTBI_99_ID = 0x99,      // Multi Placement View
  PKTBI_9A_ID = 0x9A,      // Console Entry Prompt

  PKTBI_B8_IN_ID = 0xB8,   // Character Profile
  PKTBI_B8_OUT_ID = 0xB8,  // Character Profile
  PKTBI_BB_ID = 0xBB,      // Ultima Messenger (Old school mail system)
  PKTBI_BD_ID = 0xBD,      // Client Version Info
  PKTBI_BF_ID = 0xBF,      // Generic Commands

  PKTBI_C2_ID = 0xC2,  // Text Entry Gump Unicode
  PKTBI_C8_ID = 0xC8,  // Client's Range it is using for "viewable"

  PKTBI_D1_ID = 0xD1,      // Logout Status
  PKTBI_D6_OUT_ID = 0xD6,  // Object Property List, Outgoing
  PKTBI_D6_IN_ID = 0xD6,   // Request Tooltips, Incoming
  PKTBI_D7_ID = 0xD7,      // Generic Commands (AOS Era)
  PKTBI_F0_ID = 0xF0,      // Krrios Client Packets


};
}
}  // namespace Pol
#endif
