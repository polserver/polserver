#ifndef GLOBALS_MEMORYUSAGE_H
#define GLOBALS_MEMORYUSAGE_H

#include <string>


namespace Pol::Core
{
class MemoryUsage
{
public:
  /// What a report measures. Each section costs a full walk of what it covers, and the server
  /// is stopped throughout, so a caller can leave out the parts it does not need.
  enum Section : unsigned
  {
    SECTION_PROCESS = 0x0001,      // resident size as the OS reports it: the only real number
    SECTION_OBJECTS = 0x0002,      // every item, container, mobile and multi in the object hash
    SECTION_SCRIPTS = 0x0004,      // running scripts and the compiled-script cache
    SECTION_GAMESTATE = 0x0008,    // accounts, guilds, parties, storage, templates, and the
                                   // rest of the global tables
    SECTION_REALMS = 0x0010,       // the zone grid of every realm, a fixed six-figure count
    SECTION_NETWORK = 0x0020,      // clients, servers, packet hooks
    SECTION_CONFIG = 0x0040,       // cached cfg files and datastores
    SECTION_SYSTEMSTATE = 0x0080,  // tiles and packages
    SECTION_MULTIS = 0x0100,       // the multi definition buffer
    SECTION_SETTINGS = 0x0200,     // pol.cfg settings
    SECTION_STATE = 0x0400,        // the state manager, a fixed size rather than a walk
    SECTION_CPROPS = 0x0800,       // the CProp profiler, when ProfileCProps is on
    SECTION_ALLOCATORS = 0x1000,   // the eScript object pools, which overlap SECTION_SCRIPTS
                                   // rather than adding to it

    SECTION_ALL = 0x1FFF
  };

  /// Appends one line of totals to log/memoryusage.log, which accumulates every snapshot.
  static void log( unsigned sections = SECTION_ALL );

  static bool sectionByName( const std::string& name, unsigned* flag );
  static std::string sectionNames();

  /// "log/memory/<prefix>-YYYYMMDD-HHMMSS.log", creating the directory if needed. One file per
  /// report: a single per-script snapshot runs to megabytes on a busy shard.
  static std::string reportPath( const std::string& prefix );

  /// Makes one filename part out of a script name like "pkg:scripts/foo.ecl".
  static std::string sanitizeForFilename( const std::string& name );
};
}  // namespace Pol::Core


#endif
