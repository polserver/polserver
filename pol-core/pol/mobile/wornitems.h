#ifndef MOBILE_WORNITEMS_H
#define MOBILE_WORNITEMS_H

#include <stddef.h>

#include "../containr.h"

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

namespace Pol
{
namespace Core
{
class WornItemsContainer final : public UContainer
{
  typedef UContainer base;

public:
  WornItemsContainer();
  virtual ~WornItemsContainer() = default;
  virtual size_t estimatedSize() const override;

  virtual Bscript::BObjectImp* make_ref() override;
  virtual Mobile::Character* get_chr_owner() const override;
  Mobile::Character* chr_owner;

  virtual UObject* owner() override;
  virtual const UObject* owner() const override;
  virtual UObject* self_as_owner() override;
  virtual const UObject* self_as_owner() const override;

  virtual void for_each_item( void ( *f )( Items::Item* item, void* a ), void* arg ) override;

  Items::Item* GetItemOnLayer( unsigned idx ) const;
  void PutItemOnLayer( Item* item );
  void RemoveItemFromLayer( Item* item );

  void print( Clib::StreamWriter& sw_pc, Clib::StreamWriter& sw_equip ) const;
};

inline Items::Item* WornItemsContainer::GetItemOnLayer( unsigned idx ) const
{
  if ( Items::valid_equip_layer( idx ) )
    return ITEM_ELEM_PTR( contents_[idx] );

  return nullptr;
}

inline Mobile::Character* WornItemsContainer::get_chr_owner() const
{
  return chr_owner;
}
}
}

#endif
