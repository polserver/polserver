#ifndef POLTOOL_TESTENV_H
#define POLTOOL_TESTENV_H


#include <filesystem>


namespace Pol::PolTool
{
namespace fs = std::filesystem;

class TestEnv
{
public:
  TestEnv( fs::path basedir, bool hsa, int maxtiles, int width, int height );
  void generate();

private:
  fs::path _basedir;
  bool _hsa;
  int _maxtiles;
  int _width;
  int _height;
};
}  // namespace Pol::PolTool

#endif
