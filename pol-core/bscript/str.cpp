/** @file
 *
 * @par History
 * - 2007/12/09 Shinigami: removed ( is.peek() != EOF ) check from String::unpackWithLen()
 *                         will not work with Strings in Arrays, Dicts, etc.
 * - 2008/02/08 Turley:    String::unpackWithLen() will accept zero length Strings
 * - 2009/09/12 MuadDib:   Disabled 4244 in this file due to it being on a string iter. Makes no
 * sense.
 */

#include <cstdlib>
#include <ctype.h>
#include <cwctype>
#include <string>
#include <utf8/utf8.h>

#include "../clib/clib_endian.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "berror.h"
#include "bobject.h"
#include "executor.h"
#include "impstr.h"
#include "objmethods.h"
#include "str.h"

#ifdef __GNUG__
#include <streambuf>
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#include "../clib/Header_Windows.h"
#include <codecvt>
#endif

namespace Pol
{
namespace Bscript
{
String::String( BObjectImp& objimp ) : BObjectImp( OTString ), value_( objimp.getStringRep() ) {}

String::String( const char* s, size_t len, Tainted san ) : BObjectImp( OTString ), value_( s, len )
{
  if ( san == Tainted::YES )
    Clib::sanitizeUnicodeWithIso( &value_ );
}

String::String( const std::string& str, std::string::size_type pos, std::string::size_type n )
    : BObjectImp( OTString ), value_( str, pos, n )
{
}

String::String( const char* str, Tainted san ) : BObjectImp( OTString ), value_( str )
{
  if ( san == Tainted::YES )
    Clib::sanitizeUnicodeWithIso( &value_ );
}

String::String( const std::string& str, Tainted san ) : BObjectImp( OTString ), value_( str )
{
  if ( san == Tainted::YES )
    Clib::sanitizeUnicodeWithIso( &value_ );
}

String::String( const std::string_view& str, Tainted san ) : BObjectImp( OTString ), value_( str )
{
  if ( san == Tainted::YES )
    Clib::sanitizeUnicodeWithIso( &value_ );
}

String* String::StrStr( int begin, int len ) const
{
  auto itr = value_.cbegin();
  --begin;
  size_t startpos = getBytePosition( &itr, begin );
  size_t endpos = getBytePosition( &itr, len );
  if ( startpos != std::string::npos )
    return new String( value_.substr( startpos, endpos - startpos ) );
  return new String( value_ );
}

size_t String::length() const
{
  return utf8::unchecked::distance( value_.begin(), value_.end() );
}

String* String::ETrim( const char* CRSet, int type ) const
{
  std::string tmp = value_;

  if ( type == 1 )  // This is for Leading Only.
  {
    // Find the first character position after excluding leading blank spaces
    size_t startpos = tmp.find_first_not_of( CRSet );
    if ( std::string::npos != startpos )
      tmp = tmp.substr( startpos );
    else
      tmp = "";
    return new String( tmp );
  }
  else if ( type == 2 )  // This is for Trailing Only.
  {
    // Find the first character position from reverse
    size_t endpos = tmp.find_last_not_of( CRSet );
    if ( std::string::npos != endpos )
      tmp = tmp.substr( 0, endpos + 1 );
    else
      tmp = "";
    return new String( tmp );
  }
  else if ( type == 3 )
  {
    // Find the first character position after excluding leading blank spaces
    size_t startpos = tmp.find_first_not_of( CRSet );
    // Find the first character position from reverse af
    size_t endpos = tmp.find_last_not_of( CRSet );

    // if all spaces or empty return on empty string
    if ( ( std::string::npos == startpos ) || ( std::string::npos == endpos ) )
      tmp = "";
    else
      tmp = tmp.substr( startpos, endpos - startpos + 1 );
    return new String( tmp );
  }
  else
    return new String( tmp );
}

void String::EStrReplace( String* str1, String* str2 )
{
  std::string::size_type valpos = 0;
  while ( std::string::npos != ( valpos = value_.find( str1->value_, valpos ) ) )
  {
    value_.replace( valpos, str1->value_.size(), str2->value_ );
    valpos += str2->value_.size();
  }
}

void String::ESubStrReplace( String* replace_with, unsigned int index, unsigned int len )
{
  auto itr = value_.cbegin();
  size_t begin = getBytePosition( &itr, index - 1 );
  size_t end = getBytePosition( &itr, len );
  if ( begin != std::string::npos )
    value_.replace( begin, end - begin, replace_with->value_ );
}

std::string String::pack() const
{
  return "s" + value_;
}

void String::packonto( std::ostream& os ) const
{
  os << "S" << value_.size() << ":" << value_;
}
void String::packonto( std::ostream& os, const std::string& value )
{
  os << "S" << value.size() << ":" << value;
}

BObjectImp* String::unpack( std::istream& is )
{
  std::string tmp;
  getline( is, tmp );

  return new String( tmp );
}

BObjectImp* String::unpackWithLen( std::istream& is )
{
  unsigned len;
  char colon;
  if ( !( is >> len >> colon ) )
  {
    return new BError( "Unable to unpack string length." );
  }
  if ( (int)len < 0 )
  {
    return new BError( "Unable to unpack string length. Invalid length!" );
  }
  if ( colon != ':' )
  {
    return new BError( "Unable to unpack string length. Bad format. Colon not found!" );
  }

  is.unsetf( std::ios::skipws );
  std::string tmp;
  tmp.reserve( len );
  while ( len-- )
  {
    char ch = '\0';
    if ( !( is >> ch ) || ch == '\0' )
    {
      return new BError( "Unable to unpack string length. String length excessive." );
    }
    tmp += ch;
  }

  is.setf( std::ios::skipws );
  return new String( tmp );
}

size_t String::sizeEstimate() const
{
  return sizeof( String ) + value_.capacity();
}

/*
    0-based string find
    find( "str srch", 2, "srch"):
    01^-- start
    */
int String::find( int begin, const char* target ) const
{
  // returns -1 when begin is out of range for string
  auto itr = value_.cbegin();
  size_t pos = getBytePosition( &itr, begin );
  pos = value_.find( target, pos );
  if ( pos == std::string::npos )
    return -1;
  else
  {
    pos = utf8::unchecked::distance( value_.cbegin(), std::next( value_.cbegin(), pos ) );
    return static_cast<int>( pos );
  }
}

unsigned int String::SafeCharAmt() const
{
  unsigned int strlen = static_cast<unsigned int>( length() );
  for ( unsigned int i = 0; i < strlen; ++i )
  {
    unsigned char tmp = value_[i];
    if ( tmp >= 0x80 )  // Ascii range
      return i;
    else if ( isalnum( tmp ) )  // a-z A-Z 0-9
      continue;
    else if ( ispunct( tmp ) )  // !"#$%&'()*+,-./:;<=>?@{|}~
    {
      if ( tmp == '{' || tmp == '}' )
        return i;
      else
        continue;
    }
    else
    {
      return i;
    }
  }
  return strlen;
}

BObjectImp* String::selfPlusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfPlusObj( *this );
}
BObjectImp* String::selfPlusObj( const BObjectImp& objimp ) const
{
  return new String( value_ + objimp.getStringRep() );
}
BObjectImp* String::selfPlusObj( const BLong& objimp ) const
{
  return new String( value_ + objimp.getStringRep() );
}
BObjectImp* String::selfPlusObj( const Double& objimp ) const
{
  return new String( value_ + objimp.getStringRep() );
}
BObjectImp* String::selfPlusObj( const String& objimp ) const
{
  return new String( value_ + objimp.getStringRep() );
}
BObjectImp* String::selfPlusObj( const ObjArray& objimp ) const
{
  return new String( value_ + objimp.getStringRep() );
}
void String::selfPlusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfPlusObj( *this, obj );
}
void String::selfPlusObj( BObjectImp& objimp, BObject& /*obj*/ )
{
  value_ += objimp.getStringRep();
}
void String::selfPlusObj( BLong& objimp, BObject& /*obj*/ )
{
  value_ += objimp.getStringRep();
}
void String::selfPlusObj( Double& objimp, BObject& /*obj*/ )
{
  value_ += objimp.getStringRep();
}
void String::selfPlusObj( String& objimp, BObject& /*obj*/ )
{
  value_ += objimp.getStringRep();
}
void String::selfPlusObj( ObjArray& objimp, BObject& /*obj*/ )
{
  value_ += objimp.getStringRep();
}


