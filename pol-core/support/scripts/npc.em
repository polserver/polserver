////////////////////////////////////////////////////////////////
//
//	CONSTANTS
//
////////////////////////////////////////////////////////////////

// Constants for texttype flags in the Say() and SayUC() functions
const SAY_TEXTTYPE_DEFAULT := "default";
const SAY_TEXTTYPE_WHISPER := "whisper";
const SAY_TEXTTYPE_YELL := "yell";

// Constant for Unicode Language flag in the SayUC() function
const SAY_LANG := "ENU";

// Constants for doevent flag in Say() and SayUC() functions
const SAY_DOEVENT_DISABLE := 0x0;
const SAY_DOEVENT_ENABLE := 0x1;

const FACE_NORMAL := 0x0;
const FACE_FORCE := 0x1;

////////////////////////////////////////////////////////////////
//
//	FUNCTIONS
//
////////////////////////////////////////////////////////////////

// SetAnchor: set a wander-anchor
// centerx, centery: the anchor-point
// distance_start: distance from center where boundary kicks in
// percent_subtract: percent chance per tile past the boundary that moving away will fail
SetAnchor( centerx, centery, distance_start, percent_subtract );

// Self(): return a mobileref to self
Self();

// Wander: walk, usually forward, sometimes turning
Wander();

// Face: takes an integer (0 - 7) or direction string: N, S, E, W, NW, NE, SW, SE
Face( direction, flags := FACE_NORMAL );

// N, S, E, W, NW, NE, SW, SE
// a boundingbox is okay, too
Move( direction ); 
				

// Move toward, or away from, something.
// When a pathfinding algorith exists, these functions will use if necessary.
// Puts the script to sleep for a period of time proportional to the NPC's agility.
// Note also, walking in range of your opponent may cause you to attack.
WalkToward( object );
WalkAwayFrom( object );
RunToward( object );
RunAwayFrom( object );
TurnToward( object, flags := FACE_NORMAL );
TurnAwayFrom( object, flags := FACE_NORMAL );

WalkTowardLocation( x, y );
WalkAwayFromLocation( x, y );
RunTowardLocation( x, y );
RunAwayFromLocation( x, y );
TurnTowardLocation( x, y, flags := FACE_NORMAL );
TurnAwayFromLocation( x, y, flags := FACE_NORMAL );

	
// Set your opponent.  If you are in range (or are carrying
// a projectile weapon and have LOS), you will attack.  If
// a player character is connected, the NPC will highlight.
// Implicitly sets war mode.
SetOpponent( character );

	
// SetWarMode: Usually used to leave warmode, but can be used
// to enter warmode independently of setting an opponent.
// Setting war mode to 0 clears your opponent.
SetWarMode( warmode );

// Instructs an npc to speak
// Texttype is the type of speech the npc will make (Whisper, Normal, Yell)
// If doevent is enabled, it will send a system event to other npcs.
Say( text, text_type:=SAY_TEXTTYPE_DEFAULT, do_event:=SAY_DOEVENT_DISABLE );

// Instructs an npc to speak in Unicode
// Texttype is the type of speech the npc will make (Whisper, Normal, Yell)
// If doevent is enabled, it will send a system event to other npcs.
// 'uc_text' is an Array of 2-byte integers,
//           where each integer is a Unicode character!
// 'langcode' is a 3-character "Originating Language" code
//       e.g. ENG, ENU, CHT, DEU, FRA, JPN, RUS, KOR (etc??)
SayUC(uc_text, text_type:=SAY_TEXTTYPE_DEFAULT, langcode:=SAY_LANG, do_event:=SAY_DOEVENT_DISABLE );


// * deprecated ( use Self().x, Self().y, Self().z ) - Do not use
Position();

// Get a custom property.  Equivalent to GetObjProperty( Self(), propertyname )
GetProperty( propertyname );

// Set a custom property.  
// Equivalent to SetObjProperty( Self(), propertyname, propertyvalue )
SetProperty( propertyname, propertyvalue );

// Make a bounding box from an area string.  
// An area string is a number of 'x1 y1 x2 y2' entries.
// "1 1 10 10 10 3 20 6" would create a walk area something like this:
//   XXX 
//   XXXXXX
//   XXX
MakeBoundingBox( areastring );

// IsLegalMove: Given your position, and a tentative move, would it
// fall within a bounding box?
IsLegalMove( move, boundingbox );

// CanMove: Given current position and direction, would it be possible to move there?
CanMove( direction );
