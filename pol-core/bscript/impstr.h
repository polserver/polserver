/** @file
 *
 * @par History
 * - 2018/04/04 Bodom:     adding unicode support
 *
 * @note This file uses UTF8 encoding
 */

#pragma once

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#include <stack>
#include <string>

#include "../clib/unicode.h"

namespace Pol
{
namespace Bscript
{

using Clib::UnicodeString;
using Clib::Utf8CharRef;
using Clib::StrEncoding;

/**
 * Represents an Unicode string. Like a string, but can hold unicode characters like
 * ö or 本.
 * Uses UnicodeString as internal storage.
 * bscript ID: OTString
 *
 * @see UnicodeString
 */
class String : public BObjectImp
{
  typedef BObjectImp base;

public:
// --------------------------- CONSTRUCTORS -----------------------------

  /** Creates an empty String */
  inline String() : BObjectImp( OTString ), value_() {}
  /** Creates a String from another String */
  inline String( const String& str ) : BObjectImp( OTString ), value_( str.value_ ) {};
  /** Creates a String from any other object (convert it to string) */
  inline explicit String( BObjectImp& objimp )
    : BObjectImp( OTString ), value_( objimp.getStringRep() ) {};
  /** Creates an instance from a raw Unicode string */
  inline explicit String( const UnicodeString& str ) : BObjectImp( OTString ), value_( str ) {};
  /** Creates an instance from a single char in a raw Unicode String */
  inline explicit String( const Utf8CharRef& chr) : BObjectImp( OTString ), value_( chr ) {};
  /** Constructs from a UO-UTF16 string */
  inline explicit String( const std::u16string& str ) : BObjectImp( OTString ), value_( str ) {};
  /** Constructs from a UTF32 string */
  inline explicit String( const std::u32string& str ) : BObjectImp( OTString ), value_( str ) {};
  /** Creates an instance by concatenating two strings */
  inline explicit String( const String& left, const String& right )
    : BObjectImp( OTString ), value_( left.value_ + right.value_ ) {};

// -------------------- COMPATIBILITY CONSTRUCTORS ----------------------

  /** Constructs from a standard UTF8 string.*/
  inline String( const std::string& str )
    : BObjectImp( OTString ), value_( str ) {};
  /** Constructs from a standard UTF8 C string.*/
  inline String( const char* str )
    : BObjectImp( OTString ), value_( str ) {};
  /** Construct from a fixed amount of bytes from pointer */
  inline String( const char* str, size_t nbytes )
    : BObjectImp( OTString ), value_( str, nbytes ) {};

// ---------------------- PACK/UNPACK STUFF -----------------------------
  static BObjectImp* unpack( const char* pstr );
  static BObjectImp* unpack( std::istream& is );
  static BObjectImp* unpackWithLen( std::istream& is );
  virtual std::string pack() const POL_OVERRIDE;
  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
  static void packonto( std::ostream& os, const UnicodeString& value );

// -------------------------- MISC ----------------------------------------
  virtual size_t sizeEstimate() const POL_OVERRIDE;

  void toUpper();
  void toLower();

  /** @see UnicodeString::intval() */
  inline unsigned long intval() const { return value_.intval(); }

  /** @see UnicodeString::dblval() */
  inline double dblval() const { return value_.dblval(); }

  inline virtual UnicodeString getStringRep() const POL_OVERRIDE
  {
    return UnicodeString(value_);
  }
  inline virtual UnicodeString getFormattedStringRep() const POL_OVERRIDE
  {
    return UnicodeString(StrEncoding::UTF8, "\"") + value_
      + UnicodeString(StrEncoding::UTF8, "\"");
  }
  virtual void printOn(std::ostream& os) const;

  /**
   * Returns a copy of this object allocated on the heap
   */
  virtual BObjectImp* copy() const POL_OVERRIDE { return new String( *this ); }

