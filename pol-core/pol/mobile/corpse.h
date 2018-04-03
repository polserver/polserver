#ifndef MOBILE_CORPSE_H
#define MOBILE_CORPSE_H

#include "../containr.h"

namespace Pol
{
namespace Core
{
// Corpses must NEVER EVER be movable.
// They can decay even if they are immobile.
class UCorpse : public UContainer
{
  typedef UContainer base;

public:
  virtual ~UCorpse(){};
  virtual size_t estimatedSize() const POL_OVERRIDE;
  virtual u16 get_senditem_amount() const POL_OVERRIDE;

  virtual void add( Item* item ) POL_OVERRIDE;
  virtual void remove( iterator itr ) POL_OVERRIDE;

  virtual void on_insert_add_item( Mobile::Character* mob, MoveType move,
                                   Items::Item* new_item ) POL_OVERRIDE;

  u16 corpsetype;
  bool take_contents_to_grave;
  u32 ownerserial;  // NPCs get deleted on death, so serial is used.
  Items::Item* GetItemOnLayer( unsigned idx ) const;

protected:
  void RemoveItemFromLayer( Items::Item* item );
  void PutItemOnLayer( Items::Item* item );

  explicit UCorpse( const Items::ContainerDesc& desc );
  virtual void spill_contents( Multi::UMulti* supporting_multi ) POL_OVERRIDE;
  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );
  // virtual Bscript::BObjectImp* script_member( const char *membername );
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE;  /// id test
  // virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string&
  // value );
  // virtual Bscript::BObjectImp* set_script_member( const char *membername, int value );
  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
  Contents layer_list_;
};

inline Items::Item* UCorpse::GetItemOnLayer( unsigned idx ) const
{
  // Checks if the requested layer is valid
  if ( Items::valid_equip_layer( idx ) )
    return ITEM_ELEM_PTR( layer_list_[idx] );

  return EMPTY_ELEM;
}
}
}

#endif