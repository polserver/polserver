#ifndef POLTOOL_TESTENV_H
#define POLTOOL_TESTENV_H


#include <map>
#include <string>
#include <vector>

namespace Pol
{
namespace PolTool
{
class TestEnv
{
public:
  TestEnv( std::string basedir, bool hsa, int maxtiles, int width, int height );
  void generate();

private:

  std::string _basedir;
  bool _hsa;
  int _maxtiles;
  int _width;
  int _height;
};
}  // namespace PolTool
}  // namespace Pol
#endif
