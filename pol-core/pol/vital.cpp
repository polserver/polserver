/** @file
 *
 * @par History
 * - 2005/09/14 Shinigami: regen_while_dead implemented
 */


#include "vital.h"

#include "checkpnt.h"
#include "syshook.h"
#include "globals/uvars.h"

#include "../plib/pkg.h"

#include "../clib/cfgelem.h"
#include "../clib/maputil.h"
#include "../clib/passert.h"

#include <vector>
#include <string>
#include <iostream>
#include <map>

namespace Pol
{
namespace Core
{

Vital::Vital( const Plib::Package* pkg, Clib::ConfigElem& elem ) :
  pkg( pkg ),
  name( elem.rest() ),
  aliases(),
  vitalid( 0 ),
  next( NULL ),
  get_regenrate_func( FindExportedFunction( elem, pkg, elem.remove_string( "RegenRateFunction" ), 1 ) ),
  get_maximum_func( FindExportedFunction( elem, pkg, elem.remove_string( "MaximumFunction" ), 1 ) ),
  underflow_func( NULL ),
  regen_while_dead( elem.remove_bool( "RegenWhileDead", false ) )
{
  aliases.push_back( name );
  std::string tmp;
  while ( elem.remove_prop( "Alias", &tmp ) )
    aliases.push_back( tmp );

  if ( elem.remove_prop( "UnderflowFunction", &tmp ) )
  {
    underflow_func = FindExportedFunction( elem, pkg, tmp, 2 );
  }
}

Vital::~Vital()
{
  if ( this->underflow_func != NULL )
    delete this->underflow_func;

  if ( this->get_maximum_func != NULL )
    delete this->get_maximum_func;

  if ( this->get_regenrate_func != NULL )
    delete this->get_regenrate_func;

  this->get_maximum_func = NULL;
  this->underflow_func = NULL;
  this->get_regenrate_func = NULL;
}

size_t Vital::estimateSize() const
{
  size_t size = sizeof(Vital)
                + name.capacity()
                + 3*sizeof(ExportedFunction);
  for (const auto& alias : aliases)
    size += alias.capacity();
  return size;
}

void clean_vitals()
{
  std::vector<Vital*>::iterator iter = gamestate.vitals.begin();
  for ( ; iter != gamestate.vitals.end(); ++iter )
  {
    delete *iter;
    *iter = NULL;
  }
  gamestate.vitals.clear();
  gamestate.vitals_byname.clear();
}

Vital* FindVital( const std::string& str )
{
  VitalsByName::const_iterator citr = gamestate.vitals_byname.find( str );
  if ( citr != gamestate.vitals_byname.end() )
    return ( *citr ).second;
  else
    return NULL;
}

Vital* FindVital( unsigned vitalid )
{
  if ( vitalid < gamestate.vitals.size() )
    return gamestate.vitals[vitalid];
  else
    return NULL;
}

void load_vital_entry( const Plib::Package* pkg, Clib::ConfigElem& elem )
{
  Vital* vital = new Vital( pkg, elem );

  const Vital* existing = FindVital( vital->name );
  if ( existing )
  {
    elem.throw_error( "Vital " + vital->name + " already defined by "
                      + existing->pkg->desc() );
  }
  vital->vitalid = static_cast<unsigned int>( gamestate.vitals.size() );
  if ( !gamestate.vitals.empty() )
    gamestate.vitals.back()->next = vital;
  gamestate.vitals.push_back( vital );

  for ( unsigned i = 0; i < vital->aliases.size(); ++i )
  {
    gamestate.vitals_byname[vital->aliases[i]] = vital;
  }
}

void load_vitals_cfg()
{
  checkpoint( "load_vitals_cfg: load_packed_cfgs" );
  load_packaged_cfgs( "vitals.cfg", "Vital", load_vital_entry );

  gamestate.numVitals = static_cast<unsigned int>( gamestate.vitals.size() );

  checkpoint( "load_vitals_cfg: find Life vital" );
  gamestate.pVitalLife = FindVital( "Life" );
  checkpoint( "load_vitals_cfg: find Stamina vital" );
  gamestate.pVitalStamina = FindVital( "Stamina" );
  checkpoint( "load_vitals_cfg: find Mana vital" );
  gamestate.pVitalMana = FindVital( "Mana" );

  checkpoint( "load_vitals_cfg: verify vital vitals" );
  passert_always( gamestate.pVitalLife );
  passert_always( gamestate.pVitalStamina );
  passert_always( gamestate.pVitalMana );
}
}
}