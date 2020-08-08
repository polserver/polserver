#include "baredistro.h"

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"

#include <fstream>
#include <utility>

namespace Pol
{
namespace PolTool
{
BareDistro::BareDistro( std::string basedir, bool hsa, int maxtiles, int width, int height )
    : _basedir( basedir ), _hsa( hsa ), _maxtiles( maxtiles ), _width( width ), _height( height )
{
}
void BareDistro::generate()
{
  std::map<std::string, std::vector<std::string>> distro;
  distro_files( distro );

  for ( const auto& file : distro )
  {
    std::string relpath = _basedir + "/" + file.first;
    std::string basedir = relpath;
    Clib::strip_one( basedir );
    Clib::make_dir( basedir.c_str() );
    std::ofstream ofile( relpath, std::ofstream::out );
    for ( const auto& line : file.second )
    {
      ofile << line << "\n";
    }
    ofile.close();
  }
}

}  // namespace PolTool
}  // namespace Pol
