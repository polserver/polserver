#ifndef MOBILE_WORNITEMS_H
#define MOBILE_WORNITEMS_H

#include <stddef.h>

#include "../../clib/compilerspecifics.h"
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
class WornItemsContainer : public UContainer
{
  typedef UContainer base;

public:
  WornItemsContainer();
  virtual ~WornItemsContainer(){};
  virtual size_t estimatedSize() const POL_OVERRIDE;

  virtual Bscript::BObjectImp* make_ref() POL_OVERRIDE;
  virtual Mobile::Character* get_chr_owner() POL_OVERRIDE;
  Mobile::Character* chr_owner;

  virtual UObject* owner() POL_OVERRIDE;
  virtual const UObject* owner() const POL_OVERRIDE;
  virtual UObject* self_as_owner() POL_OVERRIDE;
  virtual const UObject* self_as_owner() const POL_OVERRIDE;

  virtual void for_each_item( void ( *f )( Items::Item* item, void* a ), void* arg ) POL_OVERRIDE;

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

inline Mobile::Character* WornItemsContainer::get_chr_owner()
{
  return chr_owner;
}
}
}

#endif