  /// @see UnicodeString::trim()
  inline void trim( const UnicodeString& crSet, UnicodeString::TrimTypes type )
  {
    return value_.trim(crSet, type);
  }
  // FIXME: Possibly upgrade this later with overload functions in order to support different
  // trim methods, or add ELTrim, etc?
  //String* ETrim( const char* CRSet, int type );

  void replace( const String& src, const String& rep );
  void replace( const String& replace_with, const unsigned int index, const unsigned int len );
  String* substr( const int begin, const int len ) const;
  ObjArray* split( const std::vector<UnicodeString>& delimiters, int max_split = -1,
    bool ignore_multiple = false );

  void set( char* newstr ); /* String now owns newstr */
  /** Returns the internal storage object */
  const UnicodeString& value() const { return value_; }
  /** Returns the internal utf8 bytes */
  const std::string& utf8() const { return value_.utf8(); }
  /**
   * Returns the string encoded as ansi
   *
   * @warning Non-ANSI characters will be replaced
   */
  std::string ansi() const { return value_.asAnsi(true); }
  /**
   * Returns the string encoded as ascii
   *
   * @warning Non-ASCII characters will be replaced
   */
  std::string ascii() const { return value_.asAscii(true); }
  /** Returns true when this string is ascii-compatible */
  inline bool isAscii() const { return value_.isAscii(); }
  /** Returns length in characters */
  inline size_t lengthc() const { return value_.lengthc(); }

  virtual ~String() {}
  /*
  String& operator=( const char* s )
  {
    value_ = s;
    return *this;
  }
  */

  /** Replace this string value with a given string value */
  inline String& operator=( const String& str )
  {
    value_ = str.value_;
    return *this;
  }
  /** Replace this string value with a given string value */
  inline String& operator=( const UnicodeString& str )
  {
    value_ = str;
    return *this;
  }

  /** Appends given string to this one */
  inline String& operator+=( const String& str ) {
    value_ += str.value_;
    return *this;
  }
  /** Appends given string to this one */
  inline String& operator+=( const UnicodeString& str ) {
    value_ += str;
    return *this;
  }

  /**
   * Copies the value from the given string into this one
   */
  inline void copyvalue( const String& str ) { value_ = str.value_; }

  //operator const char*() const { return value_.data(); }
  void remove( const UnicodeString& rm );
  virtual bool isTrue() const POL_OVERRIDE { return ! value_.empty(); }
  inline bool empty() const { return value_.empty(); }

public:
  virtual BObjectImp* selfPlusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const Double& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const String& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const ObjArray& objimp ) const POL_OVERRIDE;
  virtual void selfPlusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( Double& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( String& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( ObjArray& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfMinusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const Double& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const String& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const ObjArray& objimp ) const POL_OVERRIDE;
  virtual void selfMinusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( Double& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( String& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( ObjArray& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectRef OperSubscript( const BObject& obj ) POL_OVERRIDE;
  virtual BObjectRef OperMultiSubscript( std::stack<BObjectRef>& indices ) POL_OVERRIDE;
  virtual BObjectRef OperMultiSubscriptAssign( std::stack<BObjectRef>& indices,
                                               BObjectImp* target ) POL_OVERRIDE;

  int find( int begin, const UnicodeString& target );

  UnicodeString::size_type alnumlen() const;
  UnicodeString::size_type SafeCharAmt() const;

  void reverse();

  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) POL_OVERRIDE;

  //virtual void printOn( std::ostream& ) const POL_OVERRIDE;

protected:
  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual bool operator<( const BObjectImp& objimp ) const POL_OVERRIDE;

  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) POL_OVERRIDE;
  virtual BObjectImp* call_method_id( const int id, Executor& ex,
                                      bool forcebuiltin = false ) POL_OVERRIDE;

// ------------------- INTERNAL ----------------------------------------------
private:
  /** The internal storage string */
  UnicodeString value_;
  String* midstring( size_t begin, size_t len ) const;
};

}
}
