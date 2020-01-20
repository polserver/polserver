/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - invalid use of constructor as a template
 */


#ifndef POL_POLMODL_H
#define POL_POLMODL_H

#include "../bscript/execmodl.h"
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

protected:
  UOExecutor& asUOExec();
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
};


template <class T>
class TmplPolModule : public PolModule
{
  //protected:
//  TmplPolModule( Executor& exec );
};

}  // namespace Core
}  // namespace Pol

#endif
