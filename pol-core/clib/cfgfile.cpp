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
#include <format/format.h>
#include <utf8/utf8.h>


namespace Pol
{
namespace Clib
{
namespace
{
bool commentline( const std::string& str )
{
#ifdef __GNUC__
  return ( ( str[0] == '#' ) || ( str.substr( 0, 2 ) == "//" ) );
#else
  return ( ( str[0] == '#' ) || ( str.compare( 0, 2, "//" ) == 0 ) );
#endif
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

ConfigElem::ConfigElem() : ConfigElemBase() {}

ConfigElem::~ConfigElem() {}

size_t ConfigElem::estimateSize() const
{
  size_t size = ConfigElemBase::estimateSize();
  for ( const auto& ele : properties )
    size += ele.first.capacity() + ele.second.capacity() + ( sizeof( void* ) * 3 + 1 ) / 2;
  return size;
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
  *propname = ( *itr ).first;
  *value = ( *itr ).second;
  properties.erase( itr );
  return true;
}

bool ConfigElem::has_prop( const char* propname ) const
{
  return properties.count( propname ) != 0;
}
bool VectorConfigElem::has_prop( const char* propname ) const
{
  for ( const auto& prop : properties )
  {
    if ( stricmp( prop->name_.c_str(), propname ) == 0 )
    {
      return true;
    }
  }
  return false;
}

bool ConfigElem::remove_prop( const char* propname, std::string* value )
{
  auto itr = properties.find( propname );
  if ( itr != properties.end() )
  {
    *value = ( *itr ).second;
    properties.erase( itr );
    return true;
  }
  else
  {
    return false;
  }
}
bool VectorConfigElem::remove_prop( const char* propname, std::string* value )
{
  auto itr = properties.begin(), end = properties.end();
  for ( ; itr != end; ++itr )
  {
    ConfigProperty* prop = *itr;
    if ( stricmp( prop->name_.c_str(), propname ) == 0 )
    {
      *value = prop->value_;
      delete prop;
      properties.erase( itr );
      return true;
    }
  }
  return false;
}

bool ConfigElem::read_prop( const char* propname, std::string* value ) const
{
  auto itr = properties.find( propname );
  if ( itr != properties.end() )
  {
    *value = ( *itr ).second;
    return true;
  }
  else
  {
    return false;
  }
}
bool VectorConfigElem::read_prop( const char* propname, std::string* value ) const
{
  for ( const auto& prop : properties )
  {
    if ( stricmp( prop->name_.c_str(), propname ) == 0 )
    {
      *value = prop->value_;
      return true;
    }
  }
  return false;
}

void ConfigElem::get_prop( const char* propname, unsigned int* plong ) const
{
  auto itr = properties.find( propname );
  if ( itr != properties.end() )
  {
    *plong = strtoul( ( *itr ).second.c_str(), nullptr, 0 );
  }
  else
  {
    throw_error( "SERIAL property not found" );
  }
}

bool ConfigElem::remove_prop( const char* propname, unsigned int* plong )
{
  auto itr = properties.find( propname );
  if ( itr != properties.end() )
  {
    *plong = strtoul( ( *itr ).second.c_str(), nullptr, 0 );
    properties.erase( itr );
    return true;
  }
  else
  {
    return false;
  }
}
bool VectorConfigElem::remove_prop( const char* propname, unsigned int* plong )
{
  auto itr = properties.begin(), end = properties.end();
  for ( ; itr != end; ++itr )
  {
    ConfigProperty* prop = *itr;
    if ( stricmp( prop->name_.c_str(), propname ) == 0 )
    {
      *plong = strtoul( prop->value_.c_str(), nullptr, 0 );
      delete prop;
      properties.erase( itr );
      return true;
    }
  }
  return false;
}

bool ConfigElem::remove_prop( const char* propname, unsigned short* psval )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
  {
    // FIXME isdigit isxdigit - +
    // or, use endptr

    char* endptr = nullptr;
    *psval = (unsigned short)strtoul( temp.c_str(), &endptr, 0 );
    if ( ( endptr != nullptr ) && ( *endptr != '\0' ) && !isspace( *endptr ) )
    {
      std::string errmsg;
      errmsg = "Poorly formed number in property '";
      errmsg += propname;
      errmsg += "': " + temp;
      throw_error( errmsg );
    }
    // FIXME check range within unsigned short
    return true;
  }
  return false;
}

VectorConfigElem::~VectorConfigElem() {}

bool VectorConfigElem::remove_prop( const char* propname, unsigned short* psval )
{
  auto itr = properties.begin(), end = properties.end();
  for ( ; itr != end; ++itr )
  {
    ConfigProperty* prop = *itr;
    if ( stricmp( prop->name_.c_str(), propname ) == 0 )
    {
      // FIXME isdigit isxdigit - +
      // or, use endptr

      char* endptr = nullptr;
      *psval = (unsigned short)strtoul( prop->value_.c_str(), &endptr, 0 );
      if ( ( endptr != nullptr ) && ( *endptr != '\0' ) && !isspace( *endptr ) )
      {
        std::string errmsg;
        errmsg = "Poorly formed number in property '";
        errmsg += propname;
        errmsg += "': " + prop->value_;
        throw_error( errmsg );
      }
      // FIXME check range within unsigned short
      delete prop;
      properties.erase( itr );
      return true;
    }
  }
  return false;
}

void ConfigElem::throw_error( const std::string& errmsg ) const
{
  if ( _source != nullptr )
    _source->display_error( errmsg, false, this );
  throw std::runtime_error( "Configuration file error" );
}
void VectorConfigElem::throw_error( const std::string& errmsg ) const
{
  if ( _source != nullptr )
    _source->display_error( errmsg, false, this );
  throw std::runtime_error( "Configuration file error" );
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
  std::string errmsg( "Property '" );
  errmsg += propname;
  errmsg += "' was not found.";
  throw_error( errmsg );
}

unsigned short ConfigElem::remove_ushort( const char* propname )
{
  unsigned short temp;
  if ( remove_prop( propname, &temp ) )
  {
    return temp;
  }
  else
  {
    prop_not_found( propname );  // prop_not_found throws
  }
}

unsigned short ConfigElem::remove_ushort( const char* propname, unsigned short dflt )
{
  unsigned short temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  else
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
  {
    return atoi( temp.c_str() );
  }
  else
  {
    return dflt;
  }
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
  {
    return strtoul( temp.c_str(), nullptr, 0 );  // TODO check unsigned range
  }
  else
  {
    return dflt;
  }
}


std::string ConfigElem::remove_string( const char* propname )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
  {
    return temp;
  }
  else
  {
    prop_not_found( propname );
    return "";  // this is not reached, prop_not_found throws
  }
}

std::string ConfigElem::read_string( const char* propname ) const
{
  std::string temp;
  if ( read_prop( propname, &temp ) )
  {
    return temp;
  }
  else
  {
    prop_not_found( propname );
    return "";  // this is not reached, prop_not_found throws
  }
}
std::string ConfigElem::read_string( const char* propname, const char* dflt ) const
{
  std::string temp;
  if ( read_prop( propname, &temp ) )
    return temp;
  else
    return dflt;
}

std::string ConfigElem::remove_string( const char* propname, const char* dflt )
{
  std::string temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  else
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
  {
    return static_cast<float>( strtod( tmp.c_str(), nullptr ) );
  }
  else
  {
    return dflt;
  }
}
double ConfigElem::remove_double( const char* propname, double dflt )
{
  std::string tmp;
  if ( remove_prop( propname, &tmp ) )
  {
    return strtod( tmp.c_str(), nullptr );
  }
  else
  {
    return dflt;
  }
}

unsigned int ConfigElem::remove_ulong( const char* propname )
{
  unsigned int temp;
  if ( remove_prop( propname, &temp ) )
  {
    return temp;
  }
  else
  {
    prop_not_found( propname );  // prop_not_found throws
  }
}

unsigned int ConfigElem::remove_ulong( const char* propname, unsigned int dflt )
{
  unsigned int temp;
  if ( remove_prop( propname, &temp ) )
    return temp;
  else
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
void VectorConfigElem::add_prop( std::string propname, std::string propval )
{
  ConfigProperty* prop = new ConfigProperty( std::move( propname ), std::move( propval ) );

  properties.push_back( prop );
}

void ConfigElem::add_prop( std::string propname, unsigned short sval )
{
  properties.emplace( std::move( propname ), std::to_string( sval ) );
}
void ConfigElem::add_prop( std::string propname, short sval )
{
  properties.emplace( std::move( propname ), std::to_string( sval ) );
}

void VectorConfigElem::add_prop( std::string propname, unsigned short sval )
{
  auto prop = new ConfigProperty( std::move( propname ), std::to_string( sval ) );

  properties.push_back( prop );
}

void ConfigElem::add_prop( std::string propname, unsigned int lval )
{
  properties.emplace( std::move( propname ), std::to_string( lval ) );
}
void VectorConfigElem::add_prop( std::string propname, unsigned int lval )
{
  auto prop = new ConfigProperty( std::move( propname ), std::to_string( lval ) );

  properties.push_back( prop );
}

bool VectorConfigElem::remove_first_prop( std::string* propname, std::string* value )
{
  if ( properties.empty() )
    return false;

  ConfigProperty* prop = properties.front();
  *propname = prop->name_;
  *value = prop->value_;
  delete prop;
  properties.erase( properties.begin() );
  return true;
}

ConfigFile::ConfigFile( const char* i_filename, const char* allowed_types_str )
    : _filename( "<n/a>" ),
      _modified( 0 ),
#if CFGFILE_USES_IOSTREAMS
      ifs(),
#else
      fp( nullptr ),
#endif
      _element_line_start( 0 ),
      _cur_line( 0 )
{
  init( i_filename, allowed_types_str );
}

ConfigFile::ConfigFile( const std::string& i_filename, const char* allowed_types_str )
    : _filename( "<n/a>" ),
      _modified( 0 ),
#if CFGFILE_USES_IOSTREAMS
      ifs(),
#else
      fp( nullptr ),
#endif
      _element_line_start( 0 ),
      _cur_line( 0 )
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
      allowed_types_.insert( tag.c_str() );
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

#if CFGFILE_USES_IOSTREAMS
  ifs.open( i_filename, ios::in );

  if ( !ifs.is_open() )
  {
    ERROR_PRINT << "Unable to open configuration file " << _filename << "\n";
    throw std::runtime_error( std::string( "Unable to open configuration file " ) + _filename );
  }
#else
  fp = fopen( i_filename, "rt" );
  if ( !fp )
  {
    POLLOG_ERROR << "Unable to open configuration file " << _filename << " " << errno << ": "
                 << std::strerror( errno ) << "\n";
    throw std::runtime_error( std::string( "Unable to open configuration file " ) + _filename );
  }
#endif

  struct stat cfgstat;
  stat( i_filename, &cfgstat );
  _modified = cfgstat.st_mtime;
}

ConfigFile::~ConfigFile()
{
#if !CFGFILE_USES_IOSTREAMS
  if ( fp )
    fclose( fp );
  fp = nullptr;
#endif
}

void ConfigFile::remove_bom( std::string* strbuf )
{
  if ( strbuf->size() >= 3 )
  {
    if ( utf8::starts_with_bom( strbuf->cbegin(), strbuf->cend() ) )
      strbuf->erase( 0, 3 );
  }
}

#if !CFGFILE_USES_IOSTREAMS
char ConfigFile::buffer[1024];
#endif

#if CFGFILE_USES_IOSTREAMS
// returns true if ended on a }, false if ended on EOF.
bool ConfigFile::read_properties( ConfigElem& elem )
{
  std::string strbuf;
  std::string propname, propvalue;
  while ( getline( ifs, strbuf ) )  // get
  {
    if ( !cur_line )
      remove_bom( &strbuf );
    ++cur_line;
    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );
    ISTRINGSTREAM is( strbuf );


    splitnamevalue( strbuf, propname, propvalue );

    if ( propname.empty() ||  // empty line
         commentline( propname ) )
    {
      continue;
    }

    if ( propname == "}" )
      return true;

    if ( propvalue[0] == '\"' )
      convertquotedstring( propvalue );

    ConfigProperty* prop = new ConfigProperty( propname, propvalue );
    elem.properties.push_back( prop );
  }
  return false;
}


bool ConfigFile::_read( ConfigElem& elem )
{
  while ( !elem.properties.empty() )
  {
    delete elem.properties.back();
    elem.properties.pop_back();
  }
  elem.type_ = "";
  elem.rest_ = "";

  element_line_start = 0;

  std::string strbuf;
  while ( getline( ifs, strbuf ) )
  {
    if ( !cur_line )
      remove_bom( &strbuf );
    ++cur_line;

    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );

    string type, rest;
    splitnamevalue( strbuf, type, rest );

    if ( type.empty() ||  // empty line
         commentline( type ) )
    {
      continue;
    }

    element_line_start = cur_line;

    elem.type_ = type;

    if ( !allowed_types_.empty() )
    {
      if ( allowed_types_.find( type.c_str() ) == allowed_types_.end() )
      {
        e_ostringstream os;
        os << "Unexpected type '" << type << "'" << endl;
        os << "\tValid types are:";
        for ( AllowedTypesCont::const_iterator itr = allowed_types_.begin();
              itr != allowed_types_.end(); ++itr )
        {
          os << " " << ( *itr ).c_str();
        }
        throw std::runtime_error( os.e_str() );
      }
    }

    elem.rest_ = rest;

    if ( !getline( ifs, strbuf ) )
      throw std::runtime_error( "File ends after element type -- expected a '{'" );
    ++cur_line;

    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );
    if ( strbuf.empty() || strbuf[0] != '{' )
    {
      throw std::runtime_error( "Expected '{' on a blank line after element type" );
    }

