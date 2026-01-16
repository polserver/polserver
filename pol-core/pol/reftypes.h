/** @file
 *
 * @par History
 */


#ifndef REFTYPES_H
#define REFTYPES_H
#include "../clib/refptr.h"

namespace Pol
{
namespace Accounts
{
class Account;
}
namespace Mobile
{
class Character;
class NPC;
}  // namespace Mobile
namespace Items
{
class Item;
}

namespace Core
{
#ifdef NDEBUG
// release mode
#define REFTYPE_DEBUG 0
#else
// debug mode
#define REFTYPE_DEBUG 0
#endif

class UObject;


#if !REFTYPE_DEBUG
using UObjectRef = ref_ptr<UObject>;
using CharacterRef = ref_ptr<Mobile::Character>;
using ItemRef = ref_ptr<Items::Item>;
using NpcRef = ref_ptr<Mobile::NPC>;
using AccountRef = ref_ptr<Accounts::Account>;
#else
class UObjectRef : public ref_ptr<UObject>
{
public:
  explicit UObjectRef( UObject* ptr = 0 );
  UObjectRef( const UObjectRef& rptr );
  ~UObjectRef();
};

class CharacterRef : public ref_ptr<Mobile::Character>
{
public:
  explicit CharacterRef( Mobile::Character* chr );
  CharacterRef( const CharacterRef& rptr );
  ~CharacterRef();

  CharacterRef& operator=( const CharacterRef& rptr );
  void set( Mobile::Character* chr );
  void clear();

  unsigned int cref_instance;
};

class ItemRef : public ref_ptr<Items::Item>
{
public:
  explicit ItemRef( Items::Item* item = 0 );
  ItemRef( const ItemRef& rptr );
  ~ItemRef();
};

class NpcRef : public ref_ptr<Mobile::NPC>
{
public:
  explicit NpcRef( Mobile::NPC* npc = 0 );
  NpcRef( const NpcRef& rptr );
  ~NpcRef();
};

typedef ref_ptr<Accounts::Account> AccountRef;

#endif
}  // namespace Core
}  // namespace Pol

#endif
