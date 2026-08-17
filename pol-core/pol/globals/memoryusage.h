#ifndef GLOBALS_MEMORYUSAGE_H
#define GLOBALS_MEMORYUSAGE_H

#include <string>


namespace Pol::Core
{
class MemoryUsage
{
public:
  /// Appends one line of totals to log/memoryusage.log.
  ///
  /// This one stays a single growing file on purpose: it is a time series meant to be
  /// graphed, and its header row is written once when the file is created.
  static void log();

  /// "log/memory/<prefix>-YYYYMMDD-HHMMSS.log", creating the directory if needed.
  ///
  /// The per-script reports run to megabytes on a busy shard, so each snapshot gets its own
  /// file instead of being appended to one that grows without bound. Two snapshots taken in
  /// the same second share a file and append, which is harmless.
  static std::string reportPath( const std::string& prefix );

  /// Makes one filename part out of a script name, which carries a path and an extension
  /// ("pkg:scripts/foo.ecl").
  static std::string sanitizeForFilename( const std::string& name );
};
}  // namespace Pol::Core


#endif
