/** @file
 *
 * @par History
 * - 2009/09/18 MuadDib:   Spellbook rewrite to deal with only bits, not scrolls inside them.
 * - 2009/10/09 Turley:    Added spellbook.spells() & .hasspell() methods
 * - 2009/10/10 Turley:    Added spellbook.addspell() & .removespell() methods
 */


#ifndef __SPELBOOK_H
#define __SPELBOOK_H

#ifndef CONTAINR_H
#	include "containr.h"
#endif

namespace Pol {
  namespace Network {
    class Client;
  }
  namespace Items {
    class SpellbookDesc;
  }
  namespace Core {
	class Spellbook : public UContainer
	{
      typedef UContainer base;
	public:
	  explicit Spellbook( const Items::SpellbookDesc& descriptor );
	  virtual ~Spellbook();
      virtual size_t estimatedSize( ) const POL_OVERRIDE;

	  bool has_spellid( unsigned int spellid ) const;
	  bool remove_spellid( unsigned int spellid );
	  bool add_spellid( unsigned int spellid );
	  u8 bitwise_contents[8];
	  u8 spell_school;
	  virtual void add( Items::Item *item ) POL_OVERRIDE;
	  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
	  virtual void printOn( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void printSelfOn( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void double_click( Network::Client* client ) POL_OVERRIDE;
	  void send_book_old( Network::Client* client );
	  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* script_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex ) POL_OVERRIDE;

	private:
	  virtual bool can_add( const Items::Item& item ) const POL_OVERRIDE;
	  virtual void add_bulk( int item_count_delta, int weight_delta ) POL_OVERRIDE;
	  void calc_current_bitwise_contents();
	protected:

	  friend class Items::Item;
	};

	class USpellScroll : public Items::Item
	{
      typedef Items::Item base;
	public:
      virtual ~USpellScroll() {};
      virtual size_t estimatedSize( ) const POL_OVERRIDE;
	  virtual u16 get_senditem_amount() const POL_OVERRIDE;
	  static u16 convert_objtype_to_spellnum( u32 objtype, u8 school );
	protected:
	  explicit USpellScroll( const Items::ItemDesc& descriptor );
	  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );
	private:
	};

	void send_spellbook_contents( Network::Client *client, Spellbook& spellbook );
  }
}
#endif
