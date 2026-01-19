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
#include <string>

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
#include "polobject.h"
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


namespace Pol::Module
{
extern Bscript::BApplicObjType eitemrefobjimp_type;
extern Bscript::BApplicObjType echaracterrefobjimp_type;
extern Bscript::BApplicObjType echaracterequipobjimp_type;
extern Bscript::BApplicObjType euboatrefobjimp_type;
extern Bscript::BApplicObjType emultirefobjimp_type;
extern Bscript::BApplicObjType storage_area_type;
extern Bscript::BApplicObjType menu_type;
extern Bscript::BApplicObjType eclientrefobjimp_type;

class ECharacterRefObjImp : public Core::PolApplicObj<Core::CharacterRef>
{
  using base = PolApplicObj<Core::CharacterRef>;

public:
  explicit ECharacterRefObjImp( Mobile::Character* chr )
      : PolApplicObj<Core::CharacterRef>( &echaracterrefobjimp_type, Core::CharacterRef( chr ) )
  {
    ++Core::stateManager.uobjcount.uobj_count_echrref;
    passert( obj_->ref_counted_count() > 1 );
  }
  ~ECharacterRefObjImp() override { --Core::stateManager.uobjcount.uobj_count_echrref; }
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  /// id test
  Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                  bool copy ) override;
  Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                     bool copy ) override;  // id test

  bool isTrue() const override;
  bool operator==( const Bscript::BObjectImp& objimp ) const override;
  bool operator<( const Bscript::BObjectImp& objimp ) const override;

  virtual bool offline_access_ok() const { return false; }
};

class EOfflineCharacterRefObjImp : public ECharacterRefObjImp
{
public:
  explicit EOfflineCharacterRefObjImp( Mobile::Character* chr ) : ECharacterRefObjImp( chr ) {}
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;

  bool isTrue() const override;
  bool offline_access_ok() const override { return true; }
};

class ECharacterEquipObjImp : public Core::PolApplicObj<Core::CharacterRef>
{
  using base = Core::PolApplicObj<Core::CharacterRef>;

public:
  explicit ECharacterEquipObjImp( Mobile::Character* chr )
      : PolApplicObj<Core::CharacterRef>( &echaracterequipobjimp_type, Core::CharacterRef( chr ) )
  {
  }

  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
};


class EItemRefObjImp final : public Core::PolApplicObj<Core::ItemRef>
{
  using base = Core::PolApplicObj<Core::ItemRef>;

public:
  explicit EItemRefObjImp( Items::Item* item )
      : PolApplicObj<Core::ItemRef>( &eitemrefobjimp_type, Core::ItemRef( item ) )
  {
  }

  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                  bool copy ) override;
  Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                     bool copy ) override;  // id test

  bool isTrue() const override;
  bool operator==( const Bscript::BObjectImp& objimp ) const override;
  bool operator<( const Bscript::BObjectImp& objimp ) const override;
};


class EUBoatRefObjImp final : public Core::PolApplicObj<ref_ptr<Multi::UBoat>>
{
  using base = Core::PolApplicObj<ref_ptr<Multi::UBoat>>;

public:
  explicit EUBoatRefObjImp( Multi::UBoat* boat )
      : PolApplicObj<ref_ptr<Multi::UBoat>>( &euboatrefobjimp_type, ref_ptr<Multi::UBoat>( boat ) )
  {
  }

  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                  bool copy ) override;
  Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                     bool copy ) override;  // id test

  bool isTrue() const override;
  bool operator==( const Bscript::BObjectImp& objimp ) const override;
};


class EMultiRefObjImp final : public Core::PolApplicObj<ref_ptr<Multi::UMulti>>
{
  using base = Core::PolApplicObj<ref_ptr<Multi::UMulti>>;

public:
  explicit EMultiRefObjImp( Multi::UMulti* multi )
      : PolApplicObj<ref_ptr<Multi::UMulti>>( &emultirefobjimp_type,
                                              ref_ptr<Multi::UMulti>( multi ) )
  {
  }

  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                  bool copy ) override;
  Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                     bool copy ) override;  // test id
  bool isTrue() const override;
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
  bool operator==( const Bscript::BObjectImp& objimp ) const override;
};

using ClientPtrHolder = weak_ptr<Network::Client>;
class EClientRefObjImp final : public Core::PolApplicObj<ClientPtrHolder>
{
  using base = Core::PolApplicObj<ClientPtrHolder>;

public:
  explicit EClientRefObjImp( ClientPtrHolder client )
      : PolApplicObj<ClientPtrHolder>( &eclientrefobjimp_type, std::move( client ) )
  {
  }
  ~EClientRefObjImp() override = default;

  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectRef set_member( const char* membername, Bscript::BObjectImp* value,
                                  bool copy ) override;
  Bscript::BObjectRef set_member_id( const int id, Bscript::BObjectImp* value,
                                     bool copy ) override;  // id test

  bool isTrue() const override;
  bool operator==( const Bscript::BObjectImp& objimp ) const override;
};

// EMenuObjImp defined on UOEMOD.CPP


class SpeechEvent final : public Bscript::BStruct
{
public:
  SpeechEvent( Mobile::Character* speaker, const std::string& speech, const std::string& texttype,
               std::string lang = "", Bscript::ObjArray* speechtokens = nullptr );
};

class DamageEvent final : public Bscript::BStruct
{
public:
  DamageEvent( Mobile::Character* source, unsigned short damage );
};

class BoatMovementEvent : public Bscript::BStruct
{
public:
  BoatMovementEvent( Mobile::Character* source, const u8 speed, const u8 direction,
                     const u8 relative_direction );
};

class SourcedEvent : public Bscript::BStruct
{
public:
  SourcedEvent( Core::EVENTID type, Mobile::Character* source );
};

class EngageEvent final : public SourcedEvent
{
public:
  explicit EngageEvent( Mobile::Character* engaged )
      : SourcedEvent( Core::EVID_ENGAGED, engaged ) {};
};

class DisengageEvent final : public SourcedEvent
{
public:
  explicit DisengageEvent( Mobile::Character* disengaged )
      : SourcedEvent( Core::EVID_DISENGAGED, disengaged ) {};
};

class ItemGivenEvent final : public SourcedEvent
{
public:
  ItemGivenEvent( Mobile::Character* chr_givenby, Items::Item* item_given,
                  Mobile::NPC* chr_givento );
  ~ItemGivenEvent() override;

private:
  Core::ItemRef item_;
  ref_ptr<Core::UContainer> cont_;
  Core::CharacterRef given_by_;
  Core::gameclock_t given_time_;
};
}  // namespace Pol::Module

#endif  // UOSCROBJ_H
