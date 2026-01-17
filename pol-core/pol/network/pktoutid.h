/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   Initial creation.
 * - 2009/08/01 MuadDib:   Completed moving all "Outbound only" Packet IDs to here.
 * - 2009/08/09 MuadDib:   Re factor of Packet 0x25, 0x11 for naming convention
 * - 2009/09/10 Turley:    CompressedGump support (Grin)
 * - 2009/12/02 Turley:    PKTOUT_F3 -Tomi
 * - 2012/04/14 Tomi:      PKTOUT_F5
 *
 * @note This file is for packet structs who sent by server.
 */


#ifndef __PKTOUTIDH
#define __PKTOUTIDH

namespace Pol::Core
{
enum PKTOUTID : unsigned char
{
  PKTOUT_0B_ID = 0x0B,  // Newer client damage display packet. since 4.0.7a client

  PKTOUT_11_ID = 0x11,  // Status Message
  PKTOUT_17_ID = 0x17,  // Health bar status update (KR)
  PKTOUT_1A_ID = 0x1A,  // Object Information
  PKTOUT_1B_ID = 0x1B,  // Character Startup - Login actually completed.
  PKTOUT_1C_ID = 0x1C,  // Send ASCII Speech
  PKTOUT_1D_ID = 0x1D,  // Delete/Remove Object
  PKTOUT_1F_ID = 0x1F,  // Explosion

  PKTOUT_20_ID = 0x20,  // Draw Mobile
  PKTOUT_21_ID = 0x21,  // Move Rejection
  PKTOUT_23_ID = 0x23,  // Item Drag Animation - used to make client side animation of an itemID
                        // going up from xyz, to down at xyz
  PKTOUT_24_ID = 0x24,  // Open Gump ID
  PKTOUT_25_ID = 0x25,  // Container
  PKTOUT_26_ID = 0x26,  // TODO: What clients even support this packet?
  PKTOUT_27_ID = 0x27,  // Move Item Failure
  PKTOUT_28_ID = 0x28,  // God Client Packet
  PKTOUT_29_ID = 0x29,  // KR Drop Success
  PKTOUT_2A_ID = 0x2A,  // Newer Client Blood Packet TODO: What clients even support this packet?
  PKTOUT_2B_ID = 0x2B,  // God Client Packet
  PKTOUT_2D_ID = 0x2D,  // Health
  PKTOUT_2E_ID = 0x2E,  // Worn Items
  PKTOUT_2F_ID = 0x2F,  // Fight Occuring

  PKTOUT_30_ID = 0x30,  // Attack Approved. TODO: What client versions use this?
  PKTOUT_31_ID = 0x31,  // Attack Ended. Older client packet
  PKTOUT_32_ID = 0x32,  // TODO: What packet is this?
  PKTOUT_33_ID = 0x33,  // Flow Control
  PKTOUT_36_ID = 0x36,  // Resource Tile Data - God Client
  PKTOUT_3C_ID = 0x3C,  // Container Contents
  PKTOUT_3E_ID = 0x3E,  // Versions - God Client
  PKTOUT_3F_ID = 0x3F,  // Update Statics - God Client

  PKTOUT_41_ID = 0x41,  // TODO: What packet is this?
  PKTOUT_42_ID = 0x42,  // TODO: What packet is this?
  PKTOUT_43_ID = 0x43,  // TODO: What packet is this?
  PKTOUT_44_ID = 0x44,  // TODO: What packet is this?
  PKTOUT_4E_ID = 0x4E,  // Personal Light Level
  PKTOUT_4F_ID = 0x4F,  // Overall Light Level

  PKTOUT_53_ID = 0x53,  // Idle Warning
  PKTOUT_54_ID = 0x54,  // Sound Effect
  PKTOUT_55_ID = 0x55,  // Login Complete On Server
  PKTOUT_5B_ID = 0x5B,  // Game Time


  PKTOUT_65_ID = 0x65,  // Weather
  PKTOUT_6D_ID = 0x6D,  // Play Music/Midi
  PKTOUT_6E_ID = 0x6E,  // Character Animation

  PKTOUT_70_ID = 0x70,  // Spell Effect
  PKTOUT_74_ID = 0x74,  // Send Buy Window
  PKTOUT_76_ID = 0x76,  // New Sub server
  PKTOUT_77_ID = 0x77,  // Move
  PKTOUT_78_ID = 0x78,  // Send Own Create
  PKTOUT_7C_ID = 0x7C,  // Send Old school Menu

  PKTOUT_82_ID = 0x82,  // Login Error
  PKTOUT_86_ID = 0X86,  // Resend Char After Delete
  PKTOUT_88_ID = 0x88,  // Send Open Paperdoll
  PKTOUT_89_ID = 0x89,  // Show Corpse
  PKTOUT_8C_ID = 0x8C,  // Login Packet to xfer from login server to game server

  PKTOUT_90_ID = 0x90,  // Map Message
  PKTOUT_97_ID = 0x97,  // Forced Move Player
  PKTOUT_9C_ID = 0x9C,  // Request Assistance Response
  PKTOUT_9E_ID = 0x9E,  // Sell List

  PKTOUT_A1_ID = 0xA1,  // Update Health
  PKTOUT_A2_ID = 0xA2,  // Update Mana
  PKTOUT_A3_ID = 0xA3,  // Update Stamina
  PKTOUT_A5_ID = 0xA5,  // Open URL
  PKTOUT_A6_ID = 0xA6,  // Tip Window
  PKTOUT_A8_ID = 0xA8,  // Server List
  PKTOUT_A9_ID = 0xA9,  // Characters/Starting Locations
  PKTOUT_AA_ID = 0xAA,  // Send Highlight/Opponent
  PKTOUT_AB_ID = 0xAB,  // Text entry Gump
  PKTOUT_AE_ID = 0xAE,  // Unicode Message
  PKTOUT_AF_ID = 0xAF,  // Death Action

  PKTOUT_B0_ID = 0xB0,     // Send Gump
  PKTOUT_B7_ID = 0xB7,     // Tooltip/Help Data - Old school tooltip
  PKTOUT_B9_ID = 0xB9,     // Client Features Enable
  PKTOUT_B9_V2_ID = 0xB9,  // Client Features Enable newer... Version 2 of packet
  PKTOUT_BA_ID = 0xBA,     // Quest Arrow
  PKTOUT_BC_ID = 0xBC,     // Season

  PKTOUT_C0_ID = 0xC0,  // Graphical Effect
  PKTOUT_C1_ID = 0xC1,  // Cliloc Message
  PKTOUT_C4_ID = 0xC4,  // Semi-Visible
  PKTOUT_C7_ID = 0xC7,  // 3D Graphical Effect
  PKTOUT_CC_ID = 0xCC,  // Cliloc Message Affixed

  PKTOUT_D8_ID = 0xD8,  // Custom Housing
  PKTOUT_DC_ID = 0xDC,  // Object Cahce/Revision (SE Introduced version)
  PKTOUT_DD_ID = 0xDD,  // Compressed Gump
  PKTOUT_DF_ID = 0xDF,  // Buff/Debuff System

  PKTOUT_E2_ID = 0xE2,  // HSA MobileAnimation
  PKTOUT_E3_ID = 0xE3,  // KR Encryption Response
  PKTOUT_F3_ID = 0xF3,  // SA Object Information
  PKTOUT_F5_ID = 0xF5,  // New Map Message
  PKTOUT_F6_ID = 0xF6,  // Boat Smooth Move
  PKTOUT_F7_ID = 0xF7   // Display Multi HSA
};
}

#endif
