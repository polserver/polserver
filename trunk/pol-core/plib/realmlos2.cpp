/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"
#include "realm.h"
#include "pol/los.h"
#include "pol/charactr.h"

bool Realm::has_los( const UObject& iatt, const UObject& itgt ) const
{
    LosObj att(iatt);
    LosObj tgt(itgt);

	if(iatt.realm != itgt.realm)
		return false;
    if (iatt.isa( UObject::CLASS_CHARACTER ))
    {
        const Character& chr = static_cast<const Character&>(iatt);
        if (chr.ignores_line_of_sight())
            return true;
    }

    return has_los( att, tgt );
}

bool Realm::has_los( const Character& iatt, const UObject& itgt ) const
{
    LosObj att(iatt);
    LosObj tgt(itgt);

	bool remote;
	Item* remote_container = iatt.search_remote_containers( itgt.serial, &remote );
	if( (remote_container != NULL) && remote)
		return true;
	if(iatt.realm != itgt.realm)
		return false;
    if (iatt.ignores_line_of_sight())
        return true;

    return has_los( att, tgt );
}

