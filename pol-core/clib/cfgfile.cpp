/** @file
 *
 * @par History
 * - 2005/07/01 Shinigami: added ConfigFile::_modified (stat.st_mtime) to detect cfg file
 * modification
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: elem->type() check will use strlen() now
 */


#include "clib/cfgfile.h"

#include <ctype.h>
#include <exception>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "clib/cfgelem.h"
#include "clib/clib.h"
#include "clib/logfacility.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"


namespace Pol::Clib
{
using namespace std::literals;
namespace
{
/// Blank or a comment, asked of a line that ltrim_view() has already trimmed: deciding it here
/// rather than after splitnamevalue() keeps a line that is going to be dropped from being taken
/// apart first.
bool ignored_line( std::string_view content )
{
  return content.empty() || content[0] == '#' || content.compare( 0, 2, "//"sv ) == 0;
}
}  // namespace

ConfigProperty::ConfigProperty( std::string name, std::string value )
    : name_( std::move( name ) ), value_( std::move( value ) )
{
}

ConfigElemBase::ConfigElemBase() : type_( "" ), rest_( "" ), _source( nullptr ) {}
size_t ConfigElemBase::estimateSize() const
{
  return type_.capacity() + rest_.capacity() + sizeof( _source );
}

size_t ConfigProps::estimateSize() const
{
  // Tombstoned slots past the live count still hold their string buffers, so they count too.
  return Clib::memsize( hashes_ ) + Clib::memsize( by_name_ ) +
         Clib::memsize(
             entries_, []( const auto& entry )
             { return sizeof( entry ) + entry.first.capacity() + entry.second.capacity(); } );
}

size_t ConfigElem::estimateSize() const
{
  return ConfigElemBase::estimateSize() + properties.estimateSize();
}


const char* ConfigElemBase::type() const
{
  return type_.c_str();
}

const char* ConfigElemBase::rest() const
{
  return rest_.c_str();
}

void ConfigElem::set_rest( const char* rest )
{
  rest_ = rest;
}

void ConfigElem::set_type( const char* type )
{
  type_ = type;
}

void ConfigElem::set_source( const ConfigElem& elem )
{
  _source = elem._source;
}
void ConfigElem::set_source( const ConfigSource* source )
{
  _source = source;
}

bool ConfigElemBase::type_is( const char* type ) const
{
  return ( stricmp( type_.c_str(), type ) == 0 );
}

bool ConfigElem::remove_first_prop( std::string* propname, std::string* value )
{
  const auto idx = properties.take_first();
  if ( idx == ConfigProps::npos )
    return false;

  *propname = properties.name_at( idx );
  *value = properties.value_at( idx );
  properties.remove_at( idx );
  return true;
}

bool ConfigElem::has_prop( PropKey propname ) const
{
  return properties.find( propname ) != ConfigProps::npos;
}

bool ConfigElem::remove_prop( PropKey propname, std::string* value )
{
  const auto idx = properties.find( propname );
  if ( idx == ConfigProps::npos )
    return false;
  *value = properties.value_at( idx );
  properties.remove_at( idx );
  return true;
}

bool ConfigElem::read_prop( PropKey propname, std::string* value ) const
{
  const auto idx = properties.find( propname );
  if ( idx == ConfigProps::npos )
    return false;
  *value = properties.value_at( idx );
  return true;
}

void ConfigElem::get_prop( PropKey propname, unsigned int* plong ) const
{
  const auto idx = properties.find( propname );
  if ( idx == ConfigProps::npos )
    throw_error( "SERIAL property not found" );
  *plong = strtoul( properties.value_at( idx ).c_str(), nullptr, 0 );
}

bool ConfigElem::remove_prop( PropKey propname, unsigned int* plong )
{
  const auto idx = properties.find( propname );
  if ( idx == ConfigProps::npos )
    return false;
  *plong = strtoul( properties.value_at( idx ).c_str(), nullptr, 0 );
  properties.remove_at( idx );
  return true;
}

bool ConfigElem::remove_prop( PropKey propname, unsigned short* psval )
{
  std::string temp;
  if ( !remove_prop( propname, &temp ) )
    return false;
  // FIXME isdigit isxdigit - +
  // or, use endptr

  char* endptr = nullptr;
  *psval = (unsigned short)strtoul( temp.c_str(), &endptr, 0 );
  if ( ( endptr != nullptr ) && ( *endptr != '\0' ) && !isspace( *endptr ) )
  {
    throw_error(
        fmt::format( "Poorly formed number in property '{}': {}", propname.name(), temp ) );
  }
  // FIXME check range within unsigned short
  return true;
}

void ConfigElem::throw_error( const std::string& errmsg ) const
{
  if ( _source != nullptr )
    _source->display_error( errmsg, false, this );
  throw std::runtime_error( "Configuration file error" );
}

void ConfigElem::throw_prop_not_found( const std::string& propname ) const
{
  prop_not_found( propname.c_str() );
}

void ConfigElem::warn( const std::string& errmsg ) const
{
  if ( _source != nullptr )
    _source->display_error( errmsg, false, this, false );
}

void ConfigElem::warn_with_line( const std::string& errmsg ) const
{
  if ( _source != nullptr )
    _source->display_error( errmsg, true, this, false );
}

void ConfigElem::prop_not_found( std::string_view propname ) const
{
  throw_error( fmt::format( "Property '{}' was not found", propname ) );
}

unsigned short ConfigElem::remove_ushort( PropKey propname )
{
  unsigned short temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  prop_not_found( propname.name() );  // prop_not_found throws
}

unsigned short ConfigElem::remove_ushort( PropKey propname, unsigned short dflt )
{
  unsigned short temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  return dflt;
}

int ConfigElem::remove_int( PropKey propname )
{
  std::string temp = remove_string( propname );
  return atoi( temp.c_str() );
}

int ConfigElem::remove_int( PropKey propname, int dflt )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return atoi( temp.c_str() );
  return dflt;
}

