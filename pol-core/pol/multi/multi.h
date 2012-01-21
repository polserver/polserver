/*
History
=======
2005/06/06 Shinigami: added readobjects template - to get a list of statics
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/


#ifndef MULTI_H
#define MULTI_H

#include "multidef.h"

// also consider: multimap<unsigned int, unsigned int>
// unsigned int is (x << 16) | y
// unsigned int is z << 16 | objtype
// hell, even multimap<unsigned short,unsigned int>
// unsigned short is (x<<8)|y
// (relative signed x/y, -128 to +127
// unsigned int is z << 16 | objtype

#include "../item/item.h"

class MultiDef;
class BObjectImp;
class BStruct;
class Realm;

const int CRMULTI_IGNORE_MULTIS    = 0x0001;
const int CRMULTI_IGNORE_OBJECTS   = 0x0002;
const int CRMULTI_IGNORE_FLATNESS  = 0x0004;
const int CRMULTI_FACING_NORTH     = 0x0000;
const int CRMULTI_FACING_EAST      = 0x0100;
const int CRMULTI_FACING_SOUTH     = 0x0200;
const int CRMULTI_FACING_WEST      = 0x0300;
const int CRMULTI_FACING_MASK      = 0x0300;
const int CRMULTI_FACING_SHIFT     = 8;

class UMulti : public Item
{
	typedef Item base;
public:

	u16 multiid;

    static UMulti* create( const ItemDesc& descriptor, u32 serial = 0 );
    
    static BObjectImp* scripted_create( const ItemDesc& descriptor, u16 x, u16 y, s8 z, Realm* realm, int flags );

    virtual void double_click( Client* client );
    virtual void register_object( UObject* obj );
	virtual void unregister_object( UObject* obj );
    virtual bool script_isa( unsigned isatype ) const;

    const MultiDef& multidef() const;
    virtual class UBoat* as_boat();
    virtual class UHouse* as_house();
    virtual BObjectImp* make_ref();
    virtual class BObjectImp* get_script_member( const char *membername ) const;
    virtual class BObjectImp* get_script_member_id( const int id ) const; 

	BStruct* footprint() const;
    virtual bool readshapes( MapShapeList& vec, s16 rx, s16 ry, s16 zbase ){return false;};
    virtual bool readobjects( StaticList& vec, s16 rx, s16 ry, s16 zbase ){return false;};
protected:
    explicit UMulti(const ItemDesc& itemdesc );
    virtual ~UMulti();

    virtual const char* classname() const;
    friend class ref_ptr<UMulti>;

private:

    //virtual void destroy(void);
};


void send_multi( Client* client, const UMulti* multi );
void send_multi_to_inrange( const UMulti* multi );

#endif
