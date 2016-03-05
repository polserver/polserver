/** @file
 *
 * @par History
 */


#ifndef UOEXHELP_H
#define UOEXHELP_H
#include "skillid.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}
namespace Mobile
{
class Attribute;
class Character;
}
namespace Items
{
class Item;
class ItemDesc;
}
namespace Multi
{
class UBoat;
class UMulti;
}
namespace Network
{
class Client;
}
namespace Core
{
class PropertyList;
class UObject;
class Vital;


bool getCharacterParam( Bscript::Executor& exec, unsigned param, Mobile::Character*& chr );
bool getItemParam( Bscript::Executor& exec, unsigned param, Items::Item*& item );
bool getUObjectParam( Bscript::Executor& exec, unsigned param, UObject*& objptr );
bool getObjtypeParam( Bscript::Executor& exec, unsigned param, unsigned int& objtype );
bool getObjtypeParam( Bscript::Executor& exec, unsigned param, const Items::ItemDesc*& itemdesc );
bool getSkillIdParam( Bscript::Executor& exec, unsigned param, USKILLID& skillid );
bool getUBoatParam( Bscript::Executor& exec, unsigned param, Multi::UBoat*& boat );
bool getMultiParam( Bscript::Executor& exec, unsigned param, Multi::UMulti*& multi );
bool getAttributeParam( Bscript::Executor& exec, unsigned param, const Mobile::Attribute*& attr );
bool getVitalParam( Bscript::Executor& exec, unsigned param, const Vital*& vital );
bool getCharacterOrClientParam( Bscript::Executor& exec, unsigned param, Mobile::Character*& chrptr, Network::Client*& clientptr );
}
}
#endif
