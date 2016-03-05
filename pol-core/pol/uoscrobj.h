/** @file
 *
 * @par History
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: cassert removed
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef UOSCROBJ_H
#define UOSCROBJ_H

#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../bscript/bstruct.h"
#include "../bscript/parser.h"
#include "../clib/passert.h"

#include "mobile/charactr.h"
#include "network/client.h"

#ifndef ITEM_H
#include "item/item.h"
#endif

#include "multi/boat.h"
#include "eventid.h"

#include "globals/state.h"

namespace Pol
{
namespace Bscript
{
class ExecutorModule;
}
namespace Module
{
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
    ++Core::stateManager.uobjcount.uobj_count_echrref;
    passert( obj_->ref_counted_count() > 1 );
  }
  virtual ~ECharacterRefObjImp( )
  {
    --Core::stateManager.uobjcount.uobj_count_echrref;
  }

  virtual const char* typeOf() const POL_OVERRIDE;
  virtual u8 typeOfInt() const POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE; ///id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy ) POL_OVERRIDE;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy ) POL_OVERRIDE;//id test

  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const POL_OVERRIDE;
  virtual bool operator<( const Bscript::BObjectImp& objimp ) const POL_OVERRIDE;

  virtual bool offline_access_ok() const
  {
    return false;
  }
};

class EOfflineCharacterRefObjImp : public ECharacterRefObjImp
{
public:
  explicit EOfflineCharacterRefObjImp( Mobile::Character* chr ) :
    ECharacterRefObjImp( chr )
  {}

  virtual const char* typeOf() const POL_OVERRIDE;
  virtual u8 typeOfInt() const POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;

  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool offline_access_ok() const POL_OVERRIDE
  {
    return true;
  }
};

class ECharacterEquipObjImp : public Bscript::BApplicObj< Core::CharacterRef >
{
  typedef Bscript::BApplicObj< Core::CharacterRef > base;
public:
  explicit ECharacterEquipObjImp( Mobile::Character* chr ) :
    BApplicObj< Core::CharacterRef >( &echaracterequipobjimp_type, Core::CharacterRef( chr ) )
  {}

  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) POL_OVERRIDE;
};


class EItemRefObjImp : public Bscript::BApplicObj < Core::ItemRef >
{
  typedef Bscript::BApplicObj< Core::ItemRef > base;
public:
  explicit EItemRefObjImp( Items::Item* item ) :
    BApplicObj< Core::ItemRef >( &eitemrefobjimp_type, Core::ItemRef( item ) )
  {}

  virtual const char* typeOf() const POL_OVERRIDE;
  virtual u8 typeOfInt() const POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE; //id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy ) POL_OVERRIDE;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy ) POL_OVERRIDE; //id test

  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const POL_OVERRIDE;
  virtual bool operator<( const Bscript::BObjectImp& objimp ) const POL_OVERRIDE;
};


class EUBoatRefObjImp : public Bscript::BApplicObj < ref_ptr<Multi::UBoat> >
{
  typedef BApplicObj< ref_ptr<Multi::UBoat> > base;
public:
  explicit EUBoatRefObjImp( Multi::UBoat* boat ) :
    Bscript::BApplicObj< ref_ptr<Multi::UBoat> >( &euboatrefobjimp_type, ref_ptr<Multi::UBoat>( boat ) )
  {}

  virtual const char* typeOf() const POL_OVERRIDE;
  virtual u8 typeOfInt() const POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE; //id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy ) POL_OVERRIDE;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy ) POL_OVERRIDE; //id test

  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const POL_OVERRIDE;
};


class EMultiRefObjImp : public Bscript::BApplicObj < ref_ptr<Multi::UMulti> >
{
  typedef Bscript::BApplicObj< ref_ptr<Multi::UMulti> > base;
public:
  explicit EMultiRefObjImp( Multi::UMulti* multi ) :
    BApplicObj< ref_ptr<Multi::UMulti> >( &emultirefobjimp_type, ref_ptr<Multi::UMulti>( multi ) )
  {}

  virtual const char* typeOf() const POL_OVERRIDE;
  virtual u8 typeOfInt() const POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE; //id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value, bool copy ) POL_OVERRIDE;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value, bool copy ) POL_OVERRIDE; //test id
  virtual bool isTrue() const POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const POL_OVERRIDE;
};

typedef weak_ptr<Network::Client> ClientPtrHolder;
class EClientRefObjImp : public Bscript::BApplicObj < ClientPtrHolder >
{
  typedef Bscript::BApplicObj< ClientPtrHolder > base;
public:
  explicit EClientRefObjImp( ClientPtrHolder client )
    : Bscript::BApplicObj<ClientPtrHolder>( &eclientrefobjimp_type, client )
  {}
  virtual ~EClientRefObjImp() {};

  virtual const char* typeOf() const POL_OVERRIDE;
  virtual u8 typeOfInt() const POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method( const char* methodname,
      Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
      bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE;  // id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                          bool copy ) POL_OVERRIDE;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
      bool copy ) POL_OVERRIDE;  // id test

  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const POL_OVERRIDE;
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
  ItemGivenEvent( Mobile::Character* chr_givenby, Items::Item* item_given, Mobile::NPC* chr_givento );
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