unsigned ConfigElem::remove_unsigned( PropKey propname )
{
  std::string temp = remove_string( propname );
  return strtoul( temp.c_str(), nullptr, 0 );  // TODO check unsigned range
}

unsigned ConfigElem::remove_unsigned( PropKey propname, int dflt )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return strtoul( temp.c_str(), nullptr, 0 );  // TODO check unsigned range
  return dflt;
}


std::string ConfigElem::remove_string( PropKey propname )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  prop_not_found( propname.name() );  // prop_not_found throws
}

std::string ConfigElem::read_string( PropKey propname ) const
{
  std::string temp;
  if ( read_prop( propname, &temp ) )
    return temp;
  prop_not_found( propname.name() );  // prop_not_found throws
}
std::string ConfigElem::read_string( PropKey propname, const char* dflt ) const
{
  std::string temp;
  if ( read_prop( propname, &temp ) )
    return temp;
  return dflt;
}

std::string ConfigElem::remove_string( PropKey propname, const char* dflt )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  return dflt;
}

bool ConfigElem::remove_bool( PropKey propname )
{
  return remove_ushort( propname ) ? true : false;
}

bool ConfigElem::remove_bool( PropKey propname, bool dflt )
{
  return remove_ushort( propname, dflt ) ? true : false;
}

float ConfigElem::remove_float( PropKey propname, float dflt )
{
  std::string tmp;
  if ( remove_prop( propname, &tmp ) )
    return static_cast<float>( strtod( tmp.c_str(), nullptr ) );
  return dflt;
}
double ConfigElem::remove_double( PropKey propname, double dflt )
{
  std::string tmp;
  if ( remove_prop( propname, &tmp ) )
    return strtod( tmp.c_str(), nullptr );
  return dflt;
}

unsigned int ConfigElem::remove_ulong( PropKey propname )
{
  unsigned int temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  prop_not_found( propname.name() );  // prop_not_found throws
}

unsigned int ConfigElem::remove_ulong( PropKey propname, unsigned int dflt )
{
  unsigned int temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  return dflt;
}

void ConfigElem::clear_prop( PropKey propname )
{
  unsigned int dummy;
  while ( remove_prop( propname, &dummy ) )
    continue;
}

void ConfigElem::add_prop( std::string propname, std::string propval )
{
  properties.emplace( propname, propval );
}

