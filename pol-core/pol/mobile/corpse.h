#ifndef MOBILE_CORPSE_H
#define MOBILE_CORPSE_H

#include <array>

#include "clib/rawtypes.h"
#include "pol/containr.h"
#include "pol/item/item.h"
#include "pol/layers.h"

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


namespace Pol::Core
{
class ExportScript;

// Corpses must NEVER EVER be movable.
// They can decay even if they are immobile.
class UCorpse final : public UContainer
{
  using base = UContainer;

public:
  ~UCorpse() override = default;
  size_t estimatedSize() const override;
  u16 get_senditem_amount() const override;

  /// Add an item the corpse renders on one of its layers. Which layer is the item's own business
  /// -- it is in the item's location -- so all this adds over a plain insert is that the corpse now
  /// looks different.
  void add_rendered_item( Item* item, const Pos2d& pos );
  void remove( iterator itr ) override;

  void on_insert_add_item( Mobile::Character* mob, MoveType move, Items::Item* new_item ) override;
  bool take_contents_to_grave() const;
  void take_contents_to_grave( bool newvalue );
  u16 corpsetype;
  u32 ownerserial;  // NPCs get deleted on death, so serial is used.

  /// What the corpse renders, indexed by layer; null wherever nothing is on that layer. Index 0 is
  /// never filled, layers start at one.
  using LayerView = std::array<Items::Item*, HIGHEST_LAYER + 1>;

  /// Built from the contents on every call rather than kept. A corpse renders an item exactly while
  /// that item's location says OnCorpse, so there is one answer and nothing to invalidate; the
  /// callers that want the view want all of it at once, which is one pass either way.
  LayerView layer_view() const;

  bool get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                        unsigned int* PC ) const override;

protected:
  explicit UCorpse( const Items::ContainerDesc& desc );
  void spill_contents() override;
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
};
}  // namespace Pol::Core


#endif
