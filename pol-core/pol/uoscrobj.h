/*
History
=======
2009/08/25 Shinigami: STLport-5.2.1 fix: <cassert> removed
2009/09/03 MuadDib:	  Changes for account related source file relocation
                      Changes for multi related source file relocation
2009/12/21 Turley:    ._method() call fix

Notes
=======

*/

#ifndef UOSCROBJ_H
#define UOSCROBJ_H

#include "../bscript/bobject.h"
#include "../bscript/bstruct.h"
#include "../bscript/parser.h"
#include "../clib/passert.h"

#include "mobile/charactr.h"
#include "item/item.h"
#include "multi/boat.h"
#include "uobjcnt.h"

extern BApplicObjType eitemrefobjimp_type;
extern BApplicObjType echaracterrefobjimp_type;
extern BApplicObjType echaracterequipobjimp_type;
extern BApplicObjType euboatrefobjimp_type;
extern BApplicObjType emultirefobjimp_type;
extern BApplicObjType storage_area_type;
extern BApplicObjType menu_type;
extern BApplicObjType eclientrefobjimp_type;

class ExecutorModule;

class ECharacterRefObjImp : public BApplicObj< CharacterRef >
{
    typedef BApplicObj< CharacterRef > base;
public:
    explicit ECharacterRefObjImp( Character* chr ) : 
        BApplicObj< CharacterRef >( &echaracterrefobjimp_type, CharacterRef(chr) ) 
        { 
            ++uobj_count_echrref; 
            passert( obj_->ref_counted_count() > 1 );
        }
    virtual ~ECharacterRefObjImp() { --uobj_count_echrref; }

    virtual const char* typeOf() const;
	virtual int typeOfInt() const;
    virtual BObjectImp* copy() const;
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); ///id test
    virtual BObjectRef set_member( const char* membername, BObjectImp* value );
    virtual BObjectRef set_member_id( const int id, BObjectImp* value );//id test

    virtual bool isTrue() const;
    virtual bool isEqual(const BObjectImp& objimp) const;
    virtual bool isLessThan(const BObjectImp& objimp) const;

    virtual bool offline_access_ok() const { return false; }
};

class EOfflineCharacterRefObjImp : public ECharacterRefObjImp
{
public:
    explicit EOfflineCharacterRefObjImp( Character* chr ) :
        ECharacterRefObjImp( chr ) {}

    virtual const char* typeOf() const;
	virtual int typeOfInt() const;
    virtual BObjectImp* copy() const;

    virtual bool isTrue() const;
    virtual bool offline_access_ok() const { return true; }
};

class ECharacterEquipObjImp : public BApplicObj< CharacterRef >
{
    typedef BApplicObj< CharacterRef > base;
public:
    explicit ECharacterEquipObjImp( Character* chr ) : 
        BApplicObj< CharacterRef >( &echaracterequipobjimp_type, CharacterRef(chr) ) 
        {}

    virtual BObjectImp* copy() const;
    virtual BObjectRef OperSubscript( const BObject& obj );
};


class EItemRefObjImp : public BApplicObj < ItemRef >
{
    typedef BApplicObj< ItemRef > base;
public:
    explicit EItemRefObjImp( Item* item ) :
        BApplicObj< ItemRef >( &eitemrefobjimp_type, ItemRef(item) )
        {}

    virtual const char* typeOf() const;
	virtual int typeOfInt() const;
    virtual BObjectImp* copy() const;
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); //id test
    virtual BObjectRef set_member( const char* membername, BObjectImp* value );
    virtual BObjectRef set_member_id( const int id, BObjectImp* value ); //id test

    virtual bool isTrue() const;
    virtual bool isEqual(const BObjectImp& objimp) const;
    virtual bool isLessThan(const BObjectImp& objimp) const;
};


class EUBoatRefObjImp : public BApplicObj < ref_ptr<UBoat> >
{
    typedef BApplicObj< ref_ptr<UBoat> > base;
public:
    explicit EUBoatRefObjImp( UBoat* boat ) :
        BApplicObj< ref_ptr<UBoat> >( &euboatrefobjimp_type, ref_ptr<UBoat>(boat) )
        {}

