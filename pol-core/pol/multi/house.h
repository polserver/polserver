/*
History
=======
2005/06/06 Shinigami: added readobjects - to get a list of statics
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#ifndef _HOUSE_H
#define _HOUSE_H

#include "customhouses.h"
#include "multi.h"
#include "../reftypes.h"

#include <list>
#include <vector>

namespace Pol {
  namespace Multi {

	typedef std::list<Items::Item*>         ItemList;
    typedef std::list<Mobile::Character*>    MobileList;


	class UHouse : public UMulti
	{
	  typedef UMulti base;

	public:
	  static Bscript::BObjectImp* scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y, s8 z, Plib::Realm* realm, int flags );
	  void destroy_components();

	  CustomHouseDesign CurrentDesign;
	  CustomHouseDesign WorkingDesign;
	  CustomHouseDesign BackupDesign;
      std::vector<u8> CurrentCompressed;
      std::vector<u8> WorkingCompressed;

	  bool IsCustom() const { return custom; };
	  void SetCustom( bool custom );
	  void CustomHouseSetInitialState();
	  static UHouse* FindWorkingHouse( u32 chrserial );
	  bool IsEditing() const { return editing; }
	  bool IsWaitingForAccept() const { return waiting_for_accept; }
	  bool editing;
	  bool waiting_for_accept;
	  int editing_floor_num;
	  u32 revision;

	  virtual void register_object( Core::UObject* obj ) POL_OVERRIDE;
	  virtual void unregister_object( Core::UObject* obj ) POL_OVERRIDE;

	  virtual void walk_on( Mobile::Character* chr ) POL_OVERRIDE;

	  void ClearSquatters();
	  void add_component( Items::Item* item, s32 xoff, s32 yoff, u8 zoff );
	  static void list_contents( const UHouse* house,
								 ItemList& items_in,
								 MobileList& chrs_in );
	  void AcceptHouseCommit( Mobile::Character* chr, bool accept );
	  void CustomHousesQuit( Mobile::Character* chr, bool drop_changes );

      virtual ~UHouse() {};
      virtual size_t estimatedSize() const POL_OVERRIDE;

	protected:
	  explicit UHouse( const Items::ItemDesc& itemdesc );
	  void create_components();

	  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
	  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* script_method( const char* membername, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test
	  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
	  virtual class UHouse* as_house() POL_OVERRIDE;
      virtual bool readshapes( Plib::MapShapeList& vec, short x, short y, short zbase ) POL_OVERRIDE;
	  virtual bool readobjects( Core::StaticList& vec, short x, short y, short zbase ) POL_OVERRIDE;
	  Bscript::ObjArray* component_list() const;
	  Bscript::ObjArray* items_list( ) const;
	  Bscript::ObjArray* mobiles_list( ) const;

	  friend class UMulti;
	  friend class CustomHouseDesign;

	  typedef Core::ItemRef Component;
	  typedef std::vector< Component > Components;
	  Components* get_components() { return &components_; }
	  bool custom;
	private:
	  typedef Core::UObjectRef Squatter;
	  typedef std::vector< Squatter > Squatters;
	  Squatters squatters_;

	  Components components_;
	};


	Bscript::BObjectImp* destroy_house( UHouse* house );
  }
}
#endif
