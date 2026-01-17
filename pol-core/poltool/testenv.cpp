#include "testenv.h"

#include "../clib/logfacility.h"
#include "baredistro.h"
#include "testfiles.h"

#include "pol_global_config.h"

#include <fstream>
#include <utility>


namespace Pol::PolTool
{
TestEnv::TestEnv( fs::path basedir, bool hsa, int maxtiles, int width, int height )
    : _basedir( std::move( basedir ) ),
      _hsa( hsa ),
      _maxtiles( maxtiles ),
      _width( width ),
      _height( height )
{
  if ( _basedir.empty() )
    _basedir = ".";
}
void TestEnv::generate()
{
  if ( _basedir != "." && !fs::exists( _basedir ) )
    fs::create_directories( _basedir );

  fs::path clientdir = _basedir / "client";
  FileGenerator g( clientdir, _hsa, _maxtiles, 0, _width, _height );
  g.generateTiledata();
  g.generateMap();
  g.generateStatics();
  g.generateMultis();

  BareDistro distro( _basedir, _hsa, _maxtiles, _width, _height );
  distro.generate();

  fs::path datadir = _basedir / "data";
  if ( !fs::exists( datadir ) )
    fs::create_directories( datadir );
  std::ofstream afile( datadir / "accounts.txt", std::ofstream::out );
  afile.close();
  std::ofstream pfile( datadir / "pol.txt", std::ofstream::out );
  pfile << "System\n{\n\tCoreVersion " << POL_VERSION_STR << "\n}";
  pfile.close();
  std::ofstream bfile( _basedir / "config" / "boats.cfg", std::ofstream::out );
  bfile.close();
}
}  // namespace Pol::PolTool
