/** @file
 *
 * @par History
 * - 2005/07/01 Shinigami: added ConfigFile::_modified (stat.st_mtime) to detect cfg file
 * modification
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: elem->type() check will use strlen() now
 */


#include "cfgfile.h"

#include <ctype.h>
#include <exception>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "cfgelem.h"
#include "clib.h"
#include "logfacility.h"
#include "stlutil.h"
#include "strutil.h"


namespace Pol::Clib
{
using namespace std::literals;
namespace
{
bool commentline( const std::string& str )
{
  return ( ( str[0] == '#' ) || ( str.compare( 0, 2, "//"sv ) == 0 ) );
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

size_t ConfigElem::estimateSize() const
{
  return ConfigElemBase::estimateSize() + Clib::memsize( properties );
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
  if ( properties.empty() )
    return false;

  auto itr = properties.begin();
  *propname = itr->first;
  *value = itr->second;
  properties.erase( itr );
  return true;
}

bool ConfigElem::has_prop( const char* propname ) const
{
  return properties.contains( propname );
}

bool ConfigElem::remove_prop( const char* propname, std::string* value )
{
  auto itr = properties.find( propname );
  if ( itr == properties.end() )
    return false;
  *value = itr->second;
  properties.erase( itr );
  return true;
}

bool ConfigElem::read_prop( const char* propname, std::string* value ) const
{
  auto itr = properties.find( propname );
  if ( itr == properties.end() )
    return false;
  *value = itr->second;
  return true;
}

void ConfigElem::get_prop( const char* propname, unsigned int* plong ) const
{
  auto itr = properties.find( propname );
  if ( itr == properties.end() )
    throw_error( "SERIAL property not found" );
  *plong = strtoul( itr->second.c_str(), nullptr, 0 );
}

bool ConfigElem::remove_prop( const char* propname, unsigned int* plong )
{
  auto itr = properties.find( propname );
  if ( itr == properties.end() )
    return false;
  *plong = strtoul( itr->second.c_str(), nullptr, 0 );
  properties.erase( itr );
  return true;
}

bool ConfigElem::remove_prop( const char* propname, unsigned short* psval )
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
    throw_error( fmt::format( "Poorly formed number in property '{}': {}", propname, temp ) );
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

void ConfigElem::prop_not_found( const char* propname ) const
{
  throw_error( fmt::format( "Property '{}' was not found", propname ) );
}

unsigned short ConfigElem::remove_ushort( const char* propname )
{
  unsigned short temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  prop_not_found( propname );  // prop_not_found throws
}

unsigned short ConfigElem::remove_ushort( const char* propname, unsigned short dflt )
{
  unsigned short temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  return dflt;
}

int ConfigElem::remove_int( const char* propname )
{
  std::string temp = remove_string( propname );
  return atoi( temp.c_str() );
}

int ConfigElem::remove_int( const char* propname, int dflt )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return atoi( temp.c_str() );
  return dflt;
}

unsigned ConfigElem::remove_unsigned( const char* propname )
{
  std::string temp = remove_string( propname );
  return strtoul( temp.c_str(), nullptr, 0 );  // TODO check unsigned range
}

unsigned ConfigElem::remove_unsigned( const char* propname, int dflt )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return strtoul( temp.c_str(), nullptr, 0 );  // TODO check unsigned range
  return dflt;
}


std::string ConfigElem::remove_string( const char* propname )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  prop_not_found( propname );  // prop_not_found throws
}

std::string ConfigElem::read_string( const char* propname ) const
{
  std::string temp;
  if ( read_prop( propname, &temp ) )
    return temp;
  prop_not_found( propname );  // prop_not_found throws
}
std::string ConfigElem::read_string( const char* propname, const char* dflt ) const
{
  std::string temp;
  if ( read_prop( propname, &temp ) )
    return temp;
  return dflt;
}

std::string ConfigElem::remove_string( const char* propname, const char* dflt )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  return dflt;
}

