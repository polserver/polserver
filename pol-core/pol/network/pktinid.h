/** @file
 *
 * @par History
 * - 2009/07/26 MuadDib:   Initial creation.
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 *
 * @note This file is for packet structs who sent by client.
 */


#ifndef __PKTINIDH
#define __PKTINIDH

namespace Pol::Core
{
enum PKTINID
{
  PKTIN_00_ID = 0x00,     // Create Character
  PKTIN_01_ID = 0x01,     // Logout Status
  PKTIN_02_ID = 0x02,     // Move Request
  PKTIN_03_ID = 0x03,     // Request Talk
  PKTIN_04_ID = 0x04,     // Request God Mode
  PKTIN_05_ID = 0x05,     // Request Attack
  PKTIN_06_ID = 0x06,     // Double Click
  PKTIN_07_ID = 0x07,     // Pick Up Item Request
  PKTIN_08_V1_ID = 0x08,  // Drop Item
  PKTIN_08_V2_ID = 0x08,  // Drop Item - Struct introduced with client 6.0.1.7 approximately
  PKTIN_09_ID = 0x09,     // Single Click

  PKTIN_12_ID = 0x12,  // Extended Actions Message
  PKTIN_13_ID = 0x13,  // Drop->Wear Item
  PKTIN_1E_ID = 0x1E,  // Control Animation

  PKTIN_34_ID = 0x34,  // Request Stats

  PKTIN_5D_ID = 0x5D,  // Character Selected

  PKTIN_75_ID = 0x75,  // Rename Character
  PKTIN_7D_ID = 0x7D,  // Menu Selection

  PKTIN_80_ID = 0x80,  // Login
  PKTIN_83_ID = 0x83,  // Delete Character
  PKTIN_8D_ID = 0x8D,  // KR Era Create Character

  PKTIN_91_ID = 0x91,  // Login To Game server
  PKTIN_9B_ID = 0x9B,  // Request Help
  PKTIN_9F_ID = 0x9F,  // Sell Items

  PKTIN_A0_ID = 0xA0,  // Server Selection
  PKTIN_A4_ID = 0xA4,  // Client Info
  PKTIN_A7_ID = 0xA7,  // Get Tip Window
  PKTIN_AC_ID = 0xAC,  // Text Entry Gump Reply
  PKTIN_AD_ID = 0xAD,  // Unicode Speech Request

  PKTIN_B1_ID = 0xB1,  // Gump Reply
  PKTIN_B5_ID = 0xB5,  // Chat Button
  PKTIN_B6_ID = 0xB6,  // Request Tooltip
  PKTIN_D9_ID = 0xD9,  // Client Machine's Details
  PKTIN_E1_ID = 0xE1,  // Client Type (UO3D)
  PKTIN_E4_ID = 0xE4,  // KR Encryption Request
  PKTIN_EF_ID = 0xEF,  // Seed Packet, introduced 6.0.5.0
  PKTIN_F8_ID = 0xF8,  // Create Character packet since 7.0.16.0
  PKTIN_FA_ID = 0xFA,  // Open UO Store
  PKTIN_FB_ID = 0xFB,  // Update View Public House Contents

};
}

#endif