void ConfigElem::add_prop( std::string propname, unsigned short sval )
{
  properties.emplace( propname, std::to_string( sval ) );
}
void ConfigElem::add_prop( std::string propname, short sval )
{
  properties.emplace( propname, std::to_string( sval ) );
}

void ConfigElem::add_prop( std::string propname, unsigned int lval )
{
  properties.emplace( propname, std::to_string( lval ) );
}

ConfigFile::ConfigFile( const char* i_filename, const char* allowed_types_str )
    : _filename( "<n/a>" ), _modified( 0 ), fp( nullptr ), _element_line_start( 0 ), _cur_line( 0 )
{
  init( i_filename, allowed_types_str );
}

ConfigFile::ConfigFile( const std::string& i_filename, const char* allowed_types_str )
    : _filename( "<n/a>" ), _modified( 0 ), fp( nullptr ), _element_line_start( 0 ), _cur_line( 0 )
{
  init( i_filename.c_str(), allowed_types_str );
}

void ConfigFile::init( const char* i_filename, const char* allowed_types_str )
{
  if ( i_filename )
  {
    open( i_filename );
  }

  if ( allowed_types_str != nullptr )
  {
    ISTRINGSTREAM is( allowed_types_str );
    std::string tag;
    while ( is >> tag )
    {
      allowed_types_.insert( tag );
    }
  }
}

const std::string& ConfigFile::filename() const
{
  return _filename;
}
time_t ConfigFile::modified() const
{
  return _modified;
}
unsigned ConfigFile::element_line_start() const
{
  return _element_line_start;
}

void ConfigFile::open( const char* i_filename )
{
  _filename = i_filename;
  _buffer_pos = _buffer_len = 0;

  // Binary, not text: the CRLF translation the CRT does in text mode is a byte-at-a-time pass
  // over every line, and splitnamevalue() already trims the carriage return along with the rest
  // of the trailing whitespace.
  fp = fopen( i_filename, "rb" );
  if ( !fp )
  {
    POLLOG_ERRORLN( "Unable to open configuration file {} {}: {}", _filename, errno,
                    std::strerror( errno ) );
    throw std::runtime_error( std::string( "Unable to open configuration file " ) + _filename );
  }

  struct stat cfgstat;
  stat( i_filename, &cfgstat );
  _modified = cfgstat.st_mtime;
}

ConfigFile::~ConfigFile()
{
  if ( fp )
    fclose( fp );
  fp = nullptr;
}

bool ConfigFile::refill_buffer()
{
  if ( fp == nullptr )
    return false;

  if ( _buffer.empty() )
  {
    // Big enough that the read cost per line disappears, small enough that the hundreds of
    // ConfigFiles a shard opens for its own configuration do not add up to anything.
    _buffer.resize( 64 * 1024 );
  }

  _buffer_pos = 0;
  _buffer_len = fread( _buffer.data(), 1, _buffer.size(), fp );
  return _buffer_len != 0;
}

bool ConfigFile::readline( std::string_view& line )
{
  _split_line.clear();

  for ( ;; )
  {
    if ( _buffer_pos < _buffer_len )
    {
      const char* start = _buffer.data() + _buffer_pos;
      const size_t avail = _buffer_len - _buffer_pos;
      const char* newline = static_cast<const char*>( memchr( start, '\n', avail ) );

      if ( newline != nullptr )
      {
        const size_t len = static_cast<size_t>( newline - start ) + 1;  // newline included
        _buffer_pos += len;
        if ( _split_line.empty() )
        {
          line = std::string_view( start, len );
          return true;
        }
        _split_line.append( start, len );
        line = _split_line;
        return true;
      }

      _split_line.append( start, avail );
      _buffer_pos = _buffer_len = 0;
    }

    if ( !refill_buffer() )
    {
      // A last line with no newline of its own is still a line.
      if ( _split_line.empty() )
        return false;
      line = _split_line;
      return true;
    }
  }
}

