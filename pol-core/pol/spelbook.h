/** @file
 *
 * @par History
 * - 2009/09/18 MuadDib:   Spellbook rewrite to deal with only bits, not scrolls inside them.
 * - 2009/10/09 Turley:    Added spellbook.spells() & .hasspell() methods
 * - 2009/10/10 Turley:    Added spellbook.addspell() & .removespell() methods
 */


#ifndef __SPELBOOK_H
#define __SPELBOOK_H

#include "../clib/rawtypes.h"
#include "item/item.h"
#ifndef CONTAINR_H
#include "containr.h"
#endif

namespace Pol
{
namespace Network
{
class Client;
}
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
class ItemDesc;
}  // namespace Items

namespace Items
{
class SpellbookDesc;
}
namespace Core
{
class ExportScript;
class UOExecutor;

class Spellbook final : public UContainer
{
  typedef UContainer base;

public:
  explicit Spellbook( const Items::SpellbookDesc& descriptor );
  ~Spellbook() override;
  size_t estimatedSize() const override;

  bool has_spellid( unsigned int spellid ) const;
  bool remove_spellid( unsigned int spellid );
  bool add_spellid( unsigned int spellid );
  u8 bitwise_contents[8];
  u8 spell_school;
  void add( Items::Item* item, const Pos2d& pos ) override;
  void printProperties( Clib::StreamWriter& sw ) const override;
  void readProperties( Clib::ConfigElem& elem ) override;
  void printOn( Clib::StreamWriter& sw ) const override;
  void printSelfOn( Clib::StreamWriter& sw ) const override;
  void double_click( Network::Client* client ) override;
  void send_book_old( Network::Client* client );
  bool script_isa( unsigned isatype ) const override;
  Bscript::BObjectImp* script_method( const char* methodname, UOExecutor& ex ) override;
  Bscript::BObjectImp* script_method_id( const int id, UOExecutor& ex ) override;
  bool get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                        unsigned int* PC ) const override;

private:
  bool can_add( const Items::Item& item ) const override;
  void add_bulk( int item_count_delta, int weight_delta ) override;
  void calc_current_bitwise_contents();

protected:
  friend class Items::Item;
};

class USpellScroll final : public Items::Item
{
  typedef Items::Item base;

public:
  ~USpellScroll() override = default;
  size_t estimatedSize() const override;
  u16 get_senditem_amount() const override;
  static u16 convert_objtype_to_spellnum( u32 objtype, u8 school );

protected:
  explicit USpellScroll( const Items::ItemDesc& descriptor );
  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );

private:
};

void send_spellbook_contents( Network::Client* client, Spellbook& spellbook );
}  // namespace Core
}  // namespace Pol
#endif
