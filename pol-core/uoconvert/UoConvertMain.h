#ifndef UO_CONVERT_MAIN_H
#define UO_CONVERT_MAIN_H

#include <set>
#include <string>
#include <vector>

#include "../clib/Program/ProgramMain.h"
#include "../plib/mapwriter.h"

namespace Pol
{
namespace Plib
{
struct USTRUCT_MULTI_ELEMENT;
}
namespace UoConvert
{
struct TerrainPlane;

// Per-block output of the pure compute phase (Phase A) of the solid-block
// conversion. ComputeSolidBlock() fills one of these from immutable inputs
// (terrain plane + statics + config) without touching the MapWriter; StitchBlock()
// then folds it into the MapWriter in block order. Sized to the 8x8 solid block
// (SOLIDX_X_SIZE/SOLIDX_Y_SIZE from plib/mapsolid.h).
struct BlockResult
{
  // Final map cell (flags incl. MORE_SOLIDS) for each in-bounds tile of the block.
  Plib::MAPCELL cells[SOLIDX_X_SIZE][SOLIDX_Y_SIZE];
  // Element offset of each cell's first solid, relative to this block's first solid
  // (block-local, 0-based). Zero for cells without runs, matching the old SOLIDX2_ELEM
  // which left those slots at 0; the stitch copies this array wholesale.
  unsigned short addindex[SOLIDX_X_SIZE][SOLIDX_Y_SIZE] = {};
  // Edge-clamped extent actually computed (blocks at the map edge are partial).
  unsigned short x_add_max = SOLIDX_X_SIZE;
  unsigned short y_add_max = SOLIDX_Y_SIZE;
  bool has_solids = false;  // any cell produced a run -> this block needs a solidx2 elem
  std::vector<Plib::SOLIDS_ELEM> solids;  // this block's runs, in cell order
  std::vector<std::string> warnings;      // invalid-ID messages, replayed in order in the stitch
  // Stat sums, reduced in the stitch:
  unsigned nonempty_locations = 0;  // cells with MORE_SOLIDS -> feeds with_more_solids
  unsigned total_statics = 0;
  // Opt-in per-tile profiling sub-sums (see cfg_profile); reduced in the stitch.
  long long prof_mapinfo_ns = 0;
  long long prof_statics_ns = 0;
  long long prof_shape_ns = 0;
};

class UoConvertMain final : public Pol::Clib::ProgramMain
{
public:
  UoConvertMain();
  ~UoConvertMain() override;

  std::set<unsigned int> BoatTypes;
  std::set<unsigned int> MountTypes;
  std::set<unsigned int> DiscardedWaterTypes;

  bool cfg_use_no_shoot;
  bool cfg_LOS_through_windows;

  // When set (via `profile=1`), ComputeSolidBlock accumulates per-tile timing into the
  // BlockResult and StitchBlock reduces it into the prof_*_ns members below, so create_map
  // can report a breakdown of the hot loop. Off by default because the per-tile chrono
  // reads add measurable overhead across ~25M tiles.
  bool cfg_profile = false;
  long long prof_mapinfo_ns = 0;  // safe_getmapinfo + get_lowestadjacentz + landtile flags
  long long prof_statics_ns = 0;  // readstatics + flag filter
  long long prof_shape_ns = 0;    // water/wall pass + sort + shape consolidation
  long long prof_writer_ns = 0;   // SetMapCell + AppendSolid

  void display_flags();

  void write_flags( FILE* fp, unsigned int flags );

  void create_tiles_cfg();
  void create_landtiles_cfg();

  void create_multis_cfg();
  void create_multis_cfg( FILE* multi_idx, FILE* multi_mul, FILE* multis_cfg );

  void write_multi( FILE* multis_cfg, unsigned id,
                    std::vector<Plib::USTRUCT_MULTI_ELEMENT>& multi_elems );
  void write_multi( FILE* multis_cfg, unsigned id, FILE* multi_mul, unsigned int offset,
                    unsigned int length );

  void create_map( const std::string& realm, unsigned short width, unsigned short height );
  void update_map( const std::string& realm, unsigned short x, unsigned short y );

  void create_maptile( const std::string& realmname );

  // Phase A: pure per-block compute. Reads only immutable inputs; touches no
  // MapWriter state and no UoConvertMain counters (stats/profiling accumulate into
  // the returned BlockResult). Safe to call concurrently for distinct blocks.
  BlockResult ComputeSolidBlock( unsigned short x_base, unsigned short y_base,
                                 const TerrainPlane& plane ) const;

  // Phase B: fold one block's result into the MapWriter in block order. Must be
  // called in the same y-outer/x-inner order the blocks are laid out so the
  // solids/solidx2 append offsets (baked into solidx1/solidx2) stay identical.
  void StitchBlock( Plib::MapWriter& mapwriter, unsigned short x_base, unsigned short y_base,
                    const BlockResult& result );

  std::string resolve_type_from_id( unsigned id ) const;
  void write_multi_element( FILE* multis_cfg, const Plib::USTRUCT_MULTI_ELEMENT& elem,
                            const std::string& mytype, bool& first );

  unsigned empty = 0, nonempty = 0;
  unsigned total_statics = 0;
  unsigned with_more_solids = 0;

protected:
  int main() override;

  void check_for_errors_in_map_parameters();

  bool convert_uop_to_mul();

  void setup_uoconvert();
  void load_uoconvert_cfg();

private:
  void showHelp() override;
};
}  // namespace UoConvert
}  // namespace Pol

#endif  // UO_CONVERT_MAIN_H
