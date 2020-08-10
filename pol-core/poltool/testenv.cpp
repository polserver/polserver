#include "testenv.h"

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "baredistro.h"
#include "testfiles.h"

#include "pol_global_config.h"

#include <fstream>
#include <utility>

namespace Pol
{
namespace PolTool
{
TestEnv::TestEnv( std::string basedir, bool hsa, int maxtiles, int width, int height )
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
  if ( _basedir != "." )
    Clib::make_dir( _basedir.c_str() );

  std::string clientdir = _basedir + "/client";
  FileGenerator g( clientdir, _hsa, _maxtiles, _width, _height );
  g.generateTiledata();
  g.generateMap();
  g.generateStatics();
  g.generateMultis();

  BareDistro distro( _basedir, _hsa, _maxtiles, _width, _height );
  distro.generate();

  std::string datadir = _basedir + "/data";
  Clib::make_dir( datadir.c_str() );
  std::ofstream afile( datadir + "/accounts.txt", std::ofstream::out );
  afile.close();
  std::ofstream pfile( datadir + "/pol.txt", std::ofstream::out );
  pfile << "System\n{\n\tCoreVersion " << POL_VERSION << "\n}";
  pfile.close();
  std::ofstream bfile( _basedir + "/config/boats.cfg", std::ofstream::out );
  bfile.close();
}
}  // namespace PolTool
}  // namespace Pol
