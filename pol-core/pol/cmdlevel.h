/** @file
 *
 * @par History
 */

#ifndef __CMDLEVEL_H
#define __CMDLEVEL_H

#include <memory>
#include <string>
#include <vector>

namespace Pol
{
namespace Bscript
{
class ObjArray;
class BDictionary;
}  // namespace Bscript
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
class CmdLevel
{
public:
  CmdLevel( Clib::ConfigElem& elem, int cmdlevelnum );
  size_t estimateSize() const;

  bool matches( const std::string& name ) const;
  void add_searchdir( Plib::Package* pkg, const std::string& dir );
  void add_searchdir_front( Plib::Package* pkg, const std::string& dir );

  std::string name;
  unsigned char cmdlevel;

  struct SearchDir
  {
    Plib::Package* pkg;
    std::string dir;
  };

  typedef std::vector<SearchDir> SearchList;
  SearchList searchlist;

  typedef std::vector<std::string> Aliases;
  Aliases aliases;
};

CmdLevel* find_cmdlevel( const char* name );
CmdLevel* FindCmdLevelByAlias( const std::string& str );

std::unique_ptr<Bscript::BDictionary> ListAllCommandsInPackage( Plib::Package* m_pkg,
                                                                int max_cmdlevel = -1 );
std::unique_ptr<Bscript::ObjArray> ListCommandsInPackageAtCmdlevel( Plib::Package* m_pkg,
                                                                    int cmdlvl_num );

}  // namespace Core
}  // namespace Pol
#endif