void String::remove( const std::string& rm )
{
  auto pos = value_.find( rm );
  if ( pos != std::string::npos )
    value_.erase( pos, rm.size() );
}

BObjectImp* String::selfMinusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfMinusObj( *this );
}
BObjectImp* String::selfMinusObj( const BObjectImp& objimp ) const
{
  String* tmp = (String*)copy();
  tmp->remove( objimp.getStringRep() );
  return tmp;
}
BObjectImp* String::selfMinusObj( const BLong& objimp ) const
{
  String* tmp = (String*)copy();
  tmp->remove( objimp.getStringRep() );
  return tmp;
}
BObjectImp* String::selfMinusObj( const Double& objimp ) const
{
  String* tmp = (String*)copy();
  tmp->remove( objimp.getStringRep() );
  return tmp;
}
BObjectImp* String::selfMinusObj( const String& objimp ) const
{
  String* tmp = (String*)copy();
  tmp->remove( objimp.value_ );
  return tmp;
}
BObjectImp* String::selfMinusObj( const ObjArray& objimp ) const
{
  String* tmp = (String*)copy();
  tmp->remove( objimp.getStringRep() );
  return tmp;
}
void String::selfMinusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfMinusObj( *this, obj );
}
void String::selfMinusObj( BObjectImp& objimp, BObject& /*obj*/ )
{
  remove( objimp.getStringRep() );
}
void String::selfMinusObj( BLong& objimp, BObject& /*obj*/ )
{
  remove( objimp.getStringRep() );
}
void String::selfMinusObj( Double& objimp, BObject& /*obj*/ )
{
  remove( objimp.getStringRep() );
}
void String::selfMinusObj( String& objimp, BObject& /*obj*/ )
{
  remove( objimp.value_ );
}
void String::selfMinusObj( ObjArray& objimp, BObject& /*obj*/ )
{
  remove( objimp.getStringRep() );
}

