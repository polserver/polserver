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

#include "../../clib/stl_inc.h"
#include "customhouses.h"
#include "multi.h"
namespace Pol {
  namespace Multi {

	typedef list<Items::Item*>         ItemList;
	typedef list<Mobile::Character*>    MobileList;


	class UHouse : public UMulti
	{
	  typedef UMulti base;

	public:
	  static Bscript::BObjectImp* scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y, s8 z, Plib::Realm* realm, int flags );
	  void destroy_components();

	  CustomHouseDesign CurrentDesign;
	  CustomHouseDesign WorkingDesign;
	  CustomHouseDesign BackupDesign;
	  vector<u8> CurrentCompressed;
	  vector<u8> WorkingCompressed;

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

	  virtual void register_object( Core::UObject* obj );
	  virtual void unregister_object( Core::UObject* obj );

	  virtual void walk_on( Mobile::Character* chr );

	  void ClearSquatters();
	  void add_component( Items::Item* item, s32 xoff, s32 yoff, u8 zoff );
	  static void list_contents( const UHouse* house,
								 ItemList& items_in,
								 MobileList& chrs_in );
	  void AcceptHouseCommit( Mobile::Character* chr, bool accept );
	  void CustomHousesQuit( Mobile::Character* chr, bool drop_changes );

      virtual ~UHouse() {};
      virtual size_t estimatedSize() const;

	protected:
	  explicit UHouse( const Items::ItemDesc& itemdesc );
	  void create_components();

	  virtual void readProperties( Clib::ConfigElem& elem );
	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual Bscript::BObjectImp* script_method( const char* membername, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  virtual bool script_isa( unsigned isatype ) const;
	  virtual class UHouse* as_house();
      virtual bool readshapes( Plib::MapShapeList& vec, short x, short y, short zbase );
	  virtual bool readobjects( Core::StaticList& vec, short x, short y, short zbase );
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
