/** @file
 *
 * @par History
 */


#include "uoskills.h"

#include "skillid.h"
#include "mobile/attribute.h"
#include "globals/uvars.h"
#include "globals/network.h"

#include "../plib/pkg.h"

#include "../clib/cfgelem.h"
#include "../clib/stlutil.h"
#include "../clib/logfacility.h"

#include <vector>
#include <stdexcept>

namespace Pol
{
namespace Core
{
const UOSkill& GetUOSkill( unsigned skillid )
{
  if ( skillid < gamestate.uo_skills.size() )
    return gamestate.uo_skills[skillid];
  else
    throw std::runtime_error( "Bad UO Skill ID" );
}

UOSkill::UOSkill( const Plib::Package* pkg, Clib::ConfigElem& elem )
    : inited( true ),
      skillid( strtoul( elem.rest(), NULL, 10 ) ),
      attributename( elem.remove_string( "Attribute", "" ) ),
      pAttr( NULL ),
      pkg( pkg )
{
  if ( skillid >= 500 )
    elem.throw_error( "SkillID must be < 500" );

  if ( !attributename.empty() )
  {
    bool required = false;
    if ( attributename[0] == '+' )
    {
      required = true;
      attributename = attributename.substr( 1, std::string::npos );
    }
    pAttr = Mobile::Attribute::FindAttribute( attributename );
    if ( !pAttr )
    {
      if ( required )
      {
        elem.throw_error( "Attribute " + attributename + " not found." );
      }
      else
      {
        elem.warn( "Attribute " + attributename + " not found." );
      }
    }
  }
}
UOSkill::UOSkill() : inited( false ), skillid( 0 ), attributename( "" ), pAttr( NULL ), pkg( NULL )
{
}

size_t UOSkill::estimateSize() const
{
  return sizeof( UOSkill ) + attributename.capacity();
}

void load_skill_entry( const Plib::Package* pkg, Clib::ConfigElem& elem )
{
  UOSkill uoskill( pkg, elem );

  if ( uoskill.skillid >= gamestate.uo_skills.size() )
    gamestate.uo_skills.resize( uoskill.skillid + 1 );

  if ( gamestate.uo_skills[uoskill.skillid].inited )
  {
    elem.throw_error( "UOSkill " + fmt::FormatInt( uoskill.skillid ).str() +
                      " already defined by " + gamestate.uo_skills[uoskill.skillid].pkg->desc() );
  }

  gamestate.uo_skills[uoskill.skillid] = uoskill;
}

void load_uoskills_cfg()
{
  gamestate.uo_skills.resize( SKILLID__COUNT );
  Plib::load_packaged_cfgs( "uoskills.cfg", "Skill", load_skill_entry );
}

void clean_skills()
{
  gamestate.uo_skills.clear();
}
}
}
