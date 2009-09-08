/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#include "uobject.h"
#include "mobile/charactr.h"
#include "item/item.h"
#include "npc.h"
#include "reftypes.h"

int hits = 0;
int uobjectrefs = 0;
int characterrefs;
int npcrefs;
int itemrefs;

typedef map<long, CharacterRef*> CharacterRefs;
CharacterRefs crefs;
unsigned long crefs_count;

void display_reftypes()
{
    for( CharacterRefs::iterator itr = crefs.begin(); itr != crefs.end(); ++itr )
    {
        cout << "cref " << (*itr).first << "=" << (*itr).second << endl;
    }
}
#if REFTYPE_DEBUG
UObjectRef::UObjectRef( UObject* ptr ) : ref_ptr<UObject>(ptr)
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
    }
    ++uobjectrefs;
}
UObjectRef::UObjectRef( const UObjectRef& rptr ) : ref_ptr<UObject>(rptr)
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
    }
    ++uobjectrefs;
}

UObjectRef::~UObjectRef()
{
    --uobjectrefs;
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
    }
}

CharacterRef::CharacterRef( Character* ptr ) : ref_ptr<Character>(ptr), cref_instance(0)
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
        cref_instance = crefs_count++;
        if (cref_instance == 50)
        {
            cref_instance = cref_instance;
        }
        crefs[ cref_instance ] = this;
    }
    ++characterrefs;
}
CharacterRef::CharacterRef( const CharacterRef& rptr ) : ref_ptr<Character>(rptr), cref_instance(0)
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
        cref_instance = crefs_count++;
        if (cref_instance == 50)
        {
            cref_instance = cref_instance;
        }
        crefs[ cref_instance ] = this;
    }
    ++characterrefs;
}
CharacterRef::~CharacterRef()
{
    --characterrefs;
    if (get() && get()->serial_ext == 0x619d1300)
    {
        if (cref_instance == 50)
        {
            cref_instance = cref_instance;
        }
        crefs.erase( cref_instance );
        --hits;
    }
}

const CharacterRef& CharacterRef::operator=( const CharacterRef& rptr )
{
    set( rptr.get() );

    return *this;
}
void CharacterRef::set( Character* chr )
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        crefs.erase( cref_instance );
        --hits;
    }

    ref_ptr<Character>::set( chr );

    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
        cref_instance = crefs_count++;
        crefs[ cref_instance ] = this;
    }
}
void CharacterRef::clear()
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        crefs.erase( cref_instance );
        --hits;
    }

    ref_ptr<Character>::clear();
}


NPCRef::NPCRef( NPC* ptr ) : ref_ptr<NPC>(ptr)
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
    }
    ++npcrefs;
}
NPCRef::NPCRef( const NPCRef& rptr ) : ref_ptr<NPC>(rptr)
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
    }
    ++npcrefs;
}

NPCRef::~NPCRef()
{
    --npcrefs;
    if (get() && get()->serial_ext == 0x619d1300)
    {
        --hits;
    }
}


ItemRef::ItemRef( Item* ptr ) : ref_ptr<Item>(ptr)
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
    }
    ++itemrefs;
}

ItemRef::ItemRef( const ItemRef& rptr ) : ref_ptr<Item>(rptr)
{
    if (get() && get()->serial_ext == 0x619d1300)
    {
        ++hits;
    }
    ++itemrefs;
}

ItemRef::~ItemRef()
{
    --itemrefs;
    if (get() && get()->serial_ext == 0x619d1300)
    {
        --hits;
    }
}
#endif
