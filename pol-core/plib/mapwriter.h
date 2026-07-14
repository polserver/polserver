/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - extra qualification in class removed
 */


#ifndef PLIB_MAPWRITER_H
#define PLIB_MAPWRITER_H

#include "mapblock.h"
#include "mapsolid.h"
#include "maptile.h"

#include <string>
#include <vector>


namespace Pol::Plib
{
// Builds a realm's output files (base/solidx1/solidx2/solids/maptile.dat) in
// memory and writes each one out a single time in Flush(). The conversion path
// touches map blocks in a scattered order, so buffering avoids the seek+read /
// seek+write round-trip the old one-block cache paid on every block switch.
class MapWriter
{
public:
  MapWriter() = default;
  ~MapWriter();

  void SetMapCell( unsigned short x, unsigned short y, MAPCELL cell );
  void SetMapTile( unsigned short x, unsigned short y, MAPTILE_CELL cell );

  void Flush();

  void CreateNewFiles( const std::string& realm_name, unsigned short width, unsigned short height );
  void OpenExistingFiles( const std::string& realm_name );
  void WriteConfigFile();

  unsigned int NextSolidIndex() const;
  unsigned int NextSolidOffset() const;

  unsigned int NextSolidx2Offset() const;

  void AppendSolid( const SOLIDS_ELEM& solid );
  void AppendSolidx2Elem( const SOLIDX2_ELEM& elem );
  void SetSolidx2Offset( unsigned short x_base, unsigned short y_base, unsigned int offset );

  unsigned width() const { return _width; }
  unsigned height() const { return _height; }

private:
  unsigned int total_solid_blocks() const;
  unsigned int total_blocks() const;
  unsigned int total_maptile_blocks() const;

private:
  std::string _directory;  // "realm/<name>/", where Flush() writes the .dat files
  std::string _realm_name;
  unsigned short _width = 0;
  unsigned short _height = 0;
  bool _flushed = false;

  std::vector<MAPBLOCK> _base;
  std::vector<SOLIDX1_ELEM> _solidx1;
  std::vector<char> _solidx2;  // leading bytes are the SOLIDX2 filler
  std::vector<char> _solids;   // leading bytes are the solids filler
  std::vector<MAPTILE_BLOCK> _maptile;
};
}  // namespace Pol::Plib

#endif
