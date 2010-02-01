	// MoveBoat: moves a boat in the direction of an absolute facing
	// 0=N, 1=NE, 2=E, etc
MoveBoat( boat, facing );


	// MoveBoatRelative: move a boat in the direction of a relative facing
	// same as MoveBoat, but adjusted for boat's facing.
	// If boat faces East, 0 will move E, 1 will move SE, etc.
MoveBoatRelative( boat, direction );

	// MoveBoatXY: moves a boat to the specified X,Y coordinate. Careful moving
	// close to the world edge where a part of the boat might be out of bounds.
MoveBoatXY( boat, x, y );	

	// RegisterItemWithBoat:'registering' an item with a boat causes the boat
	// to move the item when it moves, iff the item is still on the boat.
RegisterItemWithBoat( boat, item );


	// BoatFromItem: if you have an itemref referring to a boat (typically
	// in a doubleclick script) this will convert it into a boatref.
	// Note that this should not be necessary anymore.
BoatFromItem( item );


	// TurnBoat: Turn a boat in a direction, relative to its current directon.
	// direction is 0=none, 1=right, 2=around, 3=left
TurnBoat( boat, direction );


	// SystemFindBoatBySerial: given the serial number of a boat, return
	// a boatref that can be used with MoveBoat, TurnBoat, etc.
SystemFindBoatBySerial( serial );