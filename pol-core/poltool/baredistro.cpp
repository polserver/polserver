#include "baredistro.h"

#include "../clib/logfacility.h"

#include <fstream>
#include <utility>

namespace Pol
{
namespace PolTool
{
BareDistro::BareDistro( fs::path basedir, bool hsa, int maxtiles, int width, int height )
    : _basedir( std::move( basedir ) ),
      _hsa( hsa ),
      _maxtiles( maxtiles ),
      _width( width ),
      _height( height )
{
}
void BareDistro::generate()
{
  INFO_PRINT2( "Generating minimal distro files" );
  std::map<fs::path, std::vector<std::string>> distro;
  distro_files( distro );

  for ( const auto& file : distro )
  {
    fs::path relpath = _basedir / file.first;
    fs::path basedir = relpath;
    if ( basedir.has_filename() )
      basedir.remove_filename();
    if ( !fs::exists( basedir ) )
      fs::create_directories( basedir );
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
