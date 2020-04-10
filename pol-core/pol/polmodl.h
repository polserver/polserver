#ifndef POL_POLMODL_H
#define POL_POLMODL_H

#include "../bscript/execmodl.h"
#include "base/position.h"
#include "skillid.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}
namespace Mobile
{
class Attribute;
class Character;
}  // namespace Mobile
namespace Items
{
class Item;
class ItemDesc;
}  // namespace Items
namespace Multi
{
class UBoat;
class UMulti;
}  // namespace Multi
namespace Realms
{
class Realm;
}
namespace Network
{
class Client;
}

namespace Core
{
class UObject;
class Vital;
class UOExecutor;
class PolModule : public Bscript::ExecutorModule
{
public:
  PolModule( const char* moduleName, Bscript::Executor& iExec );

public:
  UOExecutor& uoexec();

protected:
  bool getCharacterOrClientParam( unsigned param, Mobile::Character*& chrptr,
                                  Network::Client*& clientptr );
  bool getCharacterParam( unsigned param, Mobile::Character*& chrptr );
  bool getItemParam( unsigned param, Items::Item*& itemptr );
  bool getUBoatParam( unsigned param, Multi::UBoat*& boatptr );
  bool getMultiParam( unsigned param, Multi::UMulti*& multiptr );
  bool getUObjectParam( unsigned param, UObject*& objptr );
  bool getObjtypeParam( unsigned param, unsigned int& objtype );
  bool getObjtypeParam( unsigned param, const Items::ItemDesc*& itemdesc_out );
  bool getSkillIdParam( unsigned param, USKILLID& skillid );
  bool getAttributeParam( unsigned param, const Mobile::Attribute*& attr );
  bool getVitalParam( unsigned param, const Vital*& vital );

  bool getRealmParam( unsigned param, Realms::Realm** realm );
  bool getPos2dParam( unsigned xparam, unsigned yparam, Pos2d* pos,
                      const Realms::Realm* realm = nullptr );
  bool getPos3dParam( unsigned xparam, unsigned yparam, unsigned zparam, Pos3d* pos,
                      const Realms::Realm* realm = nullptr );
  bool getPos4dParam( unsigned xparam, unsigned yparam, unsigned zparam, unsigned realmparam,
                      Pos4d* pos );
};
}  // namespace Core
}  // namespace Pol
#endif