bool String::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( OTString ) )
    return ( value_ == static_cast<const String&>( objimp ).value_ );

  if ( objimp.isa( OTBoolean ) )
    return isTrue() == static_cast<const BBoolean&>( objimp ).isTrue();
  return base::operator==( objimp );
}

bool String::operator<( const BObjectImp& objimp ) const
{
  if ( objimp.isa( OTString ) )
    return ( value_ < static_cast<const String&>( objimp ).value_ );

  return base::operator<( objimp );
}

namespace
{
template <typename T, typename std::enable_if<sizeof( T ) == sizeof( unsigned int ), int>::type = 0>
std::vector<wchar_t> convertutf8( const std::string& value )
{
  std::vector<wchar_t> codes;
  utf8::unchecked::utf8to32( value.begin(), value.end(), std::back_inserter( codes ) );
  return codes;
}
template <typename T,
          typename std::enable_if<sizeof( T ) == sizeof( unsigned short ), int>::type = 0>
std::vector<wchar_t> convertutf8( const std::string& value )
{
  std::vector<wchar_t> codes;
  utf8::unchecked::utf8to16( value.begin(), value.end(), std::back_inserter( codes ) );
  return codes;
}
}  // namespace

void String::toUpper()
{
  if ( !hasUTF8Characters() )
  {
    Clib::mkupperASCII( value_ );
    return;
  }
#ifndef WINDOWS
  std::vector<wchar_t> codes = convertutf8<wchar_t>( value_ );
  value_.clear();
  for ( const auto& c : codes )
  {
    utf8::unchecked::append( std::towupper( c ), std::back_inserter( value_ ) );
  }
#else
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring str = converter.from_bytes( value_ );

  int len = LCMapStringW( LOCALE_USER_DEFAULT, LCMAP_UPPERCASE | LCMAP_LINGUISTIC_CASING, &str[0],
                          static_cast<int>( str.size() ), 0, 0 );
  if ( !len )
    return;
  else if ( len == str.size() )
  {
    LCMapStringW( LOCALE_USER_DEFAULT, LCMAP_UPPERCASE | LCMAP_LINGUISTIC_CASING, &str[0],
                  static_cast<int>( str.size() ), &str[0], static_cast<int>( str.size() ) );
    value_ = converter.to_bytes( str );
  }
  else
  {
    std::wstring buf;
    buf.reserve( len );
    LCMapStringW( LOCALE_USER_DEFAULT, LCMAP_UPPERCASE | LCMAP_LINGUISTIC_CASING, &str[0],
                  static_cast<int>( str.size() ), &buf[0], static_cast<int>( buf.size() ) );
    value_ = converter.to_bytes( buf );
  }
#endif
}

void String::toLower()
{
  if ( !hasUTF8Characters() )
  {
    Clib::mklowerASCII( value_ );
    return;
  }
#ifndef WINDOWS
  std::vector<wchar_t> codes = convertutf8<wchar_t>( value_ );
  value_.clear();
  for ( const auto& c : codes )
  {
    utf8::unchecked::append( std::towlower( c ), std::back_inserter( value_ ) );
  }
#else
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring str = converter.from_bytes( value_ );

  int len = LCMapStringW( LOCALE_USER_DEFAULT, LCMAP_LOWERCASE | LCMAP_LINGUISTIC_CASING, &str[0],
                          static_cast<int>( str.size() ), 0, 0 );
  if ( !len )
    return;
  else if ( len == str.size() )
  {
    LCMapStringW( LOCALE_USER_DEFAULT, LCMAP_LOWERCASE | LCMAP_LINGUISTIC_CASING, &str[0],
                  static_cast<int>( str.size() ), &str[0], static_cast<int>( str.size() ) );
    value_ = converter.to_bytes( str );
  }
  else
  {
    std::wstring buf;
    buf.reserve( len );
    LCMapStringW( LOCALE_USER_DEFAULT, LCMAP_LOWERCASE | LCMAP_LINGUISTIC_CASING, &str[0],
                  static_cast<int>( str.size() ), &buf[0], static_cast<int>( buf.size() ) );
    value_ = converter.to_bytes( buf );
  }
#endif
}

