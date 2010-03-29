/*
History
=======


Notes
=======

*/

#ifndef REFTYPES_H
#define REFTYPES_H

#include "../clib/refptr.h"

#ifdef NDEBUG
 // release mode
#define REFTYPE_DEBUG 0
#else
 // debug mode
#define REFTYPE_DEBUG 1
#endif

class Account;
class Character;
class Item;
class NPC;
class UObject;
class Client;

#if !REFTYPE_DEBUG
typedef ref_ptr<UObject> UObjectRef;
typedef ref_ptr<Character> CharacterRef;
typedef ref_ptr<Item> ItemRef;
typedef ref_ptr<NPC> NPCRef; // TODO: rename NpcRef
typedef ref_ptr<Account> AccountRef;
typedef ref_ptr<Client> ClientRef;
#else
class UObjectRef : public ref_ptr<UObject>
{
public:
    explicit UObjectRef( UObject* ptr = 0 );
    UObjectRef( const UObjectRef& rptr );
    ~UObjectRef();
};

class CharacterRef : public ref_ptr<Character>
{
public:
    explicit CharacterRef( Character* chr );
    CharacterRef( const CharacterRef& rptr );
    ~CharacterRef();

    const CharacterRef& operator=( const CharacterRef& rptr );
    void set( Character* chr );
    void clear();

    unsigned int cref_instance;
};

class ItemRef : public ref_ptr<Item>
{
public:
    explicit ItemRef( Item* item = 0 );
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

typedef ref_ptr<Account> AccountRef;
typedef ref_ptr<Client> ClientRef;

#endif


#endif
