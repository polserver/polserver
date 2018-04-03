/** @file
 *
 * @par History
 * - 2015/20/12 Bodom:     creating this file by copy/pasting impstr.h (shame on me)
 *
 * @note This file uses UTF8 encoding
 */


#pragma once

#include "../clib/unicode.h"
#include "basestr.h"

namespace Pol {
namespace Bscript {

using Clib::UnicodeString;
using Clib::UnicodeChar;

/**
 * Represents an Unicode string. Like a string, but can hold unicode characters like
 * ö or 本. Valid chars are limited to 0x0000 to 0xFFFF, the same subset of the whole
 * unicode space that is supported by UO. Uses UnicodeString as internal storage.
 * This is intended to be "transparently" compatible to String, by simply replacing
 * non-ansi chars with a question mark '?'.
 * bscript ID: OTUnicode
 *
 * @see UnicodeString
 */
class Unicode : public BaseStringTpl<UnicodeString, Unicode, BObjectImp::BObjectType::OTUnicode>
{
public:

// ---------------------- INHERITED CONSTRUCTORS ------------------------
  // TODO: just inherit constructors the "using" way when upgrading to Visual Studio 2015,
  //       since Visual Studio 2013 does not support the following syntax:
  // using BaseStringTpl::BaseStringTpl;
  inline Unicode() : BaseStringTpl() {}
  inline Unicode( const Unicode& str ) : BaseStringTpl( str ) {}
  inline explicit Unicode( BObjectImp& objimp ) : BaseStringTpl( objimp ) {}
  inline explicit Unicode( const UnicodeString& str ) : BaseStringTpl( str ) {}
  inline explicit Unicode( const UnicodeString& str,
    UnicodeString::size_type pos, UnicodeString::size_type n ) : BaseStringTpl( str, pos, n ) {}
  inline explicit Unicode( const Unicode& left, const Unicode& right )
    : BaseStringTpl( left, right ) {}
  // TODO: end of manually inherited constructors

// ---------------------- PERSONALIZED EXTRA CONSTRUCTORS ---------------
  Unicode( const UnicodeChar* str, int nchars );

// ---------------------- PACK/UNPACK STUFF -----------------------------
  static BObjectImp* unpack( const char* pstr );
  static BObjectImp* unpack( std::istream& is );
  virtual std::string pack() const POL_OVERRIDE;
  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
  static void packonto( std::ostream& os, const UnicodeString& value );

// -------------------------- MISC ----------------------------------------
  virtual size_t sizeEstimate() const POL_OVERRIDE;

  void toUpper( void );
  void toLower( void );

  inline unsigned long intval() const { return wcstoul( value_.asWcharArray(), NULL, 0 ); }
  inline double dblval() const { return wcstod( value_.asWcharArray(), NULL ); }

  inline virtual std::string getStringRep() const POL_OVERRIDE { return value_.asAnsi(true); }
  inline virtual std::string getFormattedStringRep() const POL_OVERRIDE
  {
    return "\"" + value_.asAnsi(true) + "\"";
  }
  virtual void printOn(std::ostream& os) const;

};

}
}
