#ifndef POLTOOL_BAREDISTRO_H
#define POLTOOL_BAREDISTRO_H


#include <map>
#include <string>
#include <vector>

namespace Pol
{
namespace PolTool
{
class BareDistro
{
public:
  BareDistro( std::string basedir, bool hsa, int maxtiles, int width, int height );
  void generate();

private:
  void distro_files( std::map<std::string, std::vector<std::string>>& distro );

  std::string _basedir;
  bool _hsa;
  int _maxtiles;
  int _width;
  int _height;
};
}  // namespace PolTool
}  // namespace Pol
#endif