    if ( read_properties( elem ) )
      return true;
    else
      throw std::runtime_error( "Expected '}' on a blank line after element properties" );
  }
  return false;
}
#else

bool ConfigFile::readline( std::string& strbuf )
{
  if ( !fgets( buffer, sizeof buffer, fp ) )
    return false;

  strbuf = "";

  do
  {
    char* nl = strchr( buffer, '\n' );
    if ( nl )
    {
      if ( nl != buffer && *( nl - 1 ) == '\r' )
        --nl;
      *nl = '\0';
      strbuf += buffer;
      return true;
    }
    else
    {
      strbuf += buffer;
    }
  } while ( fgets( buffer, sizeof buffer, fp ) );

  return true;
}

// returns true if ended on a }, false if ended on EOF.
bool ConfigFile::read_properties( ConfigElem& elem )
{
  static std::string strbuf;
  static std::string propname, propvalue;
  while ( readline( strbuf ) )
  {
    if ( !_cur_line )
      remove_bom( &strbuf );
    ++_cur_line;

    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );

    ISTRINGSTREAM is( strbuf );


    splitnamevalue( strbuf, propname, propvalue );

    if ( propname.empty() ||  // empty line
         commentline( propname ) )
    {
      continue;
    }

    if ( propname == "}" )
      return true;

    if ( propvalue[0] == '\"' )
    {
      decodequotedstring( propvalue );
    }

    elem.properties.emplace( propname, propvalue );
  }
  return false;
}
bool ConfigFile::read_properties( VectorConfigElem& elem )
{
  static std::string strbuf;
  static std::string propname, propvalue;
  while ( readline( strbuf ) )
  {
    if ( !_cur_line )
      remove_bom( &strbuf );
    ++_cur_line;

    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );

    ISTRINGSTREAM is( strbuf );


    splitnamevalue( strbuf, propname, propvalue );

    if ( propname.empty() ||  // empty line
         commentline( propname ) )
    {
      continue;
    }

    if ( propname == "}" )
      return true;

    if ( propvalue[0] == '\"' )
    {
      decodequotedstring( propvalue );
    }

    auto prop = new ConfigProperty( propname, propvalue );
    elem.properties.push_back( prop );
  }
  return false;
}


