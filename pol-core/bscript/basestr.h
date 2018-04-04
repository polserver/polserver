/** @file
 *
 * @par History
 * - 2015/20/12 Bodom:     creating this file to handle string/unicode
 */


#pragma once

#ifndef STLUTIL_H
#include "../clib/stlutil.h"
#endif

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#ifndef BSCRIPT_BERROR_H
#include "berror.h"
#endif

#include "objmethods.h"
#include "executor.h"

#include <boost/lexical_cast.hpp>
#include <string>
#include <stack>

// Just like "this", but with explicit cast to child (see BaseStringTpl doc)
#define this_child (static_cast<D*>(this))
#define this_child_const (static_cast<const D*>(this))

namespace Pol {
namespace Bscript {

/**
 * This is a generic INTERFACE for the base string object
 *
 * Methods defined here are guaranteed to be implemented with the same signature
 * in all the string-like objects.
 * All methods must be PUBLIC and PURE VIRTUAL.
 */
class BaseString : public BObjectImp {
public:

  /**
   * Types for the trim() method
   */
  enum class TrimTypes
  {
    TRIM_LEFT,
    TRIM_RIGHT,
    TRIM_BOTH
  };

  /**
   * Returns this String length, in characters
   */
  virtual size_t length() const = 0;

  /**
   * Returns the amount of alpha-numeric characters in string.
   */
  virtual unsigned int alnumlen() const = 0;

  /**
   * Returns how many safe to print characters can be read
   * from string until a non-safe one is found
   */
  virtual unsigned int SafeCharAmt() const = 0;

  /**
   * In-place reverse this string
   */
  virtual void reverse() = 0;

  /*
   * Copies the c-style string into this one and deletes the pointer
   */
  virtual void set( char *newstr ) = 0;

  /**
   * Parses this string, interpreting its content as an integral number
   */
  virtual unsigned long intval() const = 0;

  /**
   * Parses this string, interpreting its content as floating point number
   */
  virtual double dblval() const = 0;

  /**
   * Returns an ANSI value for this string
   * @warning precision may be lost, non-ANSI characters are replaced
   */
  virtual std::string strval() const = 0;

  /**
   * Prints a formatted version of this string into the given stream
   */
  virtual void printOn(std::ostream& os) const = 0;

protected:
   /**
   * Constructor to be used only by child classes
   */
  explicit BaseString(BObjectImp::BObjectType type) : BObjectImp( type ) {}

};




/**
 * This is an ABSTRACT class from which all string-like bscript objects
 * must be derived from (currently String and Unicode, can't imagine more)
 *
 * @param V type of the underlying value_ container
 * @param D type of the derived class (the class implementing this template itself)
 *          Just found this technique has a name: CRTP - Curiously Recurring Template Pattern
 * @param T ID for the object type, constant for BObjectImp constructor
 */
template <typename V, typename D, BObjectImp::BObjectType T>
class BaseStringTpl : public BaseString {

typedef typename V::size_type V_size_type;

public:
// ---------------- CONSTRUCTORS ------------------------------------------

  /**
   * Creates an empty instance
   */
  BaseStringTpl() : BaseString( T ), value_() {}

  /**
   * Creates an instance from an object of the same class
   */
  BaseStringTpl( const D& str ) : BaseString( T ), value_( str.value_ ) {}

  /**
   * Creates an instance from any other object (convert to String)
   */
  explicit BaseStringTpl( BObjectImp& objimp ) :
    BaseString( T ),
    value_( objimp.getStringRep() )
  {}

  /**
   * Creates an instance from a parameter of the same type of the internal storage object
   */
  explicit BaseStringTpl( const V& str ) : BaseString( T ), value_( str ) {}

  /**
   * Creates an instance from a part of a parameter
   * of the same type of the internal storage object
   */
  explicit BaseStringTpl( const V& str, V_size_type pos, V_size_type n )
    : BaseString( T ), value_( str, pos, n ) {}

