/** @file
 *
 * Generic unicode handling library, to be used in any project
 *
 * @par History
 * - 2015/08/12 Bodom:     created this file, added Unicode and UTF8 classes
 */


#pragma once

#include <string>
#include <vector>
#include "rawtypes.h"
#include "passert.h"


namespace Pol {
namespace Clib {

/**
 * Trown when conversion fails
 * @author Bodom, 12-08-2015
 */
class UnicodeCastFailedException: public std::exception
{
};

/**
 * Represents a single POL Unicode Character, in the range 0x0000 - 0xFFFF
 * @author Bodom, 12-08-2015
 */
class UnicodeChar
{
public:
  /**
   * This trivial default constructible
   * @warning Using this constructor leaves the internal character container uninitialized
   */
  UnicodeChar() = default;
  UnicodeChar( const char16_t c );

  //   ------------------------------- OPERATORS ----------------------------------------------
  inline bool operator==( const char16_t c ) const { return val_ == c; };
  inline bool operator==( const UnicodeChar &c ) const { return val_ == c.val_; };
  inline bool operator!=( const char16_t c ) const { return val_ != c; };
  inline bool operator!=( const UnicodeChar &c ) const { return val_ != c.val_; };
  inline bool operator<=( const UnicodeChar &c ) const { return val_ <= c.val_; };
  inline bool operator>=( const UnicodeChar &c ) const { return val_ >= c.val_; };
  inline bool operator<( const UnicodeChar &c ) const { return val_ < c.val_; };
  inline bool operator>( const UnicodeChar &c ) const { return val_ > c.val_; };
  inline UnicodeChar& operator+=( const UnicodeChar &c ) { val_ += c.val_; return *this; };
  inline UnicodeChar& operator-=( const UnicodeChar &c ) { val_ -= c.val_; return *this; };

  // Automagically cast to safe char types... is this dangerous?
  inline operator char32_t() const { return val_; };
  inline operator char16_t() const { return val_; };
  inline operator wchar_t() const { return val_; };

  u8 getByteLen() const;
  std::string asUtf8() const;
  char asAnsi( const bool failsafe = false ) const;

  /** Tells wether this is a space character */
  inline bool isSpace() const { return isspace(val_) == 0; };
  /** Tells wether this is a digit character */
  inline bool isDigit() const { return isdigit(val_) == 0; };
  /** Tells wether this is an alphabetic character */
  inline bool isAlpha() const { return isalpha(val_) == 0; };

  void toLower();
  void toUpper();

private:
  char16_t val_;
};

/**
 * Provides traits for the UnicodeChar
 */
class UnicodeCharTraits : public std::char_traits<UnicodeChar> {
};

/**
 * Represents an Unicode String
 *
 * Uses utf16 for internal storage.
 * Why utf16?
 * + it takes no more space than utf8 (storing a variable number of chars
 *   int utf8 requires at least one extra byte for the null terminator)
 * + it adds way less overhead than utf8 (conversion from char16_t to char
 *   is easy, while utf8 decoding involves bit computations that are
 *   expensive in C and, even if written in ASM, will be way more complicated)
 * + utf32 is not needed since the UO protocol itself is limited to utf16
 *   (the extra storage space will be rarely used for characters that cannot
 *   be represented anyway)
 * @author Bodom, 12-08-2015
 */
class UnicodeString : public std::basic_string<UnicodeChar, UnicodeCharTraits>
{
private:
  typedef std::basic_string<UnicodeChar, UnicodeCharTraits> base;

public:
  // TODO: use ("using") all base class constructor on VS2015
  UnicodeString() { };
  UnicodeString( const char* s ) { *this += s; };
  UnicodeString( const std::string& s ) { *this += s; };
  explicit UnicodeString( const UnicodeString& s, size_type pos, size_type n ) : base(s, pos, n) {};
  explicit UnicodeString( const std::string& s, size_type pos, size_type n )
    : base(UnicodeString(s), pos, n) {};

