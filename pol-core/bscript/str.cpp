/** @file
 *
 * @par History
 * - 2007/12/09 Shinigami: removed ( is.peek() != EOF ) check from String::unpackWithLen()
 *                         will not work with Strings in Arrays, Dicts, etc.
 * - 2008/02/08 Turley:    String::unpackWithLen() will accept zero length Strings
 * - 2009/09/12 MuadDib:   Disabled 4244 in this file due to it being on a string iter. Makes no
 *                         sense.
*  - 2018/04/04 Bodom:     adding unicode support
 */

#include <cstdlib>
#include <ctype.h>
#include <string>
#include <boost/lexical_cast.hpp>

#include "../clib/stlutil.h"
#include "berror.h"
#include "bobject.h"
#include "executor.h"
#include "impstr.h"
#include "objmethods.h"

#ifdef __GNUG__
#include <streambuf>
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#endif

namespace Pol
{
namespace Bscript
{
String::String( BObjectImp& objimp ) : BObjectImp( OTString ), value_( objimp.getStringRep() ) {}

//String::String( const char* s, int len ) : BObjectImp( OTString ), value_( s, len ) {}

/*
String* String::StrStr( int begin, int len )
{
  return new String( value_.substr( begin - 1, len ) );
}

/**
  * Searches this string for all occurrences of src and replaces them with rep
  *
  * @param src The substring to search for
  * @param rep The string to replace with
  */
void String::replace( const String& src, const String& rep )
{
  UnicodeString::size_type valpos = 0;
  while ( UnicodeString::npos != (valpos = value_.find(src.value_, valpos)))
  {
    value_.replace( valpos, src.lengthc(), rep.value_ );
    valpos += rep.lengthc();
  }
}

/**
  * Replaces a portion of this string with a given one
  *
  * @param replace_with The string to replace with
  * @param index The starting position to replace from
  * @param len The amount of characters to be replaced
  */
void String::replace( const String& replace_with, const unsigned int index, const unsigned int len )
{
  value_.replace( index - 1, len, replace_with.value_ );
}

/**
 * Returns a new string containing a part of this string
 *
 * @param begin 0-based position of the first char
 * @param len number of characters to include in the substring
 */
String* String::substr( const int begin, const int len ) const
{
  return new String( value_.substr( begin, len ) );
}

void String::packonto( std::ostream& os ) const
{
  const std::string encoded = value_.utf8();
  os << "S" << encoded.size() << ":" << encoded;
}
/*
void String::packonto( std::ostream& os, const std::string& value )
{
  os << "S" << value.size() << ":" << value;
}
*/
/*
BObjectImp* String::unpack( const char* pstr )
{
  return new String( pstr );
}

BObjectImp* String::unpack( std::istream& is )
{
  std::string tmp;
  getline( is, tmp );

  return new String( tmp );
}
*/
/*
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
*/

size_t String::sizeEstimate() const
{
  return sizeof( String ) + value_.sizeEstimate();
}

/**
  * 0-based string find
  * find( "str srch", 2, "srch"):
  * 01^-- start
  *
  * @param begin: position to start from. If this is greater than the string length,
  *               the function never finds matches.
  * @param target: the string to search for
  * @return 0-based position, -1 if not found
  **/
int String::find( int begin, const UnicodeString& target )
{
  UnicodeString::size_type pos;
  pos = value_.find( target, begin );
  if ( pos == UnicodeString::npos )
    return -1;
  else
    return static_cast<int>( pos );
}

/**
 * Returns the amount of alpha-numeric characters in string
 */
unsigned int String::alnumlen() const
{
  unsigned int c = 0;
  for ( auto it : value_ )
    if ( iswalnum(it->asUtf32()) )
      c++;
  return c;
}

/**
 * Returns how many safe to print characters can be read
 * from string until a non-safe one is found
 */
unsigned int String::SafeCharAmt() const
{
  for ( auto it : value_ )
  {
    char32_t tmp = it->asUtf32();

    if ( iswalnum( tmp ) )  // a-z A-Z 0-9
      continue;

    if ( iswpunct( tmp ) )  // !"#$%&'()*+,-./:;<=>?@{|}~
    {
      if ( tmp == '{' || tmp == '}' )
        return it.pos();
      else
        continue;
    }
    else
    {
      return it.pos();
    }
  }
  return value_.lengthc();
}

/**
 * In-place reverse this string
 *
 * @see UnicodeString::reverse()
 */
void String::reverse()
{
  value_.reverse();
}

/*
 * Copies the c-style string into this one and deletes the pointer
 */
/*
void String::set( char* newstr )
{
  value_ = newstr;
  delete newstr;
}
*/


// ------------------- POL Operator implementations --------------------------


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

/**
 * @see UnicodeString::remove
 */
void String::remove( const UnicodeString& rm )
{
  value_.remove(rm);
}

BObjectImp* String::selfMinusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfMinusObj( *this );
}
BObjectImp* String::selfMinusObj( const BObjectImp& objimp ) const
{
  String* tmp = (String*)copy();
  tmp->remove( objimp.getStringRep().data() );
  return tmp;
}
BObjectImp* String::selfMinusObj( const BLong& objimp ) const
{
  String* tmp = (String*)copy();
  tmp->remove( objimp.getStringRep().data() );
  return tmp;
}
BObjectImp* String::selfMinusObj( const Double& objimp ) const
{
  String* tmp = (String*)copy();
  tmp->remove( objimp.getStringRep().data() );
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
  remove( objimp.getStringRep().data() );
}
void String::selfMinusObj( BLong& objimp, BObject& /*obj*/ )
{
  remove( objimp.getStringRep().data() );
}
void String::selfMinusObj( Double& objimp, BObject& /*obj*/ )
{
  remove( objimp.getStringRep().data() );
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

/**
 * Since non-Strings show up here as not equal, we make them less than.
 *
 * @todo TODO: Change this behavior? It doesn't make much sense - 12-27-2015 Bodom
 */
bool String::operator<( const BObjectImp& objimp ) const
{
  if ( objimp.isa( OTString ) )
    return ( value_ < static_cast<const String&>( objimp ).value_ );

  return base::operator<( objimp );
}

/**
 * Returns a pointer to a substring of this string
 */
String* String::midstring( size_t begin, size_t len ) const
{
  return new String( value_.substr( begin - 1, len ) );
}

void String::toUpper( void )
{
  value_.toUpper();
}

void String::toLower( void )
{
  value_.toLower();
}

BObjectImp* String::array_assign( BObjectImp* idx, BObjectImp* target, bool /*copy*/ )
{
  std::string::size_type pos, len;

  // first, determine position and length.
  if ( idx->isa( OTString ) )
  {
    String& rtstr = (String&)*idx;
    pos = value_.find( rtstr.value_ );
    len = rtstr.lengthc();
  }
  else if ( idx->isa( OTLong ) )
  {
    BLong& lng = (BLong&)*idx;
    pos = lng.value() - 1;
    len = 1;
  }
  else if ( idx->isa( OTDouble ) )
  {
    Double& dbl = (Double&)*idx;
    pos = static_cast<UnicodeString::size_type>( dbl.value() );
    len = 1;
  }
  else
  {
    return UninitObject::create();
  }

  if ( pos != UnicodeString::npos )
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
  unsigned index;
  if ( start.isa( OTLong ) )
  {
    BLong& lng = (BLong&)start;
    index = (unsigned)lng.value();
    if ( index == 0 || index > value_.lengthc() )
      return BObjectRef( new BError( "Subscript out of range" ) );
  }
  else if ( start.isa( OTString ) )
  {
    String& rtstr = (String&)start;
    std::string::size_type pos = value_.find( rtstr.value_ );
    if ( pos != std::string::npos )
      index = static_cast<unsigned int>( pos + 1 );
    else
      return BObjectRef( new UninitObject );
  }
  else
  {
    return BObjectRef( copy() );
  }

  // now deal with the length.
  int len;
  if ( length.isa( OTLong ) )
  {
    BLong& lng = (BLong&)length;

    len = (int)lng.value();
  }
  else if ( length.isa( OTDouble ) )
  {
    Double& dbl = (Double&)length;

    len = (int)dbl.value();
  }
  else
  {
    return BObjectRef( copy() );
  }

  if ( target->isa( OTString ) )
  {
    String* target_str = (String*)target;
    value_.replace( index - 1, len, target_str->value_ );
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
  unsigned index;
  if ( start.isa( OTLong ) )
  {
    BLong& lng = (BLong&)start;
    index = (unsigned)lng.value();
    if ( index == 0 || index > value_.lengthc() )
      return BObjectRef( new BError( "Subscript out of range" ) );
  }
  else if ( start.isa( OTString ) )
  {
    String& rtstr = (String&)start;
    std::string::size_type pos = value_.find( rtstr.value_ );
    if ( pos != std::string::npos )
      index = static_cast<unsigned int>( pos + 1 );
    else
      return BObjectRef( new UninitObject );
  }
  else
  {
    return BObjectRef( copy() );
  }

  // now deal with the length.
  int len;
  if ( length.isa( OTLong ) )
  {
    BLong& lng = (BLong&)length;

    len = (int)lng.value();
  }
  else if ( length.isa( OTDouble ) )
  {
    Double& dbl = (Double&)length;

    len = (int)dbl.value();
  }
  else
  {
    return BObjectRef( copy() );
  }

  auto str = new String( value_.substr(index - 1, len) );
  return BObjectRef( str );
}

BObjectRef String::OperSubscript( const BObject& rightobj )
{
  const BObjectImp& right = rightobj.impref();
  if ( right.isa( OTLong ) )
  {
    BLong& lng = (BLong&)right;

    unsigned index = (unsigned)lng.value();

    if ( index == 0 || index > value_.lengthc() )
      return BObjectRef( new BError( "Subscript out of range" ) );

    return BObjectRef( new BObject( new String( value_[index - 1] ) ) );
  }
  else if ( right.isa( OTDouble ) )
  {
    Double& dbl = (Double&)right;

    unsigned index = (unsigned)dbl.value();

    if ( index == 0 || index > value_.lengthc() )
      return BObjectRef( new BError( "Subscript out of range" ) );

    return BObjectRef( new BObject( new String( value_[index - 1] ) ) );
  }
  else if ( right.isa( OTString ) )
  {
    String& rtstr = (String&)right;
    auto pos = value_.find( rtstr.value_ );
    if ( pos != std::string::npos )
      return BObjectRef( new BObject( new String( value_[pos - 1] ) ) );
    else
      return BObjectRef( new UninitObject );
  }
  else
  {
    return BObjectRef( new UninitObject );
  }
}

// ------------------- FORMATTING INTERNAL FUNCTIONS ------------------------

/** Internal function used by format, parses a tag into an int */
bool s_parse_int( int& i, UnicodeString const& s )
{
  if ( s.empty() )
    return false;

  char* end;
  i = strtol( s.utf8().c_str(), &end, 10 );

  if ( !*end )
  {
    return true;
  }
  else
  {
    return false;
  }
}

/** Internal function used by format, converts int to binary string */
void int_to_binstr( int& value, UnicodeString& s )
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
      s += '1';
    else
      s += '0';
  }
}

/** Internal supplementary function used by format */
bool try_to_format( UnicodeString& to_string, BObjectImp* what, UnicodeString& frmt )
{
  if ( frmt.empty() )
  {
    to_string += what->getStringRep();
    return false;
  }

  if ( frmt.find( 'b' ) != std::string::npos )
  {
    if ( !what->isa( BObjectImp::OTLong ) )
    {
      to_string.append(StrEncoding::UTF8, "<needs Int>");
      return false;
    }
    BLong* plong = static_cast<BLong*>( what );
    int n = plong->value();
    if ( frmt.find( '#' ) != std::string::npos )
    {
      to_string.append(StrEncoding::UTF8, ( ( n < 0 ) ? "-" : "" ));
      to_string.append(StrEncoding::UTF8, "0b");
    }
    int_to_binstr( n, to_string );
  }
  else if ( frmt.find( 'x' ) != std::string::npos )
  {
    if ( !what->isa( BObjectImp::OTLong ) )
    {
      to_string.append(StrEncoding::UTF8, "<needs Int>");
      return false;
    }
    BLong* plong = static_cast<BLong*>( what );
    int n = plong->value();
    if ( frmt.find( '#' ) != std::string::npos )
      to_string.append(StrEncoding::UTF8, "0x");

    std::stringstream ss;
    ss << std::hex << n << std::dec;
    to_string.append(StrEncoding::UTF8, ss.str());
  }
  else if ( frmt.find( 'o' ) != std::string::npos )
  {
    if ( !what->isa( BObjectImp::OTLong ) )
    {
      to_string.append(StrEncoding::UTF8, "<needs Int>");
      return false;
    }
    BLong* plong = static_cast<BLong*>( what );
    int n = plong->value();
    if ( frmt.find( '#' ) != std::string::npos )
      to_string.append(StrEncoding::UTF8, "0o");

    std::stringstream ss;
    ss << std::oct << n << std::dec;
    to_string.append(StrEncoding::UTF8, ss.str());
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
      to_string.append(StrEncoding::UTF8, "<needs Int, Double>");
      return false;
    }

    std::stringstream ss;
    ss << std::dec << n;
    to_string.append(StrEncoding::UTF8, ss.str());
  }
  else
  {
    to_string.append(StrEncoding::UTF8, "<bad format: ");
    to_string += frmt;
    to_string += '>';
    return false;
  }
  return true;
}

