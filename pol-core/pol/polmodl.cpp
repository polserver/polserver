/** @file
 *
 * @par History
 */


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
}


bool PolModule::getCharacterOrClientParam( unsigned param, Mobile::Character*& chrptr,
                                           Network::Client*& clientptr )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getCharacterOrClientParam( param, chrptr, clientptr );
}

bool PolModule::getCharacterParam( unsigned param, Mobile::Character*& chrptr )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getCharacterParam( param, chrptr );
}

bool PolModule::getItemParam( unsigned param, Items::Item*& itemptr )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getItemParam( param, itemptr );
}

bool PolModule::getUBoatParam( unsigned param, Multi::UBoat*& boatptr )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getUBoatParam( param, boatptr );
}


bool PolModule::getMultiParam( unsigned param, Multi::UMulti*& multiptr )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getMultiParam( param, multiptr );
}

bool PolModule::getUObjectParam( unsigned param, UObject*& objptr )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getUObjectParam( param, objptr );
}

bool PolModule::getObjtypeParam( unsigned param, unsigned int& objtype )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getObjtypeParam( param, objtype );
}

bool PolModule::getObjtypeParam( unsigned param, const Items::ItemDesc*& itemdesc_out )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getObjtypeParam( param, itemdesc_out );
}

bool PolModule::getSkillIdParam( unsigned param, USKILLID& skillid )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getSkillIdParam( param, skillid );
}


bool PolModule::getAttributeParam( unsigned param, const Mobile::Attribute*& attr )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getAttributeParam( param, attr );
}


bool PolModule::getVitalParam( unsigned param, const Vital*& vital )
{
  passert( exec.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( exec );
  return uoex.getVitalParam( param, vital );
}

}  // namespace Core
}  // namespace Pol