bool ConfigFile::_read( ConfigElem& elem )
{
  elem.properties.clear();

  elem.type_ = "";
  elem.rest_ = "";

  _element_line_start = 0;

  std::string strbuf;
  while ( readline( strbuf ) )
  {
    if ( !_cur_line )
      remove_bom( &strbuf );
    ++_cur_line;

    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );

    std::string type, rest;
    splitnamevalue( strbuf, type, rest );

    if ( type.empty() ||  // empty line
         commentline( type ) )
    {
      continue;
    }

    _element_line_start = _cur_line;

    elem.type_ = type;

    if ( !allowed_types_.empty() )
    {
      if ( allowed_types_.find( type.c_str() ) == allowed_types_.end() )
      {
        OSTRINGSTREAM os;
        os << "Unexpected type '" << type << "'" << std::endl;
        os << "\tValid types are:";
        for ( const auto& allowed : allowed_types_ )
        {
          os << " " << allowed.c_str();
        }
        throw std::runtime_error( OSTRINGSTREAM_STR( os ) );
      }
    }

    elem.rest_ = rest;

    if ( !fgets( buffer, sizeof buffer, fp ) )
      throw std::runtime_error( "File ends after element type -- expected a '{'" );
    strbuf = buffer;
    ++_cur_line;
    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );

    if ( strbuf.empty() || strbuf[0] != '{' )
    {
      throw std::runtime_error( "Expected '{' on a blank line after element type" );
    }

    if ( read_properties( elem ) )
      return true;
    else
      throw std::runtime_error( "Expected '}' on a blank line after element properties" );
  }
  return false;
}

