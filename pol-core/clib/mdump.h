/** @file
 *
 * @par History
 */


#ifndef CLIB_MDUMP_H
#define CLIB_MDUMP_H

#include <string>

namespace Pol
{
namespace Clib
{
class MiniDumper
{
public:
  static void Initialize();
  static void SetMiniDumpType( const std::string& dumptype );
};
}
}
#endif //CLIB_MDUMP_H