// returns true if ended on a }, false if ended on EOF. Throws an error if propname is invalid.
bool ConfigFile::read_properties( ConfigElem& elem )
{
  std::string_view line;
  // Outside the loop so their buffers are reused for every line rather than reallocated.
  std::string propname, propvalue;
  while ( readline( line ) )
  {
    if ( !_cur_line )
      line = remove_bom( line );
    ++_cur_line;

    line = sanitizeUnicodeWithIso( line, &_sanitized );

    const std::string_view content = ltrim_view( line );
    if ( ignored_line( content ) )
      continue;

    splitnamevalue_trimmed( content, propname, propvalue );

    if ( propname == "}"sv )
      return true;

    // Disallow curly braces in the propname otherwise
    if ( propname.find_first_of( "{}"sv ) != std::string::npos )
      elem.throw_error( "Expected a closing brace on a line by itself, got something else" );

    if ( propvalue[0] == '\"' )
    {
      decodequotedstring( propvalue );
    }

    elem.properties.emplace( propname, propvalue );
  }
  return false;
}

bool ConfigFile::_read( ConfigElem& elem )
{
  elem.properties.clear();

  _element_line_start = 0;
  std::string_view line;
  while ( readline( line ) )
  {
    if ( !_cur_line )
      line = remove_bom( line );
    ++_cur_line;

    line = sanitizeUnicodeWithIso( line, &_sanitized );

    const std::string_view content = ltrim_view( line );
    if ( ignored_line( content ) )
      continue;

    splitnamevalue_trimmed( content, elem.type_, elem.rest_ );

    _element_line_start = _cur_line;

    if ( !allowed_types_.empty() )
    {
      if ( allowed_types_.find( elem.type_ ) == allowed_types_.end() )
      {
        auto msg = fmt::format( "Unexpected type '{}'\n\tValid types are:", elem.type_ );
        for ( const auto& allowed : allowed_types_ )
        {
          msg += " " + allowed;
        }
        throw std::runtime_error( msg );
      }
    }

    if ( !readline( line ) )
      throw std::runtime_error( "File ends after element type -- expected a '{'" );
    ++_cur_line;

    if ( line.empty() || line[0] != '{' )
    {
      throw std::runtime_error( "Expected '{' on a blank line after element type" );
    }

    if ( read_properties( elem ) )
      return true;
    throw std::runtime_error( "Expected '}' on a blank line after element properties" );
  }
  return false;
}

void ConfigFile::display_error( const std::string& msg, bool show_curline,
                                const ConfigElemBase* elem, bool error ) const
{
  bool showed_elem_line = false;
  std::string tmp = fmt::format(
      " {} reading configuration file {}:\n"
      "\t{}",
      error ? "Error" : "Warning", _filename, msg );

  if ( elem != nullptr )
  {
    if ( strlen( elem->type() ) > 0 )
    {
      tmp += fmt::format( "\n\tElement: {} {}", elem->type(), elem->rest() );
      if ( _element_line_start )
        tmp += fmt::format( ", found on line {}", _element_line_start );
      showed_elem_line = true;
    }
  }

  if ( show_curline )
    tmp += fmt::format( "\n\tNear line: {}", _cur_line );
  if ( _element_line_start && !showed_elem_line )
    tmp += fmt::format( "\n\tElement started on line: {}", _element_line_start );
  ERROR_PRINTLN( tmp );
}

void ConfigFile::display_and_rethrow_exception()
{
  try
  {
    throw;
  }
  catch ( const char* msg )
  {
    display_error( msg );
  }
  catch ( std::string& str )
  {
    display_error( str );
  }
  catch ( std::exception& ex )
  {
    display_error( ex.what() );
  }
  catch ( ... )
  {
    display_error( "(Generic exception)" );
  }

  throw std::runtime_error( "Configuration file error." );
}

bool ConfigFile::read( ConfigElem& elem )
{
  try
  {
    elem._source = this;
    return _read( elem );
  }
  catch ( ... )
  {
    display_and_rethrow_exception();  // throws and doesn't return
  }
}

void ConfigFile::readraw( ConfigElem& elem )
{
  try
  {
    elem._source = this;
    if ( read_properties( elem ) )
      throw std::runtime_error( "unexpected '}' in file" );
  }
  catch ( ... )
  {
    display_and_rethrow_exception();
  }
}

void StubConfigSource::display_error( const std::string& msg, bool /*show_curline*/,
                                      const ConfigElemBase* /*elem*/, bool error ) const
{
  ERROR_PRINTLN( "{} reading configuration element:\t{}", ( error ? "Error" : "Warning" ), msg );
}
}  // namespace Pol::Clib
