/** @file
 *
 * @par History
 * - 2005/09/14 Shinigami: regen_while_dead implemented
 */


#ifndef VITAL_H
#define VITAL_H

#include <string>
#include <vector>

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

class Vital
{
public:
  Vital( const Plib::Package* pkg, Clib::ConfigElem& elem );
  ~Vital();
  size_t estimateSize() const;

  const Plib::Package* pkg;
  std::string name;
  std::vector<std::string> aliases;  // aliases[0] is always name
  unsigned vitalid;

  ExportedFunction* get_regenrate_func;
  ExportedFunction* get_maximum_func;
  ExportedFunction* depleted_func;
  bool regen_while_dead;
};

Vital* FindVital( const std::string& vitalname );
void clean_vitals();

const int VITAL_LOWEST_REGENRATE = -30000;
const int VITAL_HIGHEST_REGENRATE = 30000;

// const unsigned VITAL_MIN_VALUE = 0; // currently not needed
const unsigned VITAL_MAX_VALUE = 100000L;

const unsigned int VITAL_MAX_HUNDREDTHS = 10000000L;  // 10,000,000 hundredths = 100,000.00

const unsigned int VITAL_LOWEST_MAX_HUNDREDTHS = 100L;        // 100 hundredths = 1.00
const unsigned int VITAL_HIGHEST_MAX_HUNDREDTHS = 10000000L;  // 10,000,000 hundredths = 100,000.00
}  // namespace Core
}  // namespace Pol
#endif
