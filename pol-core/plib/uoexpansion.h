#ifndef PLIB_UOEXPANSION_H
#define PLIB_UOEXPANSION_H

#include "..\clib\rawtypes.h"

namespace Pol {
	namespace Plib {

enum class B9Feature : u32 {
	T2A          = 0x01, // Chats, regions (1.25.35)
	Renaissance  = 0x02, // Trammel/felucca (2.0.0)
	ThirdDawn    = 0x04, // Ilshenar, 3D client (3.0.0)
	LBR          = 0x08, // LBR skills, map (3.0.7b)
	AOS          = 0x10, // AOS skills, map, spells, fightbook (3.0.8z)
	Has6thSlot   = 0x20, // 6th character slot (4.0.3a)
	SE           = 0x40, // SE Features + housing tiles (4.0.5a)
	ML           = 0x80, // ML features: elven race, spells, skills + housing tiles
	
	Splash8thAge = 0x100, // Splash screen 8th age (5.0.1a)
	
	CrystalShadowTiles = 0x200, // Splash screen 9th age + crystal/shadow house tiles (5.0.5a)
	Splash9thAge       = 0x200, //    ^

	Splash10thAge      = 0x400, // Splash screen 10th age (6.0.3.0 ??)

	IncreasedStorage   = 0x800, // Increased house/bank storage (6.0.3.0 or earlier)
	
	Has7thSlot   = 0x1000, // 7th character slot (6.0.3.0 or earlier)
	KRFaces      = 0x2000, // ?? (6.0.0.0 was KR release)
	
	TrialAcct    = 0x4000,
	LiveAcct     = 0x8000,

	SA   = 0x10000, // Gargoyle race, spells, skills (7.0.0.0)
	HSA  = 0x20000, // HSA features (7.0.9.0)
	GothicTiles = 0x40000, // Gothic housing tiles (7.0.9.0)
	RusticTiles = 0x80000, // Rustic housing tiles (7.0.9.0)
};

enum class A9Feature : u32 {
	Unk1 = 0x01, // Unknown, never sent by OSI
	ConfigReqLogout = 0x02, // Send config/req logout (IGR? overwrite configuration button?)
	SingleCharacter = 0x04, // Siege style 1 char/acct
	ContextMenus = 0x08, // Enable NPC popup/context menus
	LimitSlots = 0x10, // Limit character slots
	AOS        = 0x20, // AOS features (necromancer/paladin classes & tooltips) (3.0.8z)
	Has6thSlot = 0x40, // 6th character slot (4.0.3a)
	
	SamuraiNinja = 0x80, // Samurai and Ninja classes (4.0.5a)
	ElvenRace = 0x100, // (4.0.11d)
	
	Unk2 = 0x200, // Added with UOKR launch (6.0.0.0)
	UO3DClientType = 0x400, // Send UO3D client type (client will send 0xE1 packet) (6.0.0.0)
	
	Unk3 = 0x800, // Added between UOKR and UOSA launch
	
	Has7thSlot = 0x1000, // 7th character slot, only 2D client (6.0.3.0 or earlier)
	
	Unk4 = 0x2000, // Added with UOSA launch (7.0.0.0)

	NewMovement = 0x4000, // New movement packets 0xF0 -> 0xF2 (7.0.0.0)
	UnlockNewFeluccaAreas = 0x8000, // Unlock new felucca areas (factions map0x.mul and such) (7.0.0.0 - SA or HSA, not sure)
};

class UOExpansion
{
protected:
	u32 A9Flag;
	u32 B9Flag;

	int charSlots;
public:
	UOExpansion(void) : A9Flag(0), B9Flag(0), charSlots(5) {};

	UOExpansion(u32 A9Flag, u32 B9Flag, int charSlots) : A9Flag(A9Flag), B9Flag(B9Flag), charSlots(charSlots) {};

	UOExpansion(u32 A9Flag, u32 B9Flag) : A9Flag(A9Flag), B9Flag(B9Flag), charSlots(5) {
		if (hasFeature(B9Feature::Has6thSlot))
			charSlots = 6;

		if (hasFeature(B9Feature::Has7thSlot))
			charSlots = 7;

		if (hasFeature(A9Feature::SingleCharacter))
			charSlots = 1;
	};



	const u32 A9Flags() const { return A9Flag; };
	const u32 B9Flags() const { return B9Flag; };

	const bool hasFeature(A9Feature x) const { return (bool)(A9Flag & (u32)x); };
	const bool hasFeature(B9Feature x) const { return (bool)(B9Flag & (u32)x); };
		
	bool hasCustomHousing() { return false; };
	bool hasElvenRace() { return hasFeature(A9Feature::ElvenRace); };
	bool hasContextMenus() { return hasFeature(A9Feature::ContextMenus); };
	bool usesNewMovementPacket() { return hasFeature(A9Feature::NewMovement); };

	int characterSlots() { return charSlots; };
};

	}
}
#endif