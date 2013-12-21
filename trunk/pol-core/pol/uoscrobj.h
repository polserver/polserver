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
#include "eventid.h"

namespace Pol {
  namespace Bscript {
	class ExecutorModule;
  }
  namespace Module {
	extern Bscript::BApplicObjType eitemrefobjimp_type;
	extern Bscript::BApplicObjType echaracterrefobjimp_type;
	extern Bscript::BApplicObjType echaracterequipobjimp_type;
	extern Bscript::BApplicObjType euboatrefobjimp_type;
	extern Bscript::BApplicObjType emultirefobjimp_type;
	extern Bscript::BApplicObjType storage_area_type;
	extern Bscript::BApplicObjType menu_type;
	extern Bscript::BApplicObjType eclientrefobjimp_type;

	class ECharacterRefObjImp : public Bscript::BApplicObj< Core::CharacterRef >
	{
	  typedef Bscript::BApplicObj< Core::CharacterRef > base;
	public:
	  explicit ECharacterRefObjImp( Mobile::Character* chr ) :
		BApplicObj< Core::CharacterRef >( &echaracterrefobjimp_type, Core::CharacterRef( chr ) )
	  {
		  ++Core::uobj_count_echrref;
		  passert( obj_->ref_counted_count() > 1 );
		}
	  virtual ~ECharacterRefObjImp( ) { --Core::uobj_count_echrref; }

	  virtual const char* typeOf() const;
	  virtual int typeOfInt() const;
	  virtual Bscript::BObjectImp* copy( ) const;
	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
	  virtual Bscript::BObjectRef get_member( const char* membername );
	  virtual Bscript::BObjectRef get_member_id( const int id ); ///id test
	  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy );
	  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy );//id test

	  virtual bool isTrue() const;
	  virtual bool isEqual( const Bscript::BObjectImp& objimp ) const;
	  virtual bool isLessThan( const Bscript::BObjectImp& objimp ) const;

