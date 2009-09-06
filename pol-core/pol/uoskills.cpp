/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#include <vector>

#include "clib/cfgelem.h"
#include "clib/stlutil.h"

#include "mobile/attribute.h"
#include "plib/pkg.h"
#include "uoskills.h"
#include "skillid.h"

std::vector<UOSkill> uo_skills( SKILLID__COUNT );

const UOSkill& GetUOSkill( unsigned skillid )
{
    if (skillid < uo_skills.size())
        return uo_skills[skillid];
    else
        throw runtime_error( "Bad UO Skill ID" );
}

UOSkill::UOSkill( const Package* pkg, ConfigElem& elem ) :
    inited(true),
        skillid( strtoul( elem.rest(), NULL, 10 ) ),
    attributename(elem.remove_string( "Attribute", "" )),
    pAttr(NULL),
    pkg(pkg)
{
    if (skillid < 0 || skillid >= 500)
        elem.throw_error( "SkillID must be < 500" );

    if (!attributename.empty())
    {
        bool required = false;
        if (attributename[0] == '+')
        {
            required = true;
            attributename = attributename.substr( 1, std::string::npos );
        }
        pAttr = FindAttribute( attributename );
        if (!pAttr)
        {
            if (required)
            {
                elem.throw_error( "Attribute " 
                                    + tostring(attributename)
                                    + " not found." );
            }
            else
            {
                elem.warn( "Attribute " 
                                    + tostring(attributename)
                                    + " not found." );
            }
        }
    }
}
UOSkill::UOSkill() :
    inited(false),
    skillid(0),
    attributename(""),
    pAttr(NULL),
    pkg(NULL)
{
}

void load_skill_entry( const Package* pkg, ConfigElem& elem )
{
    UOSkill uoskill( pkg, elem );

    if ( uoskill.skillid >= uo_skills.size() )
        uo_skills.resize( uoskill.skillid+1 );
    
    if ( uo_skills[uoskill.skillid].inited )
    {
        elem.throw_error( "UOSkill "
                            + tostring(uoskill.skillid) 
                            + " already defined by " 
                            + uo_skills[uoskill.skillid].pkg->desc() );
    }
    
    uo_skills[ uoskill.skillid ] = uoskill;
}

void load_uoskills_cfg()
{
    load_packaged_cfgs( "uoskills.cfg", "Skill", load_skill_entry );
}

void clean_skills()
{
	uo_skills.clear();
}
