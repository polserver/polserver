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
#include <string_view>
#include <type_traits>
#include <vector>


namespace Pol::Plib
{
// A realm output file held fully in memory: an optional raw filler prefix
// followed by a packed array of T. Recorded file offsets are byte offsets from
// the start of the file, filler included (byte_size() is exactly the offset the
// next appended element will land at). The dirty flag lives with the buffer:
// any non-const elems() handout marks the file dirty, store() writes only dirty
// files and marks them clean again, so untouched files are never rewritten and
// a destructor flush after an explicit one writes nothing twice.
template <typename T>
class OutputFile
{
  static_assert( std::is_trivially_copyable_v<T>, "OutputFile streams the raw bytes of T" );

public:
  explicit OutputFile( std::string name, std::string_view filler = {} );

  unsigned int byte_size() const
  {
    return static_cast<unsigned int>( filler_.size() + elems_.size() * sizeof( T ) );
  }

  std::vector<T>& elems()  // mutating access dirties the file
  {
    dirty_ = true;
    return elems_;
  }
  const std::vector<T>& elems() const { return elems_; }

  // Read dir+name into the buffer, verifying and stripping the filler prefix;
  // leaves the file clean. Throws with the filename on any failure, including a
  // size that is not filler plus a whole number of elements.
  void load( const std::string& dir );
  // Write filler then the element blob to dir+name, iff dirty; marks clean.
  void store( const std::string& dir );
  // For brand-new realms: the file does not exist on disk yet, so it must be
  // written even if the conversion never touches it.
  void mark_fresh() { dirty_ = true; }

private:
  std::string name_;
  std::string filler_;
  std::vector<T> elems_;
  bool dirty_ = false;
};

// Builds a realm's output files (base/solidx1/solidx2/solids/maptile.dat) in
// memory and writes each one out a single time in Flush(). The conversion path
// touches map blocks in a scattered order, so buffering avoids the seek+read /
// seek+write round-trip the old one-block cache paid on every block switch.
class MapWriter
{
public:
  MapWriter();
  ~MapWriter();

  void SetMapCell( unsigned short x, unsigned short y, MAPCELL cell );
  // Overwrite a whole 8x8 base.dat block at once; x_base/y_base must be
  // block-aligned (multiples of MAPBLOCK_CHUNK).
  void SetMapBlock( unsigned short x_base, unsigned short y_base, const MAPBLOCK& block );
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

  OutputFile<MAPBLOCK> _base;
  OutputFile<SOLIDX1_ELEM> _solidx1;
  OutputFile<SOLIDX2_ELEM> _solidx2;
  OutputFile<SOLIDS_ELEM> _solids;
  OutputFile<MAPTILE_BLOCK> _maptile;
};
}  // namespace Pol::Plib

#endif