size_t String::getBytePosition( std::string::const_iterator* itr, size_t codeindex ) const
{
  auto itr_end = value_.cend();
  for ( size_t i = 0; i < codeindex && *itr != itr_end; ++i )
    utf8::unchecked::next( *itr );

  if ( *itr != itr_end )
  {
    return std::distance( value_.cbegin(), *itr );
  }
  return std::string::npos;
}

BObjectImp* String::array_assign( BObjectImp* idx, BObjectImp* target, bool /*copy*/ )
{
  std::string::size_type pos, len;

  // first, determine position and length.
  if ( idx->isa( OTString ) )
  {
    String& rtstr = (String&)*idx;
    pos = value_.find( rtstr.value_ );
    len = rtstr.value_.size();
  }
  else if ( idx->isa( OTLong ) )
  {
    BLong& lng = (BLong&)*idx;
    len = 1;
    pos = lng.value() - 1;
    auto itr = value_.cbegin();
    pos = getBytePosition( &itr, pos );
    if ( pos != std::string::npos )
    {
      utf8::unchecked::next( itr );
      len = std::distance( value_.cbegin(), itr ) - pos;
    }
    else
      pos = std::string::npos;
  }
  else if ( idx->isa( OTDouble ) )
  {
    Double& dbl = (Double&)*idx;
    pos = static_cast<std::string::size_type>( dbl.value() ) - 1;
    len = 1;
    auto itr = value_.cbegin();
    pos = getBytePosition( &itr, pos );
    if ( pos != std::string::npos )
    {
      utf8::unchecked::next( itr );
      len = std::distance( value_.cbegin(), itr ) - pos;
    }
    else
      pos = std::string::npos;
  }
  else
  {
    return UninitObject::create();
  }

  if ( pos != std::string::npos )
  {
    if ( target->isa( OTString ) )
    {
      String* target_str = (String*)target;
      value_.replace( pos, len, target_str->value_ );
    }
    return this;
  }
  else
  {
    return UninitObject::create();
  }
}

BObjectRef String::OperMultiSubscriptAssign( std::stack<BObjectRef>& indices, BObjectImp* target )
{
  BObjectRef start_ref = indices.top();
  indices.pop();
  BObjectRef length_ref = indices.top();
  indices.pop();

  BObject& length_obj = *length_ref;
  BObject& start_obj = *start_ref;

  BObjectImp& length = length_obj.impref();
  BObjectImp& start = start_obj.impref();

  // first deal with the start position.
  size_t index;
  if ( start.isa( OTLong ) )
  {
    BLong& lng = (BLong&)start;
    index = (size_t)lng.value();
    if ( index == 0 || index > value_.size() )
      return BObjectRef( new BError( "Subscript out of range" ) );
    --index;
    auto itr = value_.cbegin();
    index = getBytePosition( &itr, index );
    if ( index == std::string::npos )
      return BObjectRef( new BError( "Subscript out of range" ) );
  }
  else if ( start.isa( OTString ) )
  {
    String& rtstr = (String&)start;
    std::string::size_type pos = value_.find( rtstr.value_ );
    if ( pos != std::string::npos )
      index = static_cast<size_t>( pos );
    else
      return BObjectRef( new UninitObject );
  }
  else
  {
    return BObjectRef( copy() );
  }

  // now deal with the length.
  size_t len;
  if ( length.isa( OTLong ) )
  {
    BLong& lng = (BLong&)length;

    len = (size_t)lng.value();
  }
  else if ( length.isa( OTDouble ) )
  {
    Double& dbl = (Double&)length;

    len = (size_t)dbl.value();
  }
  else
  {
    return BObjectRef( copy() );
  }
  auto itr = value_.cbegin();
  std::advance( itr, index );
  size_t index_len = getBytePosition( &itr, len );

  if ( index_len != std::string::npos )
    len = index_len - index;
  else
    len = index_len;

  if ( target->isa( OTString ) )
  {
    String* target_str = (String*)target;
    value_.replace( index, len, target_str->value_ );
  }
  else
  {
    return BObjectRef( UninitObject::create() );
  }

  return BObjectRef( this );
}


