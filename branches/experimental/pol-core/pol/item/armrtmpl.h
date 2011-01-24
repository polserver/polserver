/*
History
=======


Notes
=======

*/

#ifndef ARMRTMPL_H
#define ARMRTMPL_H

#include <string>
#include <set>

#include "itemdesc.h"
#include "../equipdsc.h"

class ConfigElem;

class ArmorDesc : public EquipDesc
{
public:
    //ArmorTemplate();
	typedef EquipDesc base;
    ArmorDesc( u16 objtype, ConfigElem& elem, const Package* pkg );
	virtual void PopulateStruct( BStruct* descriptor ) const;

    unsigned short ar;
    std::set<unsigned short> zones;
    ScriptDef on_hit_script;
};

#endif
