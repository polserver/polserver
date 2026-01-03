#ifndef MOBILE_CORPSE_H
#define MOBILE_CORPSE_H

#include "../../clib/rawtypes.h"
#include "../containr.h"
#include "../item/item.h"
#include "../reftypes.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Items
{
class ContainerDesc;
class ItemDesc;
}  // namespace Items
namespace Mobile
{
class Character;
}  // namespace Mobile
namespace Multi
{
class UMulti;
}  // namespace Multi
}  // namespace Pol

namespace Pol
{
namespace Core
{
class ExportScript;

// Corpses must NEVER EVER be movable.
// They can decay even if they are immobile.
class UCorpse final : public UContainer
{
  typedef UContainer base;

public:
  ~UCorpse() override = default;
  size_t estimatedSize() const override;
  u16 get_senditem_amount() const override;

  void add( Item* item, const Pos2d& pos ) override;
  void equip_and_add( Item* item, unsigned idx );
  void remove( iterator itr ) override;

  void on_insert_add_item( Mobile::Character* mob, MoveType move, Items::Item* new_item ) override;
  bool take_contents_to_grave() const;
  void take_contents_to_grave( bool newvalue );
  u16 corpsetype;
  u32 ownerserial;  // NPCs get deleted on death, so serial is used.
  const Core::ItemRef& GetItemOnLayer( unsigned idx ) const;

  bool get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                        unsigned int* PC ) const override;

protected:
  void RemoveItemFromLayer( Items::Item* item );
  void PutItemOnLayer( Items::Item* item );

  explicit UCorpse( const Items::ContainerDesc& desc );
  void spill_contents( Multi::UMulti* supporting_multi ) override;
  void printProperties( Clib::StreamWriter& sw ) const override;
  void readProperties( Clib::ConfigElem& elem ) override;
  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );
  // Bscript::BObjectImp* script_member( const char *membername );
  Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  // Bscript::BObjectImp* set_script_member( const char *membername, const std::string&
  // value );
  // Bscript::BObjectImp* set_script_member( const char *membername, int value );
  bool script_isa( unsigned isatype ) const override;
  std::vector<Core::ItemRef> can_equip_list_;
};
}  // namespace Core
}  // namespace Pol

#endif