bool ConfigFile::_read( VectorConfigElem& elem )
{
  while ( !elem.properties.empty() )
  {
    delete elem.properties.back();
    elem.properties.pop_back();
  }
  elem.type_ = "";
  elem.rest_ = "";

  _element_line_start = 0;

  std::string strbuf;
  while ( readline( strbuf ) )
  {
    if ( !_cur_line )
      remove_bom( &strbuf );
    ++_cur_line;
    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );

    std::string type, rest;
    splitnamevalue( strbuf, type, rest );

    if ( type.empty() ||  // empty line
         commentline( type ) )
    {
      continue;
    }

    _element_line_start = _cur_line;

    elem.type_ = type;

    if ( !allowed_types_.empty() )
    {
      if ( allowed_types_.find( type.c_str() ) == allowed_types_.end() )
      {
        OSTRINGSTREAM os;
        os << "Unexpected type '" << type << "'" << std::endl;
        os << "\tValid types are:";
        for ( const auto& allowed : allowed_types_ )
        {
          os << " " << allowed.c_str();
        }
        throw std::runtime_error( OSTRINGSTREAM_STR( os ) );
      }
    }

    elem.rest_ = rest;

    if ( !fgets( buffer, sizeof buffer, fp ) )
      throw std::runtime_error( "File ends after element type -- expected a '{'" );
    strbuf = buffer;
    ++_cur_line;
    if ( !isValidUnicode( strbuf ) )
      sanitizeUnicodeWithIso( &strbuf );

    if ( strbuf.empty() || strbuf[0] != '{' )
    {
      throw std::runtime_error( "Expected '{' on a blank line after element type" );
    }

    if ( read_properties( elem ) )
      return true;
    else
      throw std::runtime_error( "Expected '}' on a blank line after element properties" );
  }
  return false;
}


#endif

void ConfigFile::display_error( const std::string& msg, bool show_curline,
                                const ConfigElemBase* elem, bool error ) const
{
  bool showed_elem_line = false;
  fmt::Writer tmp;
  tmp << ( error ? "Error" : "Warning" ) << " reading configuration file " << _filename << ":\n"
      << "\t" << msg << "\n";

  if ( elem != nullptr )
  {
    if ( strlen( elem->type() ) > 0 )
    {
      tmp << "\tElement: " << elem->type() << " " << elem->rest();
      if ( _element_line_start )
        tmp << ", found on line " << _element_line_start;
      tmp << "\n";
      showed_elem_line = true;
    }
  }

  if ( show_curline )
    tmp << "\tNear line: " << _cur_line << "\n";
  if ( _element_line_start && !showed_elem_line )
    tmp << "\tElement started on line: " << _element_line_start << "\n";
  ERROR_PRINT << tmp.str();
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
  ERROR_PRINT << ( error ? "Error" : "Warning" ) << " reading configuration element:"
              << "\t" << msg << "\n";
}
}  // namespace Clib
}  // namespace Pol