BObjectRef String::OperMultiSubscript( std::stack<BObjectRef>& indices )
{
  BObjectRef start_ref = indices.top();
  indices.pop();
  BObjectRef length_ref = indices.top();
  indices.pop();

  BObject& length_obj = *length_ref;
  BObject& start_obj = *start_ref;

  BObjectImp& length = length_obj.impref();
  BObjectImp& start = start_obj.impref();

  // first deal with the start position.
  size_t index;
  if ( start.isa( OTLong ) )
  {
    BLong& lng = (BLong&)start;
    index = (size_t)lng.value();
    if ( index == 0 || index > value_.size() )
      return BObjectRef( new BError( "Subscript out of range" ) );
    --index;
    auto itr = value_.cbegin();
    index = getBytePosition( &itr, index );
    if ( index == std::string::npos )
      return BObjectRef( new BError( "Subscript out of range" ) );
  }
  else if ( start.isa( OTString ) )
  {
    String& rtstr = (String&)start;
    std::string::size_type pos = value_.find( rtstr.value_ );
    if ( pos != std::string::npos )
      index = static_cast<unsigned int>( pos );
    else
      return BObjectRef( new UninitObject );
  }
  else
  {
    return BObjectRef( copy() );
  }

  // now deal with the length.
  size_t len;
  if ( length.isa( OTLong ) )
  {
    BLong& lng = (BLong&)length;

    len = (size_t)lng.value();
  }
  else if ( length.isa( OTDouble ) )
  {
    Double& dbl = (Double&)length;

    len = (size_t)dbl.value();
  }
  else
  {
    return BObjectRef( copy() );
  }
  auto itr = value_.cbegin();
  std::advance( itr, index );
  size_t index_len = getBytePosition( &itr, len );

  if ( index_len != std::string::npos )
    len = index_len - index;
  else
    len = index_len;
  return BObjectRef( new String( value_, index, len ) );
}

BObjectRef String::OperSubscript( const BObject& rightobj )
{
  const BObjectImp& right = rightobj.impref();
  if ( right.isa( OTLong ) )
  {
    BLong& lng = (BLong&)right;

    if ( lng.value() < 0 )
      return BObjectRef( new BError( "Subscript out of range" ) );

    size_t index = (size_t)lng.value();

    if ( index == 0 || index > value_.size() )
      return BObjectRef( new BError( "Subscript out of range" ) );

    --index;
    auto itr = value_.cbegin();
    index = getBytePosition( &itr, index );
    if ( index != std::string::npos )
    {
      utf8::unchecked::next( itr );
      size_t len = std::distance( value_.cbegin(), itr ) - index;
      return BObjectRef( new BObject( new String( value_.c_str() + index, len ) ) );
    }
    return BObjectRef( new BError( "Subscript out of range" ) );
  }
  else if ( right.isa( OTDouble ) )
  {
    Double& dbl = (Double&)right;

    if ( dbl.value() < 0 )
      return BObjectRef( new BError( "Subscript out of range" ) );
    size_t index = (size_t)dbl.value();

    if ( index == 0 || index > value_.size() )
      return BObjectRef( new BError( "Subscript out of range" ) );

    --index;
    auto itr = value_.cbegin();
    index = getBytePosition( &itr, index );
    if ( index != std::string::npos )
    {
      utf8::unchecked::next( itr );
      size_t len = std::distance( value_.cbegin(), itr ) - index;
      return BObjectRef( new BObject( new String( value_.c_str() + index, len ) ) );
    }
    return BObjectRef( new BError( "Subscript out of range" ) );
  }
  else if ( right.isa( OTString ) )
  {
    String& rtstr = (String&)right;
    auto pos = value_.find( rtstr.value_ );
    if ( pos != std::string::npos )
    {
      auto itr = value_.cbegin();
      std::advance( itr, pos );
      utf8::unchecked::next( itr );
      size_t len = std::distance( value_.cbegin(), itr ) - pos;
      return BObjectRef( new BObject( new String( value_, pos, len ) ) );
    }
    else
      return BObjectRef( new UninitObject );
  }
  else
  {
    return BObjectRef( new UninitObject );
  }
}

// -- format related stuff --
bool s_parse_int( int& i, std::string const& s )
{
  if ( s.empty() )
    return false;

  char* end;
  i = strtol( s.c_str(), &end, 10 );

  if ( !*end )
  {
    return true;
  }
  else
  {
    return false;
  }
}

