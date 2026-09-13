/** @file
 *
 * @par History
 * - 2005/07/01 Shinigami: added ConfigFile::_modified (stat.st_mtime) to detect cfg file
 * modification
 */


#ifndef CLIB_CFGFILE_H
#define CLIB_CFGFILE_H


#include <stdio.h>

#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <time.h>

#include "clib/maputil.h"


namespace Pol::Clib
{
class ConfigElem;
class ConfigElemBase;

class ConfigSource
{
public:
  virtual ~ConfigSource() = default;
  virtual void display_error( const std::string& msg, bool show_curline = true,
                              const ConfigElemBase* elem = nullptr, bool error = true ) const = 0;
};

class ConfigFile : public ConfigSource
{
public:
  explicit ConfigFile( const char* filename = nullptr, const char* allowed_types = nullptr );
  explicit ConfigFile( const std::string& filename, const char* allowed_types = nullptr );
  ~ConfigFile() override;

  void open( const char* i_filename );

  bool read( ConfigElem& elem );     // true=got one, false=end of file
  void readraw( ConfigElem& elem );  // reads 0 or more properties

  const std::string& filename() const;
  time_t modified() const;
  unsigned element_line_start() const;


protected:
  void init( const char* i_filename, const char* allowed_types_str );
  friend class ConfigElem;
  friend class ConfigSection;
  /**
   * The next line of the file, trailing newline included, or false at end of file.
   *
   * The view points into this object's own read buffer and stays valid only until the next call.
   */
  bool readline( std::string_view& line );
  bool refill_buffer();
  bool read_properties( ConfigElem& elem );
  bool _read( ConfigElem& elem );
  void display_error( const std::string& msg, bool show_curline = true,
                      const ConfigElemBase* elem = nullptr, bool error = true ) const override;
  [[noreturn]] void display_and_rethrow_exception();
  void register_allowed_type( const char* allowed_type );

private:
  std::string _filename;  // saved for exception reporting
  time_t _modified;       // used to detect modification
  FILE* fp;
  int _element_line_start;  // what line in the file did this elem start on?
  int _cur_line;

  std::vector<char> _buffer;  // allocated on the first read, not on open
  size_t _buffer_pos = 0;     // first byte not yet handed out
  size_t _buffer_len = 0;     // bytes of _buffer that hold file content
  std::string _split_line;    // a line that straddled two reads
  std::string _sanitized;     // a line that was not valid utf8, rewritten

  using AllowedTypesCont = std::set<std::string, ci_cmp_pred>;
  AllowedTypesCont allowed_types_;
};

class StubConfigSource : public ConfigSource
{
public:
  void display_error( const std::string& msg, bool show_curline, const ConfigElemBase* elem,
                      bool error ) const override;
};
}  // namespace Pol::Clib

#endif
