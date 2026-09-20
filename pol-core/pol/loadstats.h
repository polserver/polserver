/** @file
 *
 * @par History
 */


#ifndef LOADSTATS_H
#define LOADSTATS_H

#include <chrono>
#include <string>

#include "clib/rawtypes.h"

namespace Pol::Core
{
/**
 * Where reading the world spent its time, for pol.cfg LogWorldLoadDetails.
 *
 * Nanoseconds because a single element is worth microseconds and there are millions of them, so
 * milliseconds would round every phase to zero. Not atomic: the world is read on one thread.
 */
struct WorldLoadStats
{
  /// Off unless pol.cfg asked for the report; nothing below is touched while it is clear.
  bool enabled = false;

  u64 parse_ns = 0;   ///< reading lines and building the element's property list
  u64 create_ns = 0;  ///< Item::create / new Character, hash_ns included
  u64 hash_ns = 0;    ///< of create_ns: inserting into the object hash
  u64 props_ns = 0;   ///< readProperties, cprops_ns included
  u64 cprops_ns = 0;  ///< of props_ns: interning the custom properties
  u64 place_ns = 0;   ///< putting the object in the world, a container or on a character

  void reset();
  /// One indented block under the file's own line, in the shape LogWorldSaveDetails uses.
  void log( const std::string& what, u64 elements ) const;
};
extern WorldLoadStats worldLoadStats;

/**
 * Adds its own lifetime to one of the counters above, and does nothing whatever when the report
 * is off -- which is why it takes the counter by pointer and drops it.
 */
class LoadPhase
{
public:
  explicit LoadPhase( u64* into )
      : into_( worldLoadStats.enabled ? into : nullptr ),
        start_( into_ != nullptr ? clock::now() : clock::time_point{} )
  {
  }
  ~LoadPhase() { stop(); }

  /// Bank the time so far and stop counting, for a phase that ends before its scope does.
  void stop()
  {
    if ( into_ == nullptr )
      return;
    *into_ += static_cast<u64>(
        std::chrono::duration_cast<std::chrono::nanoseconds>( clock::now() - start_ ).count() );
    into_ = nullptr;
  }
  LoadPhase( const LoadPhase& ) = delete;
  LoadPhase& operator=( const LoadPhase& ) = delete;

private:
  using clock = std::chrono::steady_clock;
  u64* into_;
  clock::time_point start_;
};
}  // namespace Pol::Core

#endif
