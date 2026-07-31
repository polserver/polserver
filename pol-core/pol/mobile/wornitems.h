#ifndef MOBILE_WORNITEMS_H
#define MOBILE_WORNITEMS_H

#include <stddef.h>

#include "pol/containr.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}  // namespace Bscript
namespace Clib
{
class StreamWriter;
}  // namespace Clib
namespace Core
{
class UObject;
}  // namespace Core
namespace Items
{
class Item;
}  // namespace Items
namespace Mobile
{
class Character;
}  // namespace Mobile
}  // namespace Pol


namespace Pol::Core
{
class WornItemsContainer final : public UContainer
{
  using base = UContainer;

public:
  WornItemsContainer();
  ~WornItemsContainer() override = default;
  size_t estimatedSize() const override;

  Bscript::BObjectImp* make_ref() override;
  Mobile::Character* get_chr_owner() const override;
  Mobile::Character* chr_owner;

  UObject* owner() override;
  const UObject* owner() const override;
  UObject* self_as_owner() override;
  const UObject* self_as_owner() const override;

  void for_each_item( void ( *f )( Items::Item* item, void* a ), void* arg ) override;

  Items::Item* GetItemOnLayer( unsigned idx ) const;

  Items::Location location_for( const Items::Item* item, const Core::Pos2d& pos ) override;

  void print( Clib::StreamWriter& sw_pc, Clib::StreamWriter& sw_equip ) const;

private:
  // The layer array is only half of being equipped: the other half is the owner's weapon, shield
  // and armor rating. Going through Character::equip/unequip is therefore the only correct way in,
  // and these stay private so it cannot be bypassed.
  void PutItemOnLayer( Item* item );
  void RemoveItemFromLayer( Item* item );

  friend class Mobile::Character;
};

inline Items::Item* WornItemsContainer::GetItemOnLayer( unsigned idx ) const
{
  if ( Items::valid_equip_layer( idx ) )
    return contents_[idx];

  return nullptr;
}

inline Mobile::Character* WornItemsContainer::get_chr_owner() const
{
  return chr_owner;
}
}  // namespace Pol::Core


#endif
