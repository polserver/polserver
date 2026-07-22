#ifndef PLIB_STATICSCACHE_H
#define PLIB_STATICSCACHE_H

#include <cstdio>
#include <map>
#include <vector>

#include "../ustruct.h"
#include "clib/UniqueFile.h"
#include "clib/rawtypes.h"


namespace Pol::Plib
{
// One statics block's cached records (filled by full_read, served by getstaticblock).
struct USTRUCT_STATIC_BUFFER
{
  std::vector<USTRUCT_STATIC> statics;
  int count = 0;
};

// The statics cache: owns the staidx/statics.mul handles (plus the optional
// stadif* patch files), the dif-merged per-block record cache, and the staidx
// diff index. A collaborator of UoClientFiles, which delegates getstaticblock
// and composes readstatics_block on top (that query also needs the tiledata
// cache, so it stays on the coordinator). The map dimensions are captured at
// full_read() -- getstaticblock indexes by them rather than re-reading the
// coordinator's live dimensions per call. Queries are const and assert the cache
// is loaded rather than loading lazily; see uoclientfiles.h for the lifecycle.
class StaticsCache
{
public:
  StaticsCache() = default;
  ~StaticsCache() = default;
  StaticsCache( const StaticsCache& ) = delete;
  StaticsCache& operator=( const StaticsCache& ) = delete;

  // Open staidx<mapid>/statics<mapid>.mul (map1 falls back to the map0 files),
  // and -- when usedif and present -- the stadifl/stadifi/stadif patch files.
  void open( int mapid, bool usedif );
  // Read the staidx dif index (block -> stadif record); sets num_patches().
  void read_difflist();
  // Bulk-read staidx/statics + merge the stadif patches into the per-block cache.
  // Captures the map dimensions for later getstaticblock indexing. The two limits
  // are uoconvert's *StaticsPerBlock config knobs (assert / warn thresholds).
  void full_read( unsigned short width, unsigned short height, int max_per_block,
                  int warning_per_block );

  bool loaded() const { return loaded_; }
  unsigned int num_patches() const { return num_patches_; }
  // The open staidx/statics handles, for the uotool diagnostics that scan them
  // directly (staticsmax, the water scan).
  FILE* staidx_file() const { return staidx_.get(); }
  FILE* statics_file() const { return statics_.get(); }

  // The cached records for the 8x8 block containing (x,y). Asserts full_read() ran.
  const std::vector<USTRUCT_STATIC>& getstaticblock( unsigned short x, unsigned short y ) const;

private:
  Clib::UniqueFile staidx_;
  Clib::UniqueFile statics_;
  Clib::UniqueFile stadifl_;
  Clib::UniqueFile stadifi_;
  Clib::UniqueFile stadif_;

  std::vector<USTRUCT_STATIC_BUFFER> buffer_vec_;
  bool loaded_ = false;
  std::map<unsigned int, unsigned int> stadifl_index_;  // block -> stadif index
  unsigned int num_patches_ = 0;
  unsigned short width_ = 0;
  unsigned short height_ = 0;
};
}  // namespace Pol::Plib

#endif
