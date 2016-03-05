/** @file
 *
 * @par History
 */


#ifndef EQUIPMNT_H
#define EQUIPMNT_H

#include <iosfwd>

#ifndef ITEM_H
#include "item.h"
#endif

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}
namespace Clib
{
class ConfigElem;
}
namespace Core
{
class EquipDesc;
}
namespace Items
{

class Equipment : public Item
{
  typedef Item base;

public:
  virtual ~Equipment();
  void reduce_hp_from_hit();
  bool is_intrinsic() const;

  virtual double getQuality() const POL_OVERRIDE;
  virtual void setQuality(double value) POL_OVERRIDE;

protected:
  Equipment( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class, const Core::EquipDesc* permanent_descriptor );
  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test
  //virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value );
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member_double( const char* membername, double value ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) POL_OVERRIDE; //id test
  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value ) POL_OVERRIDE; //id test

  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
  virtual Item* clone() const POL_OVERRIDE;

  virtual size_t estimatedSize() const POL_OVERRIDE;

  const Core::EquipDesc* tmpl;

private:
  double _quality;
};

Equipment* find_intrinsic_equipment( const std::string& name, u8 type );
void register_intrinsic_equipment(const std::string& name, Equipment* equip);
void insert_intrinsic_equipment( const std::string& name, Equipment* equip );
void allocate_intrinsic_equipment_serials();
void load_npc_intrinsic_equip();
}
}
#endif
