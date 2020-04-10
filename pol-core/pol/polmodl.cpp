#include "polmodl.h"

#include <ctype.h>
#include <exception>
#include <stdlib.h>
#include <string>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../bscript/fmodule.h"
#include "../bscript/impstr.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../plib/systemstate.h"
#include "fnsearch.h"
#include "globals/network.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "multi/multi.h"
#include "network/client.h"
#include "uobject.h"
#include "uoexec.h"
#include "uoscrobj.h"
#include "vital.h"

namespace Pol
{
namespace Core
{
using namespace Bscript;
using namespace Module;

PolModule::PolModule( const char* moduleName, Bscript::Executor& iExec )
    : Bscript::ExecutorModule( moduleName, iExec )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
}

UOExecutor& PolModule::uoexec()
{
  return static_cast<UOExecutor&>( exec );
}

bool PolModule::getCharacterOrClientParam( unsigned param, Mobile::Character*& chrptr,
                                           Network::Client*& clientptr )
{
  return uoexec().getCharacterOrClientParam( param, chrptr, clientptr );
}

bool PolModule::getCharacterParam( unsigned param, Mobile::Character*& chrptr )
{
  return uoexec().getCharacterParam( param, chrptr );
}

bool PolModule::getItemParam( unsigned param, Items::Item*& itemptr )
{
  return uoexec().getItemParam( param, itemptr );
}

bool PolModule::getUBoatParam( unsigned param, Multi::UBoat*& boatptr )
{
  return uoexec().getUBoatParam( param, boatptr );
}

bool PolModule::getMultiParam( unsigned param, Multi::UMulti*& multiptr )
{
  return uoexec().getMultiParam( param, multiptr );
}

bool PolModule::getUObjectParam( unsigned param, UObject*& objptr )
{
  return uoexec().getUObjectParam( param, objptr );
}

bool PolModule::getObjtypeParam( unsigned param, unsigned int& objtype )
{
  return uoexec().getObjtypeParam( param, objtype );
}

bool PolModule::getObjtypeParam( unsigned param, const Items::ItemDesc*& itemdesc_out )
{
  return uoexec().getObjtypeParam( param, itemdesc_out );
}

bool PolModule::getSkillIdParam( unsigned param, USKILLID& skillid )
{
  return uoexec().getSkillIdParam( param, skillid );
}

bool PolModule::getAttributeParam( unsigned param, const Mobile::Attribute*& attr )
{
  return uoexec().getAttributeParam( param, attr );
}

bool PolModule::getVitalParam( unsigned param, const Vital*& vital )
{
  return uoexec().getVitalParam( param, vital );
}

bool PolModule::getPos2dParam( unsigned xparam, unsigned yparam, Pos2d* pos )
{
  u16 x;
  u16 y;
  if ( getParam( xparam, x ) && getParam( yparam, y ) )
  {
    *pos = Pos2d( x, y );
    return true;
  }
  return false;
}
bool PolModule::getPos3dParam( unsigned xparam, unsigned yparam, unsigned zparam, Pos3d* pos )
{
  u16 x;
  u16 y;
  s8 z;
  if ( getParam( xparam, x ) && getParam( yparam, y ) && getParam( zparam, z ) )
  {
    *pos = Pos3d( x, y, z );
    return true;
  }
  return false;
}
}  // namespace Core
}  // namespace Pol
