/** @file
 *
 * @par History
 */


#ifndef EQUIPMNT_H
#define EQUIPMNT_H

#include <iosfwd>
#include <string>

#include "../../bscript/bobject.h"
#include "../../clib/rawtypes.h"
#include "../baseobject.h"

namespace Pol
{
namespace Clib
{
class StreamWriter;
}  // namespace Clib
namespace Items
{
class ItemDesc;
}  // namespace Items
}  // namespace Pol

#ifndef ITEM_H
#include "item.h"
#endif

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
}
namespace Core
{
class EquipDesc;
class ExportScript;
}  // namespace Core
namespace Items
{
class Equipment : public Item
{
  typedef Item base;

public:
  ~Equipment() override;
  void reduce_hp_from_hit();
  bool is_intrinsic() const;

protected:
  Equipment( const ItemDesc& itemdesc, Core::UOBJ_CLASS uobj_class,
             const Core::EquipDesc* permanent_descriptor );
  void printProperties( Clib::StreamWriter& sw ) const override;
  void readProperties( Clib::ConfigElem& elem ) override;
  Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  // virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string&
  // value );
  Bscript::BObjectImp* set_script_member( const char* membername, int value ) override;
  Bscript::BObjectImp* set_script_member_double( const char* membername, double value ) override;
  Bscript::BObjectImp* set_script_member_id( const int id,
                                             int value ) override;  // id test
  Bscript::BObjectImp* set_script_member_id_double( const int id,
                                                    double value ) override;  // id test
  bool get_method_hook( const char* methodname, Bscript::Executor* ex, Core::ExportScript** hook,
                        unsigned int* PC ) const override;

  bool script_isa( unsigned isatype ) const override;
  Item* clone() const override;

  size_t estimatedSize() const override;

  const Core::EquipDesc* tmpl;
};

Equipment* find_intrinsic_equipment( const std::string& name, u8 type );
void register_intrinsic_equipment( const std::string& name, Equipment* equip );
void insert_intrinsic_equipment( const std::string& name, Equipment* equip );
void allocate_intrinsic_equipment_serials();
void load_npc_intrinsic_equip();
}  // namespace Items
}  // namespace Pol
#endif