bool ConfigElem::remove_bool( const char* propname )
{
  return remove_ushort( propname ) ? true : false;
}

bool ConfigElem::remove_bool( const char* propname, bool dflt )
{
  return remove_ushort( propname, dflt ) ? true : false;
}

float ConfigElem::remove_float( const char* propname, float dflt )
{
  std::string tmp;
  if ( remove_prop( propname, &tmp ) )
    return static_cast<float>( strtod( tmp.c_str(), nullptr ) );
  return dflt;
}
double ConfigElem::remove_double( const char* propname, double dflt )
{
  std::string tmp;
  if ( remove_prop( propname, &tmp ) )
    return strtod( tmp.c_str(), nullptr );
  return dflt;
}

unsigned int ConfigElem::remove_ulong( const char* propname )
{
  unsigned int temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  prop_not_found( propname );  // prop_not_found throws
}

unsigned int ConfigElem::remove_ulong( const char* propname, unsigned int dflt )
{
  unsigned int temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  return dflt;
}

void ConfigElem::clear_prop( const char* propname )
{
  unsigned int dummy;
  while ( remove_prop( propname, &dummy ) )
    continue;
}

void ConfigElem::add_prop( std::string propname, std::string propval )
{
  properties.emplace( std::move( propname ), std::move( propval ) );
}

void ConfigElem::add_prop( std::string propname, unsigned short sval )
{
  properties.emplace( std::move( propname ), std::to_string( sval ) );
}
void ConfigElem::add_prop( std::string propname, short sval )
{
  properties.emplace( std::move( propname ), std::to_string( sval ) );
}

void ConfigElem::add_prop( std::string propname, unsigned int lval )
{
  properties.emplace( std::move( propname ), std::to_string( lval ) );
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

  fp = fopen( i_filename, "rt" );
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

bool ConfigFile::readline( std::string& strbuf )
{
  static thread_local char buffer[1024];
  if ( !fgets( buffer, sizeof buffer, fp ) )
    return false;

  strbuf = "";
  do
  {
    strbuf += buffer;
    if ( strbuf.ends_with( '\n' ) )
      return true;

  } while ( fgets( buffer, sizeof buffer, fp ) );

  return true;
}

// returns true if ended on a }, false if ended on EOF. Throws an error if propname is invalid.
bool ConfigFile::read_properties( ConfigElem& elem )
{
  std::string strbuf;
  while ( readline( strbuf ) )
  {
    if ( !_cur_line )
      remove_bom( &strbuf );
    ++_cur_line;

    sanitizeUnicodeWithIso( &strbuf );

    std::string propname, propvalue;
    splitnamevalue( strbuf, propname, propvalue );

    if ( propname.empty() ||  // empty line
         commentline( propname ) )
    {
      continue;
    }

    if ( propname == "}"sv )
      return true;

    // Disallow curly braces in the propname otherwise
    if ( propname.find_first_of( "{}"sv ) != std::string::npos )
      elem.throw_error( "Expected a closing brace on a line by itself, got something else" );

    if ( propvalue[0] == '\"' )
    {
      decodequotedstring( propvalue );
    }

    elem.properties.emplace( std::move( propname ), std::move( propvalue ) );
  }
  return false;
}

bool ConfigFile::_read( ConfigElem& elem )
{
  elem.properties.clear();

  _element_line_start = 0;
  std::string strbuf;
  while ( readline( strbuf ) )
  {
    if ( !_cur_line )
      remove_bom( &strbuf );
    ++_cur_line;

    sanitizeUnicodeWithIso( &strbuf );

    splitnamevalue( strbuf, elem.type_, elem.rest_ );

    if ( elem.type_.empty() ||  // empty line
         commentline( elem.type_ ) )
    {
      continue;
    }

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

    if ( !readline( strbuf ) )
      throw std::runtime_error( "File ends after element type -- expected a '{'" );
    ++_cur_line;

    if ( strbuf.empty() || strbuf[0] != '{' )
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