  /**
   * Creates an instance by concatenating two instances
   */
  explicit BaseStringTpl( const D& left, const D& right ) :
    BaseString( T ),
    value_( left.value_ + right.value_ )
  {}

  virtual ~BaseStringTpl()
  {}

// ----------------- SEARCH/QUERY FUNCTIONS -------------------------------

  size_t length() const
  {
    return value_.length();
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
  int find( int begin, const V &target )
  {
    V_size_type pos;
    pos = value_.find( target, begin );
    if ( pos == V::npos )
      return -1;
    else
      return static_cast<int>( pos );
  }

  unsigned int alnumlen() const
  {
    unsigned int c = 0;
    while ( iswalnum(static_cast<wchar_t>(value_[c])) )
      c++;
    return c;
  }

  unsigned int SafeCharAmt() const
  {
    int strlen = static_cast<int>( this->length() );
    for ( int i = 0; i < strlen; i++ )
    {
      wchar_t tmp = value_[i];
      if ( iswalnum( tmp ) ) // a-z A-Z 0-9
        continue;
      else if ( iswpunct( tmp ) ) // !"#$%&'()*+,-./:;<=>?@{|}~
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

  /**
   * Returns the internal storage object
   */
  const V& value() const
  {
    return value_;
  }

  std::string strval() const
  {
    return static_cast<std::string>(value_);
  }


// ------------------- MANIPULATION FUNCTIONS -------------------------------

  /** Replace this string value with a given string value */
  D& operator=( const D& str ) {
    value_ = str.value_;
    return *this_child;
  }
  /** Replace this string internal value with a given internal value */
  D& operator=( const V& str ) {
    value_ = str;
    return *this_child;
  }
  /** Appends given string to this one */
  D& operator+=( const D& str ) {
    value_ += str.value_;
    return *this_child;
  }
  /** Appends internal value to this string */
  D& operator+=( const V& str ) {
    value_ += str;
    return *this_child;
  }

  /**
   * Copies the value from the given string into this one
   */
  void copyvalue( const D& str ) {
    value_ = str.value_;
  }

  void reverse()
  {
    std::reverse(value_.begin(), value_.end());
  }

  void set( char *newstr )
  {
    value_ = newstr;
    delete newstr;
  }

  /**
   * Looks for a substring and deletes it if found
   */
  void remove( const V &rm )
  {
    size_t len = rm.length();

    auto pos = value_.find(rm);
    if (pos != V::npos)
      value_.erase( pos, len );
  }

  /**
   * Trims this string: removes whitespaces at the beginning/ending/both
   *
   * @param crSet string containing the list of characters to consider whitespace
   * @param type enum, the trim type
   */
  void trim( const V &crSet, TrimTypes type )
  {
    if ( type == TrimTypes::TRIM_LEFT || type == TrimTypes::TRIM_BOTH )
    {
      // Find the first character position after excluding leading blank spaces
      size_t startpos = value_.find_first_not_of( crSet, 0 );
      if ( V::npos != startpos )
        value_ = value_.substr( startpos );
      else
        value_.clear();
    }

    if ( type == TrimTypes::TRIM_RIGHT || type == TrimTypes::TRIM_BOTH )
    {
      // Find the first character position from reverse
      size_t endpos = value_.find_last_not_of( crSet, V::npos );
      if ( V::npos != endpos )
        value_ = value_.substr( 0, endpos + 1 );
      else
        value_.clear();
    }
  }

  /**
   * Searches this string for all occurrences of src and replaces them with rep
   *
   * @param src The substring to search for
   * @param rep The string to replace with
   */
  void replace( const D& src, const D& rep )
  {
    V_size_type valpos = 0;
    while ( V::npos != (valpos = value_.find(src.value_, valpos)))
    {
      value_.replace( valpos, src.length(), rep.value_ );
      valpos += rep.length();
    }
  }

  /**
   * Replaces a portion of this string with a given one
   *
   * @param replace_with The string to replace with
   * @param index The starting position to replace from
   * @param len The amount of characters to be replaced
   */
  void replace( const D& replace_with, const unsigned int index, const unsigned int len )
  {
    value_.replace( index - 1, len, replace_with.value_ );
  }

  /**
   * Returns a new string containing a part of this string
   *
   * @param begin 0-based position of the first char
   * @param len number of characters to include in the substring
   */
  D* substr( const int begin, const int len ) const
  {
    return new D( value_.substr( begin, len ) );
  }

// ------------------- FORMATTING INTERNAL FUNCTIONS ------------------------
private:

  /** Internal function used by format, parses a tag into an int */
  bool s_parse_int(int &i, const std::string &s) const
  {
    if ( s.empty() )
      return false;

    char* end;
    i = strtol( s.c_str(), &end, 10 );

    if ( !*end )
      return true;
    else
      return false;
  }

  /** Internal function used by format, converts int to binary string */
  void int_to_binstr( int& value, std::stringstream &s ) const
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

  /** Internal supplementary function used by format */
  bool try_to_format( std::stringstream& to_stream, BObjectImp* what, std::string& frmt ) const
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
      to_stream << std::hex << n;
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
      to_stream << std::oct << n;
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

  /** Internal */
  bool try_to_format( std::unique_ptr<D>& toStr, BObjectImp* what, std::string& frmt ) const
  {
    std::stringstream to_stream;
    bool ret = try_to_format(to_stream, what, frmt);
    *toStr.get() += to_stream.str();
    return ret;
  }

// ------------------- METHOD FUNCTIONS -------------------------------------
protected:

  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) POL_OVERRIDE
  {
    ObjMethod* objmethod = getKnownObjMethod( methodname );
    if ( objmethod != NULL )
      return this->call_method_id( objmethod->id, ex );
    else
      return NULL;
  }

  virtual BObjectImp* call_method_id( const int id, Executor& ex,
    bool /* forcebuiltin */ = false ) POL_OVERRIDE
  {
    switch ( id )
    {
      case MTH_LENGTH:
        if ( ex.numParams() == 0 )
          return new BLong( static_cast<int>( length() ) );
        else
          return new BError( childName() + ".length() doesn't take parameters." );
        break;

      case MTH_FIND:
      {
        if ( ex.numParams() > 2 )
          return new BError( childName() + ".find(Search, [Start]) takes only two parameters" );
        if ( ex.numParams() < 1 )
          return new BError( childName() + ".find(Search, [Start]) takes at least one parameter" );
        //FIXME: re-check this!
        const V s = static_cast<D*>( ex.getParamImp(0,T ) )->value_;
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
          this_child->toUpper();
          return this;
        }
        else
          return new BError( childName() + ".upper() doesn't take parameters." );
      }

      case MTH_LOWER:
      {
        if ( ex.numParams() == 0 )
        {
          this_child->toLower();
          return this;
        }
        else
          return new BError( childName() + ".lower() doesn't take parameters." );
      }

      case MTH_FORMAT:
      {
        if ( ex.numParams() > 0 )
        {
          std::unique_ptr<D> result(new D());

          size_t tag_start_pos;  // the position of tag's start "{"
          size_t tag_stop_pos;   // the position of tag's end "}"
          size_t tag_dot_pos;

          int tag_param_idx;

          size_t str_pos = 0;         // current string position
          unsigned int next_param_idx = 0;  // next index of .format() parameter

          V w_spaces = "\t ";

          // Tells whether last found tag was an integer
          bool last_tag_was_int = true;

          while ( ( tag_start_pos = value_.find( '{', str_pos ) ) != V::npos )
          {
            if ( ( tag_stop_pos = value_.find( '}', tag_start_pos ) ) != V::npos )
            {
              *result.get() += value_.substr( str_pos, tag_start_pos - str_pos );
              str_pos = tag_stop_pos + 1;

              V tag_body = value_.substr( tag_start_pos + 1,
                                          ( tag_stop_pos - tag_start_pos ) - 1 );

              tag_start_pos = tag_body.find_first_not_of( w_spaces );
              tag_stop_pos = tag_body.find_last_not_of( w_spaces );

              // cout << "' tag_body1: '" << tag_body << "'";

              // trim the tag of whitespaces (slightly faster code ~25%)
              if ( tag_start_pos != V::npos && tag_stop_pos != V::npos )
                tag_body = tag_body.substr( tag_start_pos, ( tag_stop_pos - tag_start_pos ) + 1 );
              else if ( tag_start_pos != V::npos )
                tag_body = tag_body.substr( tag_start_pos );
              else if ( tag_stop_pos != V::npos )
                tag_body = tag_body.substr( 0, tag_stop_pos + 1 );

              // cout << "' tag_body2: '" << tag_body << "'";

              std::string frmt;
              size_t formatter_pos = tag_body.find( ':' );

              if ( formatter_pos != V::npos )
              {
                frmt = static_cast<std::string>(tag_body.substr( formatter_pos + 1, V::npos ));
                tag_body = tag_body.substr( 0, formatter_pos );  // remove property from the tag
              }

              V prop_name;
              // parsing {1.this_part}
              tag_dot_pos = tag_body.find( '.', 0 );

              // '.' is found within the tag, there is a property name
              if ( tag_dot_pos != std::string::npos )
              {
                last_tag_was_int = true;
                prop_name = tag_body.substr( tag_dot_pos + 1, V::npos );  //
                tag_body = tag_body.substr( 0, tag_dot_pos );  // remove property from the tag

                // if s_tag_body is numeric then use it as an index
                if ( s_parse_int( tag_param_idx, static_cast<std::string>(tag_body) ) )
                {
                  tag_param_idx -= 1;  // sinse POL indexes are 1-based
                }
                else
                {
                  *result.get() += V("<idx required before: '") + prop_name + V("'>");
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
                else if ( s_parse_int( tag_param_idx, static_cast<std::string>(tag_body) ) )
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

              if ( ex.numParams() <= tag_param_idx )
              {
                *result.get() += V("<invalid index: #")
                              + V(boost::lexical_cast<std::string>( tag_param_idx + 1 )) + V(">");
                continue;
              }

              BObjectImp* imp = ex.getParamImp( tag_param_idx );

              if ( !prop_name.empty() )
              {  // accesing object
                BObjectRef obj_member = imp->get_member(
                  static_cast<std::string>(prop_name).c_str() );
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
            *result.get() += value_.substr( str_pos, std::string::npos );
          }

          return result.release();
        }
        else
        {
          return new BError( childName() + ".format() requires a parameter." );
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
          std::unique_ptr<D> joined(new D());
          bool first = true;
          for ( const BObjectRef& ref : container->ref_arr )
          {
            if ( ref.get() )
            {
              BObject* bo = ref.get();

              if ( bo == NULL )
                continue;
              if ( !first )
                *joined.get() += value_;
              else
                first = false;
              *joined.get() += bo->impptr()->getStringRep();
            }
          }
          return joined.release();
        }
        else
          return new BError( childName() + ".join(array) requires a parameter." );
      }

      default:
        return NULL;
    }
  }

// ------------------- POL Operator implementations --------------------------
public:

  virtual BObjectImp* selfPlusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE
  {
    return objimp.selfPlusObj( *this_child_const );
  }
  virtual BObjectImp* selfPlusObj( const BObjectImp& objimp ) const POL_OVERRIDE
  {
    return new D( value_ + objimp.getStringRep() );
  }
  virtual BObjectImp* selfPlusObj( const BLong& objimp ) const POL_OVERRIDE
  {
    return new D( value_ + objimp.getStringRep() );
  }
  virtual BObjectImp* selfPlusObj( const Double& objimp ) const POL_OVERRIDE
  {
    return new D( value_ + objimp.getStringRep() );
  }
  virtual BObjectImp* selfPlusObj( const String& objimp ) const POL_OVERRIDE
  {
    return new D( value_ + objimp.getStringRep() );
  }
  virtual BObjectImp* selfPlusObj( const ObjArray& objimp ) const POL_OVERRIDE
  {
    return new D( value_ + objimp.getStringRep() );
  }
  virtual void selfPlusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE
  {
    objimp.selfPlusObj( *this_child, obj );
  }
  virtual void selfPlusObj( BObjectImp& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    value_ += objimp.getStringRep();
  }
  virtual void selfPlusObj( BLong& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    value_ += objimp.getStringRep();
  }
  virtual void selfPlusObj( Double& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    value_ += objimp.getStringRep();
  }
  virtual void selfPlusObj( String& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    value_ += objimp.getStringRep();
  }
  virtual void selfPlusObj( ObjArray& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    value_ += objimp.getStringRep();
  }

  virtual BObjectImp* selfMinusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE
  {
    return objimp.selfMinusObj( *this_child_const );
  }
  virtual BObjectImp* selfMinusObj( const BObjectImp& objimp ) const POL_OVERRIDE
  {
    D* tmp = static_cast<D*>(copy());
    tmp->remove( objimp.getStringRep() );
    return tmp;
  }
  virtual BObjectImp* selfMinusObj( const BLong& objimp ) const POL_OVERRIDE
  {
    D* tmp = static_cast<D*>(copy());
    tmp->remove( objimp.getStringRep() );
    return tmp;
  }
  virtual BObjectImp* selfMinusObj( const Double& objimp ) const POL_OVERRIDE
  {
    D* tmp = static_cast<D*>(copy());
    tmp->remove( objimp.getStringRep() );
    return tmp;
  }
  virtual BObjectImp* selfMinusObj( const String& objimp ) const POL_OVERRIDE
  {
    D* tmp = static_cast<D*>(copy());
    tmp->remove( objimp.getStringRep() );
    return tmp;
  }
  virtual BObjectImp* selfMinusObj( const ObjArray& objimp ) const POL_OVERRIDE
  {
   D* tmp = static_cast<D*>(copy());
    tmp->remove( objimp.getStringRep() );
    return tmp;
  }
  virtual void selfMinusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE
  {
    objimp.selfMinusObj( *this_child, obj );
  }
  virtual void selfMinusObj( BObjectImp& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    remove( objimp.getStringRep() );
  }
  virtual void selfMinusObj( BLong& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    remove( objimp.getStringRep() );
  }
  virtual void selfMinusObj( Double& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    remove( objimp.getStringRep() );
  }
  virtual void selfMinusObj( String& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    remove( objimp.value() );
  }
  virtual void selfMinusObj( ObjArray& objimp, BObject& /*obj*/ ) POL_OVERRIDE
  {
    remove( objimp.getStringRep() );
  }

  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE
  {
    if ( objimp.isa( T ) )
      return ( value_ == static_cast<const D&>( objimp ).value_ );

    if ( objimp.isa( OTBoolean ) )
      return isTrue() == static_cast<const BBoolean&>( objimp ).isTrue();

    return false;
  }

  /**
   * Since non-Strings show up here as not equal, we make them less than.
   *
   * @todo TODO: Change this behavior? It doesn't make much sense - 12-27-2015 Bodom
   */
  virtual bool operator<( const BObjectImp& objimp ) const POL_OVERRIDE
  {
    if ( objimp.isa( T ) )
      return ( value_ < static_cast<const D&>( objimp ).value_ );

    return BObjectImp::operator<( objimp );
  }


  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target,
    bool /*copy*/ ) POL_OVERRIDE
  {
    V_size_type pos, len;

    // first, determine position and length.
    if ( idx->isa( T ) )
    {
      D& rtstr = (D&)*idx;
      pos = value_.find( rtstr.value_ );
      len = rtstr.length();
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
      pos = static_cast<std::string::size_type>(dbl.value());
      len = 1;
    }
    else
    {
      return UninitObject::create();
    }

    if (pos != V::npos)
    {
      if ( target->isa( OTString ) )
      {
        D* target_str = (D*)target;
        value_.replace( pos, len, target_str->value_ );
      }
      return this_child;
    }
    else
    {
      return UninitObject::create();
    }
  }

  virtual BObjectRef OperMultiSubscriptAssign(std::stack<BObjectRef>& indices,
    BObjectImp* target) POL_OVERRIDE
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
      if ( index == 0 || index > value_.size() )
        return BObjectRef( new BError( "Subscript out of range" ) );

    }
    else if ( start.isa( T ) )
    {
      D& rtstr = (D&)start;
      V_size_type pos = value_.find(rtstr.value_);
      if (pos != V::npos)
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
      BLong& lng = (BLong &)length;

      len = (int)lng.value();
    }
    else if ( length.isa( OTDouble ) )
    {
      Double& dbl = (Double &)length;

      len = (int)dbl.value();
    }
    else
    {
      return BObjectRef( copy() );
    }

    if ( target->isa( T ) )
    {
      D* target_str = (D*)target;
      value_.replace( index - 1, len, target_str->value_ );
    }
    else
    {
      return BObjectRef( UninitObject::create() );
    }

    return BObjectRef( this_child );
  }


  virtual BObjectRef OperMultiSubscript(std::stack<BObjectRef>& indices) POL_OVERRIDE
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
      if ( index == 0 || index > value_.size() )
        return BObjectRef( new BError( "Subscript out of range" ) );

    }
    else if ( start.isa( T ) )
    {
      D& rtstr = (D&)start;
      V_size_type pos = value_.find(rtstr.value_);
      if (pos != V::npos)
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
      BLong& lng = (BLong &)length;

      len = (int)lng.value();
    }
    else if ( length.isa( OTDouble ) )
    {
      Double& dbl = (Double &)length;

      len = (int)dbl.value();
    }
    else
    {
      return BObjectRef( copy() );
    }

    auto str = new D( value_, index - 1, len );
    return BObjectRef( str );
  }