// ------------------- METHOD FUNCTIONS -------------------------------------

BObjectImp* String::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != NULL )
    return this->call_method_id( objmethod->id, ex );
  else
    return NULL;
}
BObjectImp* String::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  switch ( id )
  {
  case MTH_LENGTH:
    if ( ex.numParams() == 0 )
      return new BLong( static_cast<int>( value_.lengthc() ) );
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
      UnicodeString result = UnicodeString();

      size_t tag_start_pos;  // the position of tag's start "{"
      size_t tag_stop_pos;   // the position of tag's end "}"
      size_t tag_dot_pos;

      int tag_param_idx;

      size_t str_pos = 0;               // current string position
      unsigned int next_param_idx = 0;  // next index of .format() parameter

      UnicodeString w_spaces = UnicodeString(StrEncoding::UTF8, "\t ");

      // Tells whether last found tag was an integer
      bool last_tag_was_int = true;

      while ( ( tag_start_pos = value_.find( '{', str_pos ) ) != UnicodeString::npos )
      {
        if ( ( tag_stop_pos = value_.find( '}', tag_start_pos ) ) != UnicodeString::npos )
        {
          result += value_.substr( str_pos, tag_start_pos - str_pos );
          str_pos = tag_stop_pos + 1;

          UnicodeString tag_body =
              value_.substr( tag_start_pos + 1, ( tag_stop_pos - tag_start_pos ) - 1 );

          tag_start_pos = tag_body.find_first_not_of( w_spaces );
          tag_stop_pos = tag_body.find_last_not_of( w_spaces );

          // cout << "' tag_body1: '" << tag_body << "'";

          // trim the tag of whitespaces (slightly faster code ~25%)
          if ( tag_start_pos != UnicodeString::npos && tag_stop_pos != UnicodeString::npos )
            tag_body = tag_body.substr( tag_start_pos, ( tag_stop_pos - tag_start_pos ) + 1 );
          else if ( tag_start_pos != UnicodeString::npos )
            tag_body = tag_body.substr( tag_start_pos );
          else if ( tag_stop_pos != UnicodeString::npos )
            tag_body = tag_body.substr( 0, tag_stop_pos + 1 );

          // s_trim( tag_body ); // trim the tag of whitespaces

          // cout << "' tag_body2: '" << tag_body << "'";

          UnicodeString frmt;
          size_t formatter_pos = tag_body.find( ':' );

          if ( formatter_pos != UnicodeString::npos )
          {
            frmt = tag_body.substr( formatter_pos + 1, UnicodeString::npos );  //
            tag_body = tag_body.substr( 0, formatter_pos );  // remove property from the tag
          }

          UnicodeString prop_name;
          // parsing {1.this_part}
          tag_dot_pos = tag_body.find( '.', 0 );

          // '.' is found within the tag, there is a property name
          if ( tag_dot_pos != UnicodeString::npos )
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
              result.append(StrEncoding::UTF8, "<idx required before: '");
              result += prop_name;
              result.append(StrEncoding::UTF8, "'>");
              continue;
            }
          }
          else
          {
            if ( tag_body.empty() )
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

          if ( (int)ex.numParams() <= tag_param_idx )
          {
            result.append(StrEncoding::UTF8, "<invalid index: #");
            result.append(StrEncoding::UTF8, boost::lexical_cast<std::string>(tag_param_idx + 1));
            result += '>';
            continue;
          }

          BObjectImp* imp = ex.getParamImp( tag_param_idx );

          if ( !prop_name.empty() )
          {  // accesing object
            BObjectRef obj_member = imp->get_member( prop_name.utf8().c_str() );
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

      if ( str_pos < value_.lengthc() )
      {
        result += value_.substr( str_pos, std::string::npos );
      }

      return new String( result );
    }
    else
    {
      return new BError( "string.format() requires a parameter." );
    }
  }
  case MTH_JOIN:
  {
    BObject* cont;
    if ( ex.numParams() == 1 && ( cont = ex.getParamObj( 0 ) ) != NULL )
    {
      if ( !( cont->isa( OTArray ) ) )
        return new BError( "string.join expects an array" );
      ObjArray* container = static_cast<ObjArray*>( cont->impptr() );
      // no empty check here on purpose
      UnicodeString joined= UnicodeString();
      bool first = true;
      for ( const BObjectRef& ref : container->ref_arr )
      {
        if ( ref.get() )
        {
          BObject* bo = ref.get();

          if ( bo == NULL )
            continue;
          if ( !first )
            joined += value_;
          else
            first = false;
          joined += bo->impptr()->getStringRep();
        }
      }
      return new String( joined );
    }
    else
      return new BError( "string.join(array) requires a parameter." );
  }
  default:
    return NULL;
  }
}

}
}
