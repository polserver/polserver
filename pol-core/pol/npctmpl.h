/** @file
 *
 * @par History
 * - 2010/02/03 Turley:    MethodScript support for mobiles
 */


#ifndef NPCTMPL_H
#define NPCTMPL_H

#include <map>
#include <memory>
#include <stddef.h>
#include <string>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/rawtypes.h"

namespace Pol
{
namespace Plib
{
class Package;
}
namespace Items
{
class UArmor;
class UWeapon;
}  // namespace Items
namespace Core
{
class ExportScript;

class NpcTemplate : public std::enable_shared_from_this<NpcTemplate>
{
public:
  std::string name;

  Items::UWeapon* intrinsic_weapon;
  Items::UArmor* intrinsic_shield;
  const Plib::Package* pkg;

  // std::string script;
  enum ALIGNMENT : u8
  {
    NEUTRAL,
    EVIL,
    GOOD
  } alignment;

  ExportScript* method_script;

  NpcTemplate( const Clib::ConfigElem& elem, const Plib::Package* pkg );
  size_t estimateSize() const;
  ~NpcTemplate();
};

std::shared_ptr<NpcTemplate> find_npc_template( const Clib::ConfigElem& elem );
bool FindNpcTemplate( const char* template_name, Clib::ConfigElem& elem );
bool FindNpcTemplate( const char* template_name, Clib::ConfigFile& cf, Clib::ConfigElem& elem );

class NpcTemplateConfigSource : public Clib::ConfigSource
{
public:
  NpcTemplateConfigSource();
  NpcTemplateConfigSource( const Clib::ConfigFile& cf );
  size_t estimateSize() const;
  void display_error( const std::string& msg, bool show_curline = true,
                      const Clib::ConfigElemBase* elem = nullptr,
                      bool error = true ) const override;

private:
  std::string _filename;
  unsigned _fileline;
};

class NpcTemplateElem
{
public:
  NpcTemplateElem();
  NpcTemplateElem( const Clib::ConfigFile& cf, const Clib::ConfigElem& elem );
  size_t estimateSize() const;
  void copyto( Clib::ConfigElem& elem ) const;

private:
  NpcTemplateConfigSource _source;
  Clib::ConfigElem _elem;
};
}  // namespace Core
}  // namespace Pol
#endif
