#ifndef POLTOOL_TESTFILES_H
#define POLTOOL_TESTFILES_H

#include "../plib/ustruct.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Pol
{
namespace PolTool
{
namespace fs = std::filesystem;
class FileGenerator
{
public:
  FileGenerator( fs::path basedir, bool hsa, int maxtiles, int mapid, int width, int height );
  void generateTiledata();
  void generateMap();
  void generateStatics();
  void generateMultis();

private:
  template <typename T, typename U>
  void genTiledata( std::vector<T>& land, std::vector<U>& item );
  template <typename T, typename U>
  void modifyTiledata( std::vector<T>& land, std::vector<U>& item );
  template <typename T, typename U>
  void writeTiledata( std::vector<T>& land, std::vector<U>& item );

  template <typename T>
  void modifyMultis( std::vector<std::vector<T>>& multis );
  template <typename T>
  void writeMultis( std::vector<std::vector<T>>& multis );

  void modifyMap( std::vector<std::vector<Plib::USTRUCT_MAPINFO>>& map );

  void modifyStatics( std::vector<std::vector<std::vector<Plib::USTRUCT_STATIC>>>& statics );

  template <typename T>
  static void writeFile( std::ofstream& stream, T& data );

  fs::path _basedir;
  bool _hsa;
  int _maxtiles;
  int _mapid;
  int _width;
  int _height;
};
}  // namespace PolTool
}  // namespace Pol
#endif