	  virtual bool offline_access_ok() const { return false; }
	};

	class EOfflineCharacterRefObjImp : public ECharacterRefObjImp
	{
	public:
	  explicit EOfflineCharacterRefObjImp( Mobile::Character* chr ) :
		ECharacterRefObjImp( chr )
	  {}

	  virtual const char* typeOf() const;
	  virtual int typeOfInt() const;
	  virtual Bscript::BObjectImp* copy( ) const;

	  virtual bool isTrue() const;
	  virtual bool offline_access_ok() const { return true; }
	};

	class ECharacterEquipObjImp : public Bscript::BApplicObj< Core::CharacterRef >
	{
	  typedef Bscript::BApplicObj< Core::CharacterRef > base;
	public:
	  explicit ECharacterEquipObjImp( Mobile::Character* chr ) :
		BApplicObj< Core::CharacterRef >( &echaracterequipobjimp_type, Core::CharacterRef( chr ) )
	  {}

	  virtual Bscript::BObjectImp* copy( ) const;
	  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj );
	};


	class EItemRefObjImp : public Bscript::BApplicObj < Core::ItemRef >
	{
	  typedef Bscript::BApplicObj< Core::ItemRef > base;
	public:
	  explicit EItemRefObjImp( Items::Item* item ) :
		BApplicObj< Core::ItemRef >( &eitemrefobjimp_type, Core::ItemRef( item ) )
	  {}

	  virtual const char* typeOf() const;
	  virtual int typeOfInt() const;
	  virtual Bscript::BObjectImp* copy( ) const;
	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
	  virtual Bscript::BObjectRef get_member( const char* membername );
	  virtual Bscript::BObjectRef get_member_id( const int id ); //id test
	  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy );
	  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy ); //id test

	  virtual bool isTrue() const;
	  virtual bool isEqual( const Bscript::BObjectImp& objimp ) const;
	  virtual bool isLessThan( const Bscript::BObjectImp& objimp ) const;
	};


	class EUBoatRefObjImp : public Bscript::BApplicObj < ref_ptr<Multi::UBoat> >
	{
	  typedef BApplicObj< ref_ptr<Multi::UBoat> > base;
	public:
	  explicit EUBoatRefObjImp( Multi::UBoat* boat ) :
		Bscript::BApplicObj< ref_ptr<Multi::UBoat> >( &euboatrefobjimp_type, ref_ptr<Multi::UBoat>( boat ) )
	  {}

	  virtual const char* typeOf() const;
	  virtual int typeOfInt() const;
	  virtual Bscript::BObjectImp* copy( ) const;
	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
	  virtual Bscript::BObjectRef get_member( const char* membername );
	  virtual Bscript::BObjectRef get_member_id( const int id ); //id test
	  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy );
	  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy ); //id test

	  virtual bool isTrue() const;
	  virtual bool isEqual( const Bscript::BObjectImp& objimp ) const;
	};


	class EMultiRefObjImp : public Bscript::BApplicObj < ref_ptr<Multi::UMulti> >
	{
	  typedef Bscript::BApplicObj< ref_ptr<Multi::UMulti> > base;
	public:
	  explicit EMultiRefObjImp( Multi::UMulti* multi ) :
		BApplicObj< ref_ptr<Multi::UMulti> >( &emultirefobjimp_type, ref_ptr<Multi::UMulti>( multi ) )
	  {}

	  virtual const char* typeOf() const;
	  virtual int typeOfInt() const;
	  virtual Bscript::BObjectImp* copy( ) const;
	  virtual Bscript::BObjectRef get_member( const char* membername );
	  virtual Bscript::BObjectRef get_member_id( const int id ); //id test
	  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy );
	  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy ); //test id
	  virtual bool isTrue() const;
	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
	  virtual bool isEqual( const Bscript::BObjectImp& objimp ) const;
	};

	class ClientPtrHolder
	{
	public:
	  explicit ClientPtrHolder( Core::ClientRef i_client ) : client( i_client ) {}
	  Network::Client* operator->( ) { return client.get(); }
	  const Network::Client* operator->( ) const { return client.get(); }
	  /*bool operator!=(const ClientPtrHolder& a) const	{ return ConstPtr() != a.ConstPtr(); }
	  bool operator==(const ClientPtrHolder& a) const	{ return ConstPtr() == a.ConstPtr(); }
	  bool operator!=(const ClientPtrHolder a) const	{ return ConstPtr() != a.ConstPtr(); }
	  bool operator==(const ClientPtrHolder a) const	{ return ConstPtr() == a.ConstPtr(); }*/

	  Network::Client* Ptr( ) { return client.get( ); }
	  const Network::Client* ConstPtr( ) const { return client.get( ); }
	private:
	  Core::ClientRef client;
	};

	class EClientRefObjImp : public Bscript::BApplicObj < ClientPtrHolder >
	{
	  typedef Bscript::BApplicObj< ClientPtrHolder > base;
	public:
	  explicit EClientRefObjImp( const ClientPtrHolder& client ) :
		Bscript::BApplicObj< ClientPtrHolder >( &eclientrefobjimp_type, client )
	  {}
	  virtual ~EClientRefObjImp() {};


	  virtual const char* typeOf() const;
	  virtual int typeOfInt() const;
	  virtual Bscript::BObjectImp* copy( ) const;
	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
	  virtual Bscript::BObjectRef get_member( const char* membername );
	  virtual Bscript::BObjectRef get_member_id( const int id ); //id test
	  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy );
	  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy ); //id test

	  virtual bool isTrue() const;
	  virtual bool isEqual( const Bscript::BObjectImp& objimp ) const;
	};

	// EMenuObjImp defined on UOEMOD.CPP


	class SpeechEvent : public Bscript::BStruct
	{
	public:
	  SpeechEvent( Mobile::Character* speaker, const char* speech );
	  SpeechEvent( Mobile::Character* speaker, const char* speech, const char* texttype ); //DAVE
	};

	class UnicodeSpeechEvent : public Bscript::BStruct
	{
	public:
	  UnicodeSpeechEvent( Mobile::Character* speaker, const char* speech,
						  const u16* wspeech, const char lang[4], Bscript::ObjArray* speechtokens = NULL );
	  UnicodeSpeechEvent( Mobile::Character* speaker, const char* speech, const char* texttype,
						  const u16* wspeech, const char lang[4], Bscript::ObjArray* speechtokens = NULL );
	};

	class DamageEvent : public Bscript::BStruct
	{
	public:
	  DamageEvent( Mobile::Character* source, unsigned short damage );
	};



	class SourcedEvent : public Bscript::BStruct
	{
	public:
	  SourcedEvent( Core::EVENTID type, Mobile::Character* source );
	};

	class EngageEvent : public SourcedEvent
	{
	public:
	  explicit EngageEvent( Mobile::Character* engaged ) :
		SourcedEvent( Core::EVID_ENGAGED, engaged )
	  {};
	};

	class DisengageEvent : public SourcedEvent
	{
	public:
	  explicit DisengageEvent( Mobile::Character* disengaged ) :
		SourcedEvent( Core::EVID_DISENGAGED, disengaged )
	  {};
	};

	class ItemGivenEvent : public SourcedEvent
	{
	public:
	  ItemGivenEvent( Mobile::Character* chr_givenby, Items::Item* item_given, Core::NPC* chr_givento );
	  virtual ~ItemGivenEvent();

	private:
	  Core::ItemRef item_;
	  ref_ptr< Core::UContainer > cont_;
	  Core::CharacterRef given_by_;
	  Core::gameclock_t given_time_;
	};
  }
}
#endif // UOSCROBJ_H
