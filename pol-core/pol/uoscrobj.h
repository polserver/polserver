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

#include <stddef.h>

#include "../bscript/bobject.h"
#include "../bscript/bstruct.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../clib/weakptr.h"
#include "eventid.h"
#include "gameclck.h"
#include "globals/state.h"
#include "mobile/charactr.h"
#include "multi/boat.h"
#include "network/client.h"
#include "uobjcnt.h"

#ifndef ITEM_H
#include "item/item.h"
#endif


namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
namespace Core
{
class UContainer;
}  // namespace Core
namespace Mobile
{
class NPC;
}  // namespace Mobile
namespace Multi
{
class UMulti;
}  // namespace Multi
}  // namespace Pol

namespace Pol
{
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

class ECharacterRefObjImp : public Bscript::BApplicObj<Core::CharacterRef>
{
  typedef Bscript::BApplicObj<Core::CharacterRef> base;

public:
  explicit ECharacterRefObjImp( Mobile::Character* chr )
      : BApplicObj<Core::CharacterRef>( &echaracterrefobjimp_type, Core::CharacterRef( chr ) )
  {
    ++Core::stateManager.uobjcount.uobj_count_echrref;
    passert( obj_->ref_counted_count() > 1 );
  }
  virtual ~ECharacterRefObjImp() { --Core::stateManager.uobjcount.uobj_count_echrref; }
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  /// id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                          bool copy ) override;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                             bool copy ) override;  // id test

  virtual bool isTrue() const override;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const override;
  virtual bool operator<( const Bscript::BObjectImp& objimp ) const override;

  virtual bool offline_access_ok() const { return false; }
};

class EOfflineCharacterRefObjImp : public ECharacterRefObjImp
{
public:
  explicit EOfflineCharacterRefObjImp( Mobile::Character* chr ) : ECharacterRefObjImp( chr ) {}
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;

  virtual bool isTrue() const override;
  virtual bool offline_access_ok() const override { return true; }
};

class ECharacterEquipObjImp : public Bscript::BApplicObj<Core::CharacterRef>
{
  typedef Bscript::BApplicObj<Core::CharacterRef> base;

public:
  explicit ECharacterEquipObjImp( Mobile::Character* chr )
      : BApplicObj<Core::CharacterRef>( &echaracterequipobjimp_type, Core::CharacterRef( chr ) )
  {
  }

  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
};


class EItemRefObjImp final : public Bscript::BApplicObj<Core::ItemRef>
{
  typedef Bscript::BApplicObj<Core::ItemRef> base;

public:
  explicit EItemRefObjImp( Items::Item* item )
      : BApplicObj<Core::ItemRef>( &eitemrefobjimp_type, Core::ItemRef( item ) )
  {
  }

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                          bool copy ) override;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                             bool copy ) override;  // id test

  virtual bool isTrue() const override;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const override;
  virtual bool operator<( const Bscript::BObjectImp& objimp ) const override;
};


class EUBoatRefObjImp final : public Bscript::BApplicObj<ref_ptr<Multi::UBoat>>
{
  typedef BApplicObj<ref_ptr<Multi::UBoat>> base;

public:
  explicit EUBoatRefObjImp( Multi::UBoat* boat )
      : Bscript::BApplicObj<ref_ptr<Multi::UBoat>>( &euboatrefobjimp_type,
                                                    ref_ptr<Multi::UBoat>( boat ) )
  {
  }

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                          bool copy ) override;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                             bool copy ) override;  // id test

  virtual bool isTrue() const override;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const override;
};


class EMultiRefObjImp final : public Bscript::BApplicObj<ref_ptr<Multi::UMulti>>
{
  typedef Bscript::BApplicObj<ref_ptr<Multi::UMulti>> base;

public:
  explicit EMultiRefObjImp( Multi::UMulti* multi )
      : BApplicObj<ref_ptr<Multi::UMulti>>( &emultirefobjimp_type, ref_ptr<Multi::UMulti>( multi ) )
  {
  }

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                          bool copy ) override;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                             bool copy ) override;  // test id
  virtual bool isTrue() const override;
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const override;
};

typedef weak_ptr<Network::Client> ClientPtrHolder;
class EClientRefObjImp final : public Bscript::BApplicObj<ClientPtrHolder>
{
  typedef Bscript::BApplicObj<ClientPtrHolder> base;

public:
  explicit EClientRefObjImp( ClientPtrHolder client )
      : Bscript::BApplicObj<ClientPtrHolder>( &eclientrefobjimp_type, client )
  {
  }
  virtual ~EClientRefObjImp(){};

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                          bool copy ) override;
  virtual Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                             bool copy ) override;  // id test

  virtual bool isTrue() const override;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const override;
};

// EMenuObjImp defined on UOEMOD.CPP


class SpeechEvent final : public Bscript::BStruct
{
public:
  SpeechEvent( Mobile::Character* speaker, const char* speech );
  SpeechEvent( Mobile::Character* speaker, const char* speech, const char* texttype );  // DAVE
};

class UnicodeSpeechEvent final : public Bscript::BStruct
{
public:
  UnicodeSpeechEvent( Mobile::Character* speaker, const char* speech, const u16* wspeech,
                      const char lang[4], Bscript::ObjArray* speechtokens = nullptr );
  UnicodeSpeechEvent( Mobile::Character* speaker, const char* speech, const char* texttype,
                      const u16* wspeech, const char lang[4],
                      Bscript::ObjArray* speechtokens = nullptr );
};

class DamageEvent final : public Bscript::BStruct
{
public:
  DamageEvent( Mobile::Character* source, unsigned short damage );
};


class SourcedEvent : public Bscript::BStruct
{
public:
  SourcedEvent( Core::EVENTID type, Mobile::Character* source );
};

class UnsourcedEvent : public Bscript::BStruct
{
public:
  UnsourcedEvent( Core::EVENTID type );
};

class EngageEvent final : public SourcedEvent
{
public:
  explicit EngageEvent( Mobile::Character* engaged )
      : SourcedEvent( Core::EVID_ENGAGED, engaged ){};
};

class DisengageEvent final : public SourcedEvent
{
public:
  explicit DisengageEvent( Mobile::Character* disengaged )
      : SourcedEvent( Core::EVID_DISENGAGED, disengaged ){};
};

class ItemGivenEvent final : public SourcedEvent
{
public:
  ItemGivenEvent( Mobile::Character* chr_givenby, Items::Item* item_given,
                  Mobile::NPC* chr_givento );
  virtual ~ItemGivenEvent();

private:
  Core::ItemRef item_;
  ref_ptr<Core::UContainer> cont_;
  Core::CharacterRef given_by_;
  Core::gameclock_t given_time_;
};
}
}
#endif  // UOSCROBJ_H
