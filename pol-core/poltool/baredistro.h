#ifndef POLTOOL_BAREDISTRO_H
#define POLTOOL_BAREDISTRO_H


#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace Pol
{
namespace PolTool
{
namespace fs = std::filesystem;

class BareDistro
{
public:
  BareDistro( fs::path basedir, bool hsa, int maxtiles, int width, int height );
  void generate();

private:
  void distro_files( std::map<fs::path, std::vector<std::string>>& distro );

  fs::path _basedir;
  bool _hsa;
  int _maxtiles;
  int _width;
  int _height;
};
}  // namespace PolTool
}  // namespace Pol
#endif
