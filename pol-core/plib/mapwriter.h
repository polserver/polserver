/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - extra qualification in class removed
 */


#ifndef PLIB_MAPWRITER_H
#define PLIB_MAPWRITER_H

#include "mapblock.h"
#include "maptile.h"

#include <fstream>
#include <string>


namespace Pol::Plib
{
struct SOLIDS_ELEM;
struct SOLIDX2_ELEM;
struct MAPCELL;

class MapWriter
{
public:
  MapWriter();
  ~MapWriter();

  void SetMapCell( unsigned short x, unsigned short y, MAPCELL cell );
  void SetMapTile( unsigned short x, unsigned short y, MAPTILE_CELL cell );

  void Flush();

  void CreateNewFiles( const std::string& realm_name, unsigned short width, unsigned short height );
  void OpenExistingFiles( const std::string& realm_name );
  void WriteConfigFile();

  void CreateBaseDat( const std::string& realm_name, const std::string& directory );
  void CreateSolidx1Dat( const std::string& realm_name, const std::string& directory );
  void CreateSolidx2Dat( const std::string& realm_name, const std::string& directory );
  void CreateSolidsDat( const std::string& realm_name, const std::string& directory );
  void CreateMaptileDat( const std::string& realm_name, const std::string& directory );

  unsigned int NextSolidIndex();
  unsigned int NextSolidOffset();

  unsigned int NextSolidx2Offset();

  void AppendSolid( const SOLIDS_ELEM& solid );
  void AppendSolidx2Elem( const SOLIDX2_ELEM& elem );
  void SetSolidx2Offset( unsigned short x_base, unsigned short y_base, unsigned int offset );

  unsigned width() const { return _width; }
  unsigned height() const { return _height; }

private:
  std::fstream::pos_type total_size();

  unsigned int total_solid_blocks();
  unsigned int total_blocks();
  unsigned int total_maptile_blocks();

  void FlushBaseFile();
  void FlushMapTileFile();

private:
  std::string _realm_name;
  unsigned short _width;
  unsigned short _height;
  std::fstream _ofs_base;
  int _cur_mapblock_index;
  MAPBLOCK _block;

  std::fstream _ofs_solidx1;
  std::fstream _ofs_solidx2;
  std::fstream _ofs_solids;

  std::fstream _ofs_maptile;
  int _cur_maptile_index;
  MAPTILE_BLOCK _maptile_block;
  size_t solidx2_offset;
  size_t solids_offset;
};
}  // namespace Pol::Plib

#endif
