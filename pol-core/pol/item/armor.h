/** @file
 *
 * @par History
 */


#ifndef ARMOR_H
#define ARMOR_H

#include <iosfwd>
#include <set>
#include <string>

#include "../../bscript/bobject.h"
#include "../../clib/rawtypes.h"
#include "../dynproperties.h"
#include "../scrdef.h"
#include "equipmnt.h"

namespace Pol
{
namespace Bscript
{
class String;
class BStruct;
class Executor;
}  // namespace Bscript
namespace Clib
{
class StreamWriter;
class ConfigElem;
}  // namespace Clib
namespace Plib
{
class Package;
}
namespace Core
{
class ExportScript;
}
}  // namespace Pol
#define ARMOR_TMPL ( static_cast<const ArmorDesc*>( tmpl ) )

namespace Pol
{
namespace Items
{
class ArmorDesc;
class Item;

class UArmor final : public Equipment
{
  typedef Equipment base;

public:
  virtual ~UArmor() = default;
  unsigned short ar() const;
  unsigned short ar_base() const;
  bool covers( unsigned short zlayer ) const;
  virtual Item* clone() const override;
  virtual size_t estimatedSize() const override;

  void set_onhitscript( const std::string& scriptname );
  std::set<unsigned short> tmplzones();
  virtual bool get_method_hook( const char* methodname, Bscript::Executor* ex,
                                Core::ExportScript** hook, unsigned int* PC ) const override;

  DYN_PROPERTY( ar_mod, s16, Core::PROP_AR_MOD, 0 );

protected:
  virtual void printProperties( Clib::StreamWriter& sw ) const override;
  virtual void readProperties( Clib::ConfigElem& elem ) override;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test

  virtual Bscript::BObjectImp* set_script_member( const char* membername,
                                                  const std::string& value ) override;
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value ) override;
  virtual Bscript::BObjectImp* set_script_member_id(
      const int id, const std::string& value ) override;  // id test
  virtual Bscript::BObjectImp* set_script_member_id( const int id,
                                                     int value ) override;  // id test
  virtual bool script_isa( unsigned isatype ) const override;

  UArmor( const ArmorDesc& descriptor, const ArmorDesc* permanent_descriptor );
  friend class Item;
  // friend void load_data();
  friend void load_weapon_templates();
  friend UArmor* create_intrinsic_shield( const char* name, Clib::ConfigElem& elem,
                                          const Plib::Package* pkg );

  const ArmorDesc& descriptor() const;
  const Core::ScriptDef& onhitscript() const;

private:
  Core::ScriptDef onhitscript_;
};

void load_armor_templates();
void unload_armor_templates();

void validate_intrinsic_shield_template();
UArmor* create_intrinsic_shield_from_npctemplate( Clib::ConfigElem& elem,
                                                  const Plib::Package* pkg );
}  // namespace Items
}  // namespace Pol
#endif
