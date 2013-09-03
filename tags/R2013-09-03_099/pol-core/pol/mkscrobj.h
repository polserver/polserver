/*
History
=======


Notes
=======

*/

#ifndef MKSCROBJ_H
#define MKSCROBJ_H

class BObjectImp;
class Character;
class Item;
class UBoat;

BObjectImp* make_mobileref( Character* chr );
BObjectImp* make_itemref( Item* item );
BObjectImp* make_boatref( UBoat* boat );

#endif