  //   ------------------------------- OPERATORS ----------------------------------------------
  inline UnicodeString& operator=( const char16_t c )
  {
    UnicodeChar uc = UnicodeChar(c);
    assign(&uc); return *this;
  };

  inline UnicodeString& operator+=( const char16_t c )
  {
    UnicodeChar uc = UnicodeChar(c);
    append(&uc);
    return *this;
  };

  inline UnicodeString& operator=( const UnicodeChar& c )
  {
    assign(&c);
    return *this;
  };

  inline UnicodeString& operator+=( const UnicodeChar& c )
  {
    append(&c);
    return *this;
  };

  inline UnicodeString& operator=( const UnicodeString& s )
  {
    assign(s);
    return *this;
  };

  inline UnicodeString& operator+=( const UnicodeString& s )
  {
    append(s);
    return *this;
  };

  inline UnicodeString& operator=( const char* s )
  {
    while( *s++ != '\0' )
    {
      UnicodeChar uc = UnicodeChar(*s);
      assign(&uc);
    }
    return *this;
  };

  inline UnicodeString& operator+=( const char* s )
  {
    while( *s++ != '\0' )
    {
      UnicodeChar uc = UnicodeChar(*s);
      append(&uc);
    }
    return *this;
  };

  inline UnicodeString operator+( const char16_t c ) const
  {
    UnicodeString res(*this);
    res += c;
    return res;
  };

  inline UnicodeString operator+( const UnicodeChar& c ) const
  {
    UnicodeString res(*this);
    res += c;
    return res;
  };

  inline UnicodeString operator+( const UnicodeString& s ) const
  {
    UnicodeString res(*this);
    res += s;
    return res;
  };

  inline UnicodeString operator+( const char* s ) const
  {
    UnicodeString res(*this);
    res += s;
    return res;
  };

  inline bool UnicodeString::operator==( const char16_t c ) const
  {
    return size() == 1 && front() == c;
  };

  inline bool UnicodeString::operator!=( const char16_t c ) const
  {
    return size() != 1 || front() != c;
  };

  /**
   * Converts to an ANSI string, same as calling Unicode.asAnsi(true)
   *
   * @warning converison may lead to loss of precision
   */
  explicit operator std::string() const { return asAnsi(true); };

  //   ------------------------------- REDEFINED BASE CLASS FUNCTIONS -------------------------
  inline UnicodeString substr( size_t pos = 0, size_t len = npos ) { return substr(pos, len); };


  //   ------------------------------- MISCELLANEOUS ------------------------------------------
  void toLower();
  void toUpper();

  const wchar_t* asWcharArray() const;
  bool asAnsi( std::string* outStr ) const;
  std::string asAnsi( const bool failsafe = false ) const;
  std::string asUtf8() const;

  size_t sizeEstimate() const;
};

/**
 * Represents a single UTF8 character
 * @author Bodom, 12-08-2015
 */
class Utf8Char
{
friend class Utf8CharValidator;

public:
  typedef std::vector<char> Utf8Bytes;

  Utf8Char();
  Utf8Char( const char32_t c );

  u8 getByteLen() const;
  char32_t asUtf32() const;
  char16_t asUtf16( const bool failsafe = false ) const;
  char asAnsi( const bool failsafe = false ) const;

  const Utf8Bytes* getBytes() const;

protected:
  inline bool operator==( const char c ) const { return bytes_.size() == 1 && bytes_[0] == c; };

private:
  Utf8Bytes bytes_;
};

/**
 * Validates an utf8 multi-byte character
 * Usage: Instantiate it, then add bytes until the char is complete or you get an error,
 *        call getChar() to read your byte
 * @author Bodom, 12-08-2015
 */
class Utf8CharValidator
{
public:
  enum class AddByteResult
  {
    DONE,
    INVALID,
    MORE
  };

  Utf8CharValidator();
  AddByteResult addByte(const char byte);
  Utf8Char getChar();
  void reset();

private:
  /** The character being validated */
  Utf8Char char_;
  /** Number of bytes to expect next */
  u8 bytesNext_;
  /** True if the byte is completed */
  bool completed_;
};

}
}