    virtual const char* typeOf() const;
	virtual int typeOfInt() const;
    virtual BObjectImp* copy() const;
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); //id test
    virtual BObjectRef set_member( const char* membername, BObjectImp* value );
    virtual BObjectRef set_member_id( const int id, BObjectImp* value ); //id test

    virtual bool isTrue() const;
	virtual bool isEqual(const BObjectImp& objimp) const;
};


class EMultiRefObjImp : public BApplicObj < ref_ptr<UMulti> >
{
    typedef BApplicObj< ref_ptr<UMulti> > base;
public:
    explicit EMultiRefObjImp( UMulti* multi ) :
        BApplicObj< ref_ptr<UMulti> >( &emultirefobjimp_type, ref_ptr<UMulti>(multi) )
        {}

    virtual const char* typeOf() const;
	virtual int typeOfInt() const;
    virtual BObjectImp* copy() const;
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); //id test
    virtual BObjectRef set_member( const char* membername, BObjectImp* value );
    virtual BObjectRef set_member_id( const int id, BObjectImp* value ); //test id
    virtual bool isTrue() const;
	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
	virtual bool isEqual(const BObjectImp& objimp) const;
};

class ClientPtrHolder
{
public:
	explicit ClientPtrHolder( ClientRef i_client ) : client(i_client) {}
	Client* operator->() { return client.get(); }
	const Client* operator->() const { return client.get(); }
	/*bool operator!=(const ClientPtrHolder& a) const	{ return ConstPtr() != a.ConstPtr(); }
	bool operator==(const ClientPtrHolder& a) const	{ return ConstPtr() == a.ConstPtr(); }
	bool operator!=(const ClientPtrHolder a) const	{ return ConstPtr() != a.ConstPtr(); }
	bool operator==(const ClientPtrHolder a) const	{ return ConstPtr() == a.ConstPtr(); }*/

	Client* Ptr() { return client.get(); }
	const Client* ConstPtr() const { return client.get(); }
private:
	ClientRef client;
};

class EClientRefObjImp : public BApplicObj < ClientPtrHolder >
{
	typedef BApplicObj< ClientPtrHolder > base;
public:
	explicit EClientRefObjImp( const ClientPtrHolder& client ) :
	BApplicObj< ClientPtrHolder >( &eclientrefobjimp_type, client )
	{}
	virtual ~EClientRefObjImp() {};


	virtual const char* typeOf() const;
	virtual int typeOfInt() const;
	virtual BObjectImp* copy() const;
	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
	virtual BObjectRef get_member( const char* membername );
	virtual BObjectRef get_member_id( const int id ); //id test
	virtual BObjectRef set_member( const char* membername, BObjectImp* value );
	virtual BObjectRef set_member_id( const int id, BObjectImp* value ); //id test

	virtual bool isTrue() const;
	virtual bool isEqual(const BObjectImp& objimp) const;
};

// EMenuObjImp defined on UOEMOD.CPP


class SpeechEvent : public BStruct
{
public:
    SpeechEvent( Character* speaker, const char* speech);
	SpeechEvent( Character* speaker, const char* speech, const char* texttype); //DAVE
};

class UnicodeSpeechEvent : public BStruct
{
public:
    UnicodeSpeechEvent( Character* speaker, const char* speech,
											const u16* wspeech, const char lang[4], ObjArray* speechtokens=NULL);
	UnicodeSpeechEvent( Character* speaker, const char* speech, const char* texttype,
											const u16* wspeech, const char lang[4], ObjArray* speechtokens=NULL);
};

class DamageEvent : public BStruct
{
public:
    DamageEvent( Character* source, unsigned short damage );
};

#include "eventid.h"

class SourcedEvent : public BStruct
{
public:
    SourcedEvent( EVENTID type, Character* source );
};

class EngageEvent : public SourcedEvent
{
public:
    explicit EngageEvent( Character* engaged ) :
      SourcedEvent( EVID_ENGAGED, engaged ) {};
};

class DisengageEvent : public SourcedEvent
{
public:
    explicit DisengageEvent( Character* disengaged ) : 
      SourcedEvent( EVID_DISENGAGED, disengaged ) {};
};

class ItemGivenEvent : public SourcedEvent
{
public:
    ItemGivenEvent( Character* chr_givenby, Item* item_given, NPC* chr_givento );
    virtual ~ItemGivenEvent();

private:
    ItemRef item_;
    ref_ptr< UContainer > cont_;
    CharacterRef given_by_;
    gameclock_t given_time_;
};

#endif // UOSCROBJ_H
