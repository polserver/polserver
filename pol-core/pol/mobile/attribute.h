/** @file
 *
 * @par History
 */


#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string>
#include <vector>

#include "../scrdef.h"
#include "../skillid.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Plib
{
class Package;
}
namespace Core
{
class ExportedFunction;
}
namespace Mobile
{
class Attribute
{
public:
  Attribute( const Plib::Package* pkg, Clib::ConfigElem& elem );
  explicit Attribute( const std::string& name );
  ~Attribute();
  size_t estimateSize() const;

  const Plib::Package* pkg;
  std::string name;
  unsigned attrid;

  std::vector<std::string> aliases;  // aliases[0] is always name

  Attribute* next;

  Core::ExportedFunction* getintrinsicmod_func;

  // UseSkill information
  unsigned delay_seconds;
  bool unhides;
  bool disable_core_checks;

  // Default cap
  unsigned short default_cap;

  Core::ScriptDef script_;

  Core::USKILLID skillid;

  static Attribute* FindAttribute( const std::string& str );
  static Attribute* FindAttribute( unsigned attrid );
};


void load_attributes_cfg();
void clean_attributes();
void combine_attributes_skillid();

const unsigned ATTRIBUTE_MIN_EFFECTIVE = 0;
const unsigned ATTRIBUTE_MAX_EFFECTIVE = 6000;

const unsigned ATTRIBUTE_MIN_BASE = 0;
const unsigned ATTRIBUTE_MAX_BASE = 60000;

const short ATTRIBUTE_MIN_TEMP_MOD = -30000;
const short ATTRIBUTE_MAX_TEMP_MOD = +30000;

const short ATTRIBUTE_MIN_INTRINSIC_MOD = -30000;
const short ATTRIBUTE_MAX_INTRINSIC_MOD = +30000;
}  // namespace Mobile
}  // namespace Pol
#endif
