/** @file
 *
 * @par History
 */


#ifndef CLIB_CFGSECT_H
#define CLIB_CFGSECT_H

#include <string>


namespace Pol::Clib
{
class ConfigFile;
class ConfigElem;

const unsigned CST_NORMAL = 0;
const unsigned CST_MANDATORY = 1;
const unsigned CST_UNIQUE = 2;

class ConfigSection
{
public:
  ConfigSection( ConfigFile& cf, const std::string& sectname,
                 unsigned flags = CST_MANDATORY | CST_UNIQUE );
  ~ConfigSection() noexcept( false );
  ConfigSection& operator=( const ConfigSection& ) { return *this; }
  bool matches( const ConfigElem& elem );

private:
  ConfigFile& _cf;
  std::string _sectname;
  bool _found;
  unsigned _flags;
};
}  // namespace Pol::Clib

#endif
