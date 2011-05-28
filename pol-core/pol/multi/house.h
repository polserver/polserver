/*
History
=======
2005/06/06 Shinigami: added readobjects - to get a list of statics
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#ifndef _HOUSE_H
#define _HOUSE_H

#include "../../clib/stl_inc.h"
#include "customhouses.h"
#include "multi.h"

class ObjArray;
class BStruct;
class BObjectImp;
class Realm;

typedef list<Item*>         ItemList;
typedef list<Character*>    MobileList;


class UHouse : public UMulti
{
    typedef UMulti base;

public:
    static BObjectImp* scripted_create( const ItemDesc& descriptor, u16 x, u16 y, s8 z, Realm* realm, int flags );
    void destroy_components();

    CustomHouseDesign CurrentDesign;
    CustomHouseDesign WorkingDesign;
    CustomHouseDesign BackupDesign;
    vector<u8> CurrentCompressed;
    vector<u8> WorkingCompressed;

    bool IsCustom() const {return custom;};
    void SetCustom(bool custom);
    void CustomHouseSetInitialState();
    static UHouse* FindWorkingHouse(u32 chrserial);
	bool IsEditing() const { return editing; }
	bool IsWaitingForAccept() const { return waiting_for_accept; }
    bool editing;
	bool waiting_for_accept;
    int editing_floor_num;
    u32 revision;

	virtual void register_object( UObject* obj );
	virtual void unregister_object( UObject* obj );

	virtual void walk_on( Character* chr );

	void ClearSquatters();
	void add_component(Item* item, s32 xoff, s32 yoff, u8 zoff);
	static void list_contents( const UHouse* house,
                    ItemList& items_in,
                    MobileList& chrs_in );
	void AcceptHouseCommit(Character* chr, bool accept);
	

protected:
    explicit UHouse( const ItemDesc& itemdesc );
    void create_components();

    virtual void readProperties( ConfigElem& elem );
    virtual void printProperties( std::ostream& os ) const;
    virtual BObjectImp* script_method( const char* membername, Executor& ex );
    virtual BObjectImp* script_method_id( const int id, Executor& ex );
    virtual class BObjectImp* get_script_member( const char *membername ) const;
    virtual class BObjectImp* get_script_member_id( const int id ) const; ///id test
    virtual bool script_isa( unsigned isatype ) const;
    virtual class UHouse* as_house();
    virtual bool readshapes( MapShapeList& vec, short x, short y, short zbase );
    virtual bool readobjects( StaticList& vec, short x, short y, short zbase );
    ObjArray* component_list() const;
    ObjArray* items_list() const;
    ObjArray* mobiles_list() const;

    friend class UMulti;
	friend class CustomHouseDesign;

	typedef ItemRef Component;
	typedef std::vector< Component > Components;
	Components* get_components() { return &components_; }
    bool custom;
private:
	typedef UObjectRef Squatter;
	typedef std::vector< Squatter > Squatters;
	Squatters squatters_;

    Components components_;
};


BObjectImp* destroy_house( UHouse* house );

#endif