void int_to_binstr( int& value, std::stringstream& s )
{
  int i;
  for ( i = 31; i > 0; i-- )
  {
    if ( value & ( 1 << i ) )
      break;
  }
  for ( ; i >= 0; i-- )
  {
    if ( value & ( 1 << i ) )
      s << "1";
    else
      s << "0";
  }
}

// suplementory function to format
bool try_to_format( std::stringstream& to_stream, BObjectImp* what, std::string& frmt )
{
  if ( frmt.empty() )
  {
    to_stream << what->getStringRep();
    return false;
  }

  if ( frmt.find( 'b' ) != std::string::npos )
  {
    if ( !what->isa( BObjectImp::OTLong ) )
    {
      to_stream << "<needs Int>";
      return false;
    }
    BLong* plong = static_cast<BLong*>( what );
    int n = plong->value();
    if ( frmt.find( '#' ) != std::string::npos )
      to_stream << ( ( n < 0 ) ? "-" : "" ) << "0b";
    int_to_binstr( n, to_stream );
  }
  else if ( frmt.find( 'x' ) != std::string::npos )
  {
    if ( !what->isa( BObjectImp::OTLong ) )
    {
      to_stream << "<needs Int>";
      return false;
    }
    BLong* plong = static_cast<BLong*>( what );
    int n = plong->value();
    if ( frmt.find( '#' ) != std::string::npos )
      to_stream << "0x";
    to_stream << std::hex << n << std::dec;
  }
  else if ( frmt.find( 'o' ) != std::string::npos )
  {
    if ( !what->isa( BObjectImp::OTLong ) )
    {
      to_stream << "<needs Int>";
      return false;
    }
    BLong* plong = static_cast<BLong*>( what );
    int n = plong->value();
    if ( frmt.find( '#' ) != std::string::npos )
      to_stream << "0o";
    to_stream << std::oct << n << std::dec;
  }
  else if ( frmt.find( 'd' ) != std::string::npos )
  {
    int n;
    if ( what->isa( BObjectImp::OTLong ) )
    {
      BLong* plong = static_cast<BLong*>( what );
      n = plong->value();
    }
    else if ( what->isa( BObjectImp::OTDouble ) )
    {
      Double* pdbl = static_cast<Double*>( what );
      n = (int)pdbl->value();
    }
    else
    {
      to_stream << "<needs Int, Double>";
      return false;
    }
    to_stream << std::dec << n;
  }
  else
  {
    to_stream << "<bad format: " << frmt << ">";
    return false;
  }
  return true;
}
// --

std::string try_to_format(BObjectImp* what, std::string& frmt) {
  std::stringstream result;
  try_to_format( result, what, frmt );
  return result.str();
}

