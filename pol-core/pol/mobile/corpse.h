#ifndef MOBILE_CORPSE_H
#define MOBILE_CORPSE_H

#include "../../clib/rawtypes.h"
#include "../containr.h"
#include "../item/item.h"

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
  virtual ~UCorpse() = default;
  virtual size_t estimatedSize() const override;
  virtual u16 get_senditem_amount() const override;

  virtual void add( Item* item ) override;
  virtual void remove( iterator itr ) override;

  virtual void on_insert_add_item( Mobile::Character* mob, MoveType move,
                                   Items::Item* new_item ) override;
  bool take_contents_to_grave() const;
  void take_contents_to_grave( bool newvalue );
  u16 corpsetype;
  u32 ownerserial;  // NPCs get deleted on death, so serial is used.
  Items::Item* GetItemOnLayer( unsigned idx ) const;

  virtual bool get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                                unsigned int* PC ) const override;

protected:
  void RemoveItemFromLayer( Items::Item* item );
  void PutItemOnLayer( Items::Item* item );

  explicit UCorpse( const Items::ContainerDesc& desc );
  virtual void spill_contents( Multi::UMulti* supporting_multi ) override;
  virtual void printProperties( Clib::StreamWriter& sw ) const override;
  virtual void readProperties( Clib::ConfigElem& elem ) override;
  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );
  // virtual Bscript::BObjectImp* script_member( const char *membername );
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  // virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string&
  // value );
  // virtual Bscript::BObjectImp* set_script_member( const char *membername, int value );
  virtual bool script_isa( unsigned isatype ) const override;
  Contents layer_list_;
};

inline Items::Item* UCorpse::GetItemOnLayer( unsigned idx ) const
{
  // Checks if the requested layer is valid
  if ( Items::valid_equip_layer( idx ) )
    return layer_list_[idx];

  return nullptr;
}
}  // namespace Core
}  // namespace Pol

#endif
