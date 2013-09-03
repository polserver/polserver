/*
History
=======


Notes
=======

*/

#ifndef EQUIPDSC_H
#define EQUIPDSC_H

#include "item/itemdesc.h"

class ConfigElem;
class Package;

class EquipDesc : public ItemDesc
{
public:
	typedef ItemDesc base;
    EquipDesc( u32 objtype, ConfigElem& elem, Type type, const Package* pkg );
    EquipDesc(); // for dummy template
	virtual void PopulateStruct( BStruct* descriptor ) const;
};

#endif
