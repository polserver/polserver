/** @file
 *
 * @par History
 */


#ifndef CLIB_MDUMP_H
#define CLIB_MDUMP_H

#include <string>


namespace Pol::Clib
{
class MiniDumper
{
public:
  static void Initialize();
  static void SetMiniDumpType( const std::string& dumptype );
};
}  // namespace Pol::Clib

#endif  // CLIB_MDUMP_H