  virtual BObjectRef OperSubscript( const BObject& rightobj ) POL_OVERRIDE
  {
    const BObjectImp& right = rightobj.impref();
    if ( right.isa( OTLong ) )
    {
      BLong& lng = (BLong&)right;

      unsigned index = (unsigned)lng.value();

      if ( index == 0 || index > value_.size() )
        return BObjectRef( new BError( "Subscript out of range" ) );

      return BObjectRef( new BObject( new D( value_.c_str() + index - 1, 1 ) ) );
    }
    else if ( right.isa( OTDouble ) )
    {
      Double& dbl = (Double&)right;

      unsigned index = (unsigned)dbl.value();

      if ( index == 0 || index > value_.size() )
        return BObjectRef( new BError( "Subscript out of range" ) );

      return BObjectRef( new BObject( new D( value_.c_str() + index - 1, 1 ) ) );
    }
    else if ( right.isa( T ) )
    {
      D& rtstr = (D&)right;
      auto pos = value_.find(rtstr.value_);
      if (pos != V::npos)
        return BObjectRef( new BObject( new D( value_, pos, 1 ) ) );
      else
        return BObjectRef( new UninitObject );
    }
    else
    {
      return BObjectRef( new UninitObject );
    }
  }

  /**
   * Returns a copy of this object allocated on the heap
   */
  virtual BObjectImp* copy() const POL_OVERRIDE
  {
    return new D( *this_child_const );
  }

  virtual bool isTrue() const POL_OVERRIDE
  {
    return ! value_.empty();
  }


// ------------------- INTERNAL ----------------------------------------------

protected:
  /** The internal storage string */
  V value_;

  /**
   * Returns a pointer to a substring of this string
   */
  D* midstring( int begin, int len ) const
  {
    return new D( value_.substr( begin - 1, len ) );
  }

  /**
   * Returns the name of the child class
   */
  std::string childName() const
  {
    return std::string( typeid(D).name() );
  }

};

}
}

#undef this_child
#undef this_child_const
