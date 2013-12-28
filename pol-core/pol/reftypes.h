/*
History
=======


Notes
=======

*/

#ifndef REFTYPES_H
#define REFTYPES_H

#include "../clib/refptr.h"
namespace Pol {
  namespace Accounts {
	class Account;
  }
  namespace Network {
	class Client;
  }
  namespace Mobile {
	class Character;
	
  }
  namespace Items {
	class Item;
  }

  namespace Core {
#ifdef NDEBUG
	// release mode
#define REFTYPE_DEBUG 0
#else
	// debug mode
#define REFTYPE_DEBUG 1
#endif

	class UObject;
	class NPC;
	

#if !REFTYPE_DEBUG
	typedef ref_ptr<UObject> UObjectRef;
	typedef ref_ptr<Mobile::Character> CharacterRef;
	typedef ref_ptr<Items::Item> ItemRef;
	typedef ref_ptr<NPC> NPCRef; // TODO: rename NpcRef
	typedef ref_ptr<Accounts::Account> AccountRef;
	typedef ref_ptr<Network::Client> ClientRef;
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

	class NPCRef : public ref_ptr<NPC>
	{
	public:
	  explicit NPCRef( NPC* npc = 0 );
	  NPCRef( const NPCRef& rptr );
	  ~NPCRef();
	};

	typedef ref_ptr<Accounts::Account> AccountRef;
	typedef ref_ptr<Network::Client> ClientRef;

#endif
  }
}

#endif