BObjectImp* String::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  else
    return nullptr;
}
BObjectImp* String::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  switch ( id )
  {
  case MTH_LENGTH:
    if ( ex.numParams() == 0 )
      return new BLong( static_cast<int>( length() ) );
    else
      return new BError( "string.length() doesn't take parameters." );
    break;
  case MTH_FIND:
  {
    if ( ex.numParams() > 2 )
      return new BError( "string.find(Search, [Start]) takes only two parameters" );
    if ( ex.numParams() < 1 )
      return new BError( "string.find(Search, [Start]) takes at least one parameter" );
    const char* s = ex.paramAsString( 0 );
    int d = 0;
    if ( ex.numParams() == 2 )
      d = ex.paramAsLong( 1 );
    int posn = find( d ? ( d - 1 ) : 0, s ) + 1;
    return new BLong( posn );
  }
  case MTH_UPPER:
  {
    if ( ex.numParams() == 0 )
    {
      toUpper();
      return this;
    }
    else
      return new BError( "string.upper() doesn't take parameters." );
  }

  case MTH_LOWER:
  {
    if ( ex.numParams() == 0 )
    {
      toLower();
      return this;
    }
    else
      return new BError( "string.lower() doesn't take parameters." );
  }
  case MTH_FORMAT:
  {
    if ( ex.numParams() > 0 )
    {
      // string s = this->getStringRep(); // string itself
      std::stringstream result;

      size_t tag_start_pos;  // the position of tag's start "{"
      size_t tag_stop_pos;   // the position of tag's end "}"
      size_t tag_dot_pos;

      int tag_param_idx;

      size_t str_pos = 0;               // current string position
      unsigned int next_param_idx = 0;  // next index of .format() parameter

      char w_spaces[] = "\t ";

      // Tells whether last found tag was an integer
      bool last_tag_was_int = true;

      while ( ( tag_start_pos = value_.find( '{', str_pos ) ) != std::string::npos )
      {
        if ( ( tag_stop_pos = value_.find( '}', tag_start_pos ) ) != std::string::npos )
        {
          result << value_.substr( str_pos, tag_start_pos - str_pos );
          str_pos = tag_stop_pos + 1;

          std::string tag_body =
              value_.substr( tag_start_pos + 1, ( tag_stop_pos - tag_start_pos ) - 1 );

          tag_start_pos = tag_body.find_first_not_of( w_spaces );
          tag_stop_pos = tag_body.find_last_not_of( w_spaces );

          // cout << "' tag_body1: '" << tag_body << "'";

          // trim the tag of whitespaces (slightly faster code ~25%)
          if ( tag_start_pos != std::string::npos && tag_stop_pos != std::string::npos )
            tag_body = tag_body.substr( tag_start_pos, ( tag_stop_pos - tag_start_pos ) + 1 );
          else if ( tag_start_pos != std::string::npos )
            tag_body = tag_body.substr( tag_start_pos );
          else if ( tag_stop_pos != std::string::npos )
            tag_body = tag_body.substr( 0, tag_stop_pos + 1 );

          // cout << "' tag_body2: '" << tag_body << "'";

          std::string frmt;
          size_t formatter_pos = tag_body.find( ':' );

          if ( formatter_pos != std::string::npos )
          {
            frmt = tag_body.substr( formatter_pos + 1, std::string::npos );  //
            tag_body = tag_body.substr( 0, formatter_pos );  // remove property from the tag
          }

          std::string prop_name;
          // parsing {1.this_part}
          tag_dot_pos = tag_body.find( '.', 0 );

          // '.' is found within the tag, there is a property name
          if ( tag_dot_pos != std::string::npos )
          {
            last_tag_was_int = true;
            prop_name = tag_body.substr( tag_dot_pos + 1, std::string::npos );  //
            tag_body = tag_body.substr( 0, tag_dot_pos );  // remove property from the tag

            // if s_tag_body is numeric then use it as an index
            if ( s_parse_int( tag_param_idx, tag_body ) )
            {
              tag_param_idx -= 1;  // sinse POL indexes are 1-based
            }
            else
            {
              result << "<idx required before: '" << prop_name << "'>";
              continue;
            }
          }
          else
          {
            if ( tag_body == "" )
            {
              // empty body just takes next integer idx
              last_tag_was_int = true;
              tag_param_idx = next_param_idx++;
            }
            else if ( s_parse_int( tag_param_idx, tag_body ) )
            {
              last_tag_was_int = true;
              tag_param_idx -= 1;  // sinse POL indexes are 1-based
            }
            else
            {
              // string body takes next idx in line if this is
              // the first string body occurrence,
              // will reuse last idx if this is 2nd or more in a row
              last_tag_was_int = false;
              prop_name = tag_body;
              tag_param_idx = last_tag_was_int ? next_param_idx++ : next_param_idx;
            }
          }

          // -- end of property parsing

          // cout << "prop_name: '" << prop_name << "' tag_body: '" << tag_body << "'";
          if ( tag_param_idx < 0 || (int)ex.numParams() <= tag_param_idx )
          {
            result << "<invalid index: #" << ( tag_param_idx + 1 ) << ">";
            continue;
          }

          BObjectImp* imp = ex.getParamImp( tag_param_idx );

          if ( !prop_name.empty() )
          {  // accesing object
            BObjectRef obj_member = imp->get_member( prop_name.c_str() );
            BObjectImp* member_imp = obj_member->impptr();
            try_to_format( result, member_imp, frmt );
          }
          else
          {
            try_to_format( result, imp, frmt );
          }
        }
        else
        {
          break;
        }
      }

      if ( str_pos < value_.length() )
      {
        result << value_.substr( str_pos, std::string::npos );
      }

      return new String( result.str() );
    }
    else
    {
      return new BError( "string.format() requires a parameter." );
    }
  }
  case MTH_JOIN:
  {
    BObject* cont;
    if ( ex.numParams() == 1 && ( cont = ex.getParamObj( 0 ) ) != nullptr )
    {
      if ( !( cont->isa( OTArray ) ) )
        return new BError( "string.join expects an array" );
      ObjArray* container = static_cast<ObjArray*>( cont->impptr() );
      // no empty check here on purpose
      OSTRINGSTREAM joined;
      bool first = true;
      for ( const BObjectRef& ref : container->ref_arr )
      {
        if ( ref.get() )
        {
          BObject* bo = ref.get();

          if ( bo == nullptr )
            continue;
          if ( !first )
            joined << value_;
          else
            first = false;
          joined << bo->impptr()->getStringRep();
        }
      }
      return new String( OSTRINGSTREAM_STR( joined ) );
    }
    else
      return new BError( "string.join(array) requires a parameter." );
  }
  default:
    return nullptr;
  }
}

