/** @file
 *
 * @par History
 * - 2005/06/06 Shinigami: added readobjects - to get a list of statics
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */

#ifndef _HOUSE_H
#define _HOUSE_H

#include <list>
#include <vector>

#include "../../bscript/bobject.h"
#include "../../clib/rawtypes.h"
#include "../../plib/udatfile.h"
#include "../item/item.h"
#include "../reftypes.h"
#include "customhouses.h"
#include "multi.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Core
{
class UObject;
class UOExecutor;
class ExportScript;
}  // namespace Core
namespace Items
{
class ItemDesc;
}  // namespace Items
namespace Mobile
{
class Character;
}  // namespace Mobile
namespace Plib
{
class MapShapeList;
}  // namespace Plib
namespace Realms
{
class Realm;
}  // namespace Realms
}  // namespace Pol

namespace Pol
{
namespace Multi
{
typedef std::list<Items::Item*> ItemList;
typedef std::list<Mobile::Character*> MobileList;


class UHouse final : public UMulti
{
  typedef UMulti base;

  // Components are not removed from the list when destroyed by the core,
  // so you should always check if the component is an orphan.
  typedef Core::ItemRef Component;
  typedef std::vector<Component> Components;

public:
  static Bscript::BObjectImp* scripted_create( const Items::ItemDesc& descriptor,
                                               const Core::Pos4d& pos, int flags );
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

  virtual void register_object( Core::UObject* obj ) override;
  virtual void unregister_object( Core::UObject* obj ) override;

  virtual void walk_on( Mobile::Character* chr ) override;

  void ClearSquatters();
  bool add_component( Items::Item* item, s32 xoff, s32 yoff, s16 zoff );
  bool add_component( Component component );
  static void list_contents( const UHouse* house, ItemList& items_in, MobileList& chrs_in );
  void AcceptHouseCommit( Mobile::Character* chr, bool accept );
  void CustomHousesQuit( Mobile::Character* chr, bool drop_changes, bool send_pkts = true );

  virtual ~UHouse() = default;
  virtual size_t estimatedSize() const override;
  virtual bool get_method_hook( const char* methodname, Bscript::Executor* ex,
                                Core::ExportScript** hook, unsigned int* PC ) const override;

protected:
  explicit UHouse( const Items::ItemDesc& itemdesc );
  void create_components();

  virtual void readProperties( Clib::ConfigElem& elem ) override;
  virtual void printProperties( Clib::StreamWriter& sw ) const override;
  virtual Bscript::BObjectImp* script_method( const char* membername,
                                              Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* script_method_id( const int id, Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  virtual bool script_isa( unsigned isatype ) const override;
  virtual class UHouse* as_house() override;
  virtual bool readshapes( Plib::MapShapeList& vec, const Core::Vec2d& rxy, short zbase ) override;
  virtual bool readobjects( Plib::StaticList& vec, const Core::Vec2d& rxy, short zbase ) override;
  Bscript::ObjArray* component_list() const;
  Bscript::ObjArray* items_list() const;
  Bscript::ObjArray* mobiles_list() const;

  friend class UMulti;
  friend class CustomHouseDesign;

  Components* get_components() { return &components_; }
  bool custom;

private:
  typedef Core::UObjectRef Squatter;
  typedef std::vector<Squatter> Squatters;
  Squatters squatters_;

  /**
   * Checks if item can be added as component:
   * an Item can't be a component in two houses
   *
   * @param item Pointer to the item to be added
   */
  inline bool can_add_component( const Items::Item* item ) { return item->house() == nullptr; }
  /**
   * Adds an Item as component, performs no checks, internal usage
   */
  inline void add_component_no_check( Component item )
  {
    item->house( this );
    components_.push_back( item );
  }

  /**
   * Stores ItemRefs that are part of this house (eg. sign, doors, etc...)
   *
   * @warning Do not add items directly, use add_component(component) instead, because
   *          the mattching house() dinamyc property must be set on the item being added
   * @see add_component
   */
  Components components_;
};


Bscript::BObjectImp* destroy_house( UHouse* house );
}  // namespace Multi
}  // namespace Pol
#endif
