#ifndef GLOBALS_MEMORYUSAGE_H
#define GLOBALS_MEMORYUSAGE_H

#include <string>


namespace Pol::Core
{
class MemoryUsage
{
public:
  /// The subsystems a report can measure.
  ///
  /// Each one costs a full walk of what it covers, and the whole thing runs on the scripts
  /// thread while the server is stopped, so a caller chasing one kind of growth can leave the
  /// rest out. Objects walks every object in the world; Realms walks every zone of every
  /// realm, which is a fixed six-figure count no matter how quiet the shard is.
  enum Section : unsigned
  {
    SECTION_PROCESS = 0x0001,      ///< resident size as the OS reports it
    SECTION_OBJECTS = 0x0002,      ///< every item, container, mobile and multi
    SECTION_SCRIPTS = 0x0004,      ///< running scripts and the compiled-script cache
    SECTION_GAMESTATE = 0x0008,    ///< accounts, guilds, parties, templates, storage
    SECTION_REALMS = 0x0010,       ///< the zone grid of every realm
    SECTION_NETWORK = 0x0020,      ///< clients, servers, packet hooks
    SECTION_CONFIG = 0x0040,       ///< cached cfg files and datastores
    SECTION_SYSTEMSTATE = 0x0080,  ///< tiles and packages
    SECTION_MULTIS = 0x0100,       ///< the multi definition buffer
    SECTION_SETTINGS = 0x0200,
    SECTION_STATE = 0x0400,
    SECTION_CPROPS = 0x0800,      ///< the CProp profiler, if enabled
    SECTION_ALLOCATORS = 0x1000,  ///< the eScript object pools

    SECTION_ALL = 0x1FFF
  };

  /// Appends one line of totals to log/memoryusage.log.
  ///
  /// This one stays a single growing file on purpose: it is a time series meant to be
  /// graphed, and its header row is written once when the file is created. Columns outside
  /// 'sections' are therefore written empty rather than dropped, so that header keeps
  /// describing the file - and an empty field reads differently from a measured zero.
  static void log( unsigned sections = SECTION_ALL );

  /// Maps an eScript section name ("objects", "realms", ...) to its flag, false if unknown.
  static bool sectionByName( const std::string& name, unsigned* flag );

  /// Every valid section name, comma separated, for error messages.
  static std::string sectionNames();

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