bool String::hasUTF8Characters() const
{
  return hasUTF8Characters( value_ );
}

bool String::hasUTF8Characters( const std::string& str )
{
  for ( const auto& c : str )
  {
    if ( c & 0x80 )
      return true;
  }
  return false;
}

std::vector<unsigned short> String::toUTF16() const
{
  std::vector<unsigned short> u16;
  utf8::unchecked::utf8to16( value_.begin(), value_.end(), std::back_inserter( u16 ) );
  return u16;
}

std::string String::fromUTF16( unsigned short code )
{
  std::string s;
  std::vector<unsigned short> utf16( 1, code );
  utf8::unchecked::utf16to8( utf16.begin(), utf16.end(), std::back_inserter( s ) );
  Clib::sanitizeUnicode( &s );
  return s;
}

std::string String::fromUTF16( const unsigned short* code, size_t len, bool big_endian )
{
  std::string s;
  size_t short_len = 0;
  // convert until the first null terminator
  while ( code[short_len] != 0 && short_len < len )
    ++short_len;

  // minimum incomplete iterator implementation, just for the internal usage with utf8lib to
  // directly decode flipped bytes
  struct BigEndianIterator
  {
    const u16* ptr;
    BigEndianIterator( const u16* begin ) : ptr( begin ){};
    BigEndianIterator& operator++()
    {
      ++ptr;
      return *this;
    };
    BigEndianIterator operator++( int )
    {
      BigEndianIterator itr( ptr );
      ++ptr;
      return itr;
    };
    u16 operator*() { return cfBEu16( *ptr ); };
    bool operator!=( const BigEndianIterator& o ) { return ptr != o.ptr; };
  };
  if ( big_endian )
    utf8::unchecked::utf16to8( BigEndianIterator( code ), BigEndianIterator( code + short_len ),
                               std::back_inserter( s ) );
  else
    utf8::unchecked::utf16to8( code, code + short_len, std::back_inserter( s ) );
  Clib::sanitizeUnicode( &s );
  return s;
}

std::string String::fromUTF8( const char* code, size_t len )
{
  size_t short_len = 0;
  // convert until the first null terminator
  while ( code[short_len] != 0 && short_len < len )
    ++short_len;
  std::string s( code, short_len );
  Clib::sanitizeUnicode( &s );
  return s;
}

std::vector<unsigned short> String::toUTF16( const std::string& text )
{
  std::vector<unsigned short> utf16;
  if ( text.empty() )
    return utf16;
  utf8::unchecked::utf8to16( text.begin(), text.end(), std::back_inserter( utf16 ) );
  return utf16;
}

bool String::compare( const String& str ) const
{
  return value_.compare( str.value_ ) == 0;
}

bool String::compare( size_t pos1, size_t len1, const String& str ) const
{
  auto itr1 = value_.cbegin();
  pos1 = getBytePosition( &itr1, pos1 );
  len1 = getBytePosition( &itr1, len1 ) - pos1;
  return value_.compare( pos1, len1, str.value_ ) == 0;
}

bool String::compare( size_t pos1, size_t len1, const String& str, size_t pos2, size_t len2 ) const
{
  auto itr1 = value_.cbegin();
  pos1 = getBytePosition( &itr1, pos1 );
  len1 = getBytePosition( &itr1, len1 ) - pos1;
  auto itr2 = str.value_.cbegin();
  pos2 = str.getBytePosition( &itr2, pos2 );
  len2 = str.getBytePosition( &itr2, len2 ) - pos2;
  return value_.compare( pos1, len1, str.value_, pos2, len2 ) == 0;
}

String* String::fromUCArray( ObjArray* array, bool break_first_null )
{
  std::string res;
  std::vector<u16> utf16;
  for ( const auto& c : array->ref_arr )
  {
    if ( !c )
      continue;
    BObjectImp* imp = c.get()->impptr();
    if ( imp && imp->isa( BObjectImp::OTLong ) )
    {
      BLong* blong = static_cast<BLong*>( imp );
      if ( blong->value() == 0 && break_first_null )
        break;
      utf16.push_back( blong->value() & 0xFFFF );
    }
  }
  if ( !utf16.empty() )
    utf8::unchecked::utf16to8( utf16.begin(), utf16.end(), std::back_inserter( res ) );

  Clib::sanitizeUnicode( &res );
  return new String( res );
}
}  // namespace Bscript
}  // namespace Pol
