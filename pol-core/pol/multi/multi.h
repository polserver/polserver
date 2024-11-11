/** @file
 *
 * @par History
 * - 2005/06/06 Shinigami: added readobjects template - to get a list of statics
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#ifndef MULTI_H
#define MULTI_H

#include "plib/udatfile.h"

// also consider: multimap<unsigned int, unsigned int>
// unsigned int is (x << 16) | y
// unsigned int is z << 16 | objtype
// hell, even multimap<unsigned short,unsigned int>
// unsigned short is (x<<8)|y
// (relative signed x/y, -128 to +127
// unsigned int is z << 16 | objtype

#ifndef ITEM_H
#include "item/item.h"
#endif

#include "base/range.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class BStruct;
class Executor;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Plib
{
class MapShapeList;
}
namespace Realms
{
class Realm;
}
namespace Core
{
class ExportScript;
}
namespace Multi
{
class MultiDef;

const int CRMULTI_IGNORE_MULTIS = 0x0001;
const int CRMULTI_IGNORE_OBJECTS = 0x0002;
const int CRMULTI_IGNORE_FLATNESS = 0x0004;
const int CRMULTI_RECREATE_COMPONENTS = 0x0008;

const int CRMULTI_FACING_NORTH = 0x0000;
const int CRMULTI_FACING_EAST = 0x0100;
const int CRMULTI_FACING_SOUTH = 0x0200;
const int CRMULTI_FACING_WEST = 0x0300;
const int CRMULTI_FACING_MASK = 0x0300;
const int CRMULTI_FACING_SHIFT = 8;

class UMulti : public Items::Item
{
  typedef Items::Item base;

public:
  static UMulti* create( const Items::ItemDesc& descriptor, u32 serial = 0 );

  static Bscript::BObjectImp* scripted_create( const Items::ItemDesc& descriptor,
                                               const Core::Pos4d& pos, int flags );

  u16 multiid() const { return multiid_; }

  bool items_decay() const { return items_decay_; }
  void items_decay( bool decay );

  virtual void double_click( Network::Client* client ) override;
  virtual void register_object( UObject* obj );
  virtual void unregister_object( UObject* obj );
  virtual bool script_isa( unsigned isatype ) const override;

  virtual bool setgraphic( u16 newobjtype ) override;

  const MultiDef& multidef() const;
  virtual class UBoat* as_boat();
  virtual class UHouse* as_house();
  virtual Bscript::BObjectImp* make_ref() override;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;
  using Items::Item::set_script_member_id;
  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) override;
  virtual bool get_method_hook( const char* methodname, Bscript::Executor* ex,
                                Core::ExportScript** hook, unsigned int* PC ) const override;

  Bscript::BStruct* footprint() const;
  virtual bool readshapes( Plib::MapShapeList& vec, s16 rx, s16 ry, s16 zbase );
  virtual bool readobjects( Plib::StaticList& vec, s16 rx, s16 ry, s16 zbase );
  virtual Core::Range3d current_box() const;

  virtual ~UMulti();
  virtual size_t estimatedSize() const override;

  virtual u8 visible_size() const override;

  virtual void readProperties( Clib::ConfigElem& elem ) override;
  virtual void printProperties( Clib::StreamWriter& sw ) const override;

protected:
  explicit UMulti( const Items::ItemDesc& itemdesc );

  virtual const char* classname() const override;

  friend class ref_ptr<UMulti>;

  u16 multiid_;

  bool items_decay_;

  // virtual void destroy(void);
};

/* Default implementations */
inline bool UMulti::readshapes( Plib::MapShapeList& /*vec*/, s16 /*rx*/, s16 /*ry*/, s16 /*zbase*/ )
{
  return false;
};
inline bool UMulti::readobjects( Plib::StaticList& /*vec*/, s16 /*rx*/, s16 /*ry*/, s16 /*zbase*/ )
{
  return false;
};
}  // namespace Multi
namespace Core
{
void send_multi( Network::Client* client, const Multi::UMulti* multi );
void send_multi_to_inrange( const Multi::UMulti* multi );
}  // namespace Core
}  // namespace Pol
#endif
