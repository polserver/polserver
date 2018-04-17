/** @file
 *
 * Generic unicode handling library, to be used in any project
 *
 * @warning This source file must be utf8-encoded
 * @par History
 * - 2015/08/12 Bodom:     created this file, added Unicode and UTF8 classes
 */

#pragma once

#if __cplusplus < 201103L
//  Strings must be sequential or most of this will not work
//  #error C++11 or better is required!
#endif


#include <string>
#include <vector>
#include "rawtypes.h"
#include "passert.h"

//TODO: Implement it
/// If defined, will enable optimizations while handling ASCII-compatible strings
#define UNICODE_ENABLE_ASCII_OPTIMIZATIONS 1

/// The Unicode replacement character '�'
#define UNICODE_REPL static_cast<char32_t>(0xFFFD)
// The Unicode replacement character encoded as UTF8 string
#define UTF8_REPL u8"�"
/// The ASCII replacemente character
#define ASCII_REPL '?'

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
 * List of encodings handled by POL
 */
enum class StrEncoding
{
  ANSI,
  UTF8,
};


// Forward declarations
class UnicodeString;
class UnicodeChar;


/**
 * Collections of static UTF8 utils
 */
class Utf8Util
{
public:

  static UnicodeChar getNextCharFromStrAndAdvancePointer(const char*& str, bool& error);
  static UnicodeChar getNextCharFromStrAndAdvancePointer(const char*& str);

  static u8 getByteCountFromFirstByte(const char byte);
  /** Returns true is first char is an UTF8 first byte */
  inline static bool isFirstByte(const char byte) { return ( byte & 0x80 ) == 0; };

  static u8 getCharByteLen( const char32_t c );
  static u8 getCharByteLen( const char16_t c );
  /**
   * Returns the length of given character when represented in UTF8
   *
   * @return 1 to 2
   */
  inline static u8 getCharByteLen( const char c ) {
    if( c & 0x80 ) {
      // up to 7 bits, ASCII-compatibility, 1 byte long result
      return 1;
    }
    // up to 11 bits, 2 byte long result
    return 2;
  };

  static u8 appendToStringAsUtf8( std::string& str, const char32_t c );
  static u8 appendToStringAsUtf8( std::string& str, const char16_t c );
  /**
   * Appends given ANSI char to given string as UTF8 bytes
   *
   * @patam str std::string, the string to append to
   * @param c the ANSI char
   * @return number of appended bytes
   */
  inline static u8 appendToStringAsUtf8( std::string& str, const char c )
  {
    u8 bl = getCharByteLen(c);

    if ( bl == 1 ) {
      str.push_back(c);
      return 1;
    }

    // up to 11 bits, 2 byte long result
    str.push_back(static_cast<char>(0xC0 | ( c >> 6 ))); // first byte: 110XXXXX
    str.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // second byte: 10XXXXXX
    return 2;
  };
};

/**
 * Represents a single UTF8 character inside a string
 * @author Bodom, 12-08-2015
 */
class Utf8CharRef
{
  friend class Utf8CharValidator;
  friend class UnicodeString;
  friend class UnicodeStringIterator;

private:
  /** This constructor is private because it is unsafe */
  Utf8CharRef( const UnicodeString& str, size_t pos );

  const char* fc() const;
  void updateLen();
  char getByteAt(u8 idx) const;

public:

  /* Returns number of bytes forming by this utf8 char (1 to 4) */
  inline u8 getByteLen() const { return len_; };
  char32_t asChar32() const;
  char16_t asChar16( const bool failsafe = true ) const;
  char asAnsi( const bool failsafe = true ) const;
  char asAscii( const bool failsafe = true ) const;

  /** Tells wether this is a space character */
  inline bool isSpace() const { return isspace(asChar32()) != 0; };
  /** Tells wether this is a digit character */
  inline bool isDigit() const { return isdigit(asChar32()) != 0; };
  /** Tells wether this is an alphabetic character */
  inline bool isAlpha() const { return isalpha(asChar32()) != 0; };
  /** Tells whether this is an alphanumeric cracater */
  inline bool isAlNum() const { return isalnum(asChar32()) != 0; };

  /** Returns current position, inside the string, chars */
  inline size_t pos() const { return pos_; };
  /** Returns a reference to the whole string */
  inline const UnicodeString& str() const { return str_; };

  inline bool operator==( const char32_t c ) const { return asChar32() == c; };
  inline bool operator!=( const char32_t c ) const { return asChar32() != c; };
  inline bool operator>( const char32_t c ) const { return asChar32() > c; };
  inline bool operator<( const char32_t c ) const { return asChar32() < c; };
  inline bool operator>=( const char32_t c ) const { return asChar32() >= c; };
  inline bool operator<=( const char32_t c ) const { return asChar32() <= c; };

  inline void operator=( const Utf8CharRef& other ) {
    //TODO: Remove the assertion and support the case?
    passert_always_r( &(str_) == &(other.str_), "Cannot assign reference to a different string" );
    pos_ = other.pos_;
    len_ = other.len_;
  };

  /** This is true if non-zero */
  inline explicit operator bool() const { return this->asChar32() != 0; };

private:
  const UnicodeString& str_;
  size_t pos_;
  u8 len_;
};

/**
 * Iterator for UnicodeString
 *
 * Anything different than ++ is expensive
 */
class UnicodeStringIterator
{
  friend class Utf8CharRef;

private:
  /** The string position in characters */
  size_t posc_;
  /** The internal character reference */
  Utf8CharRef ref_;

  inline void inc();
  inline void dec();

public:
  /**
   * Constructs the iterator from the given string
   *
   * @param str Reference to the string being iterated
   * @param cpos Current string position in characters
   * @param bpos Current string position in bytes
   */
  inline UnicodeStringIterator( const UnicodeString& str, size_t cpos, size_t bpos )
    : posc_(cpos), ref_(str, bpos) {};
  /** Copy constructor */
  inline UnicodeStringIterator( const UnicodeStringIterator& it )
    : posc_(it.posc_), ref_(it.ref_) {};

  inline bool operator==( const UnicodeStringIterator& it ) const {
    return ref_.fc() == it.ref_.fc();
  };
  inline bool operator!=( const UnicodeStringIterator& it ) const {
    return ref_.fc() != it.ref_.fc();
  };
  inline void operator=( const UnicodeStringIterator& it ) {
    posc_ = it.posc_;
    ref_ = it.ref_;
  };
  /** ++this */
  inline UnicodeStringIterator& operator++() {
    this->inc();
    return *this;
  };
  /** this++ */
  inline UnicodeStringIterator operator++( int ) {
    UnicodeStringIterator original = UnicodeStringIterator(*this);
    this->inc();
    return original;
  };
  inline UnicodeStringIterator operator+( size_t n ) const {
    // TODO: implement it in a better way? Is there any?
    UnicodeStringIterator other = UnicodeStringIterator(*this);
    for ( n; n > 0; --n )
      ++other;
    return other;
  };
  inline UnicodeStringIterator& operator+=( size_t n ) {
    // TODO: implement it in a better way? Is there any?
    for ( n; n > 0; --n )
      ++(*this);
    return *this;
  };
  /** --this */
  inline UnicodeStringIterator& operator--() {
    this->dec();
    return *this;
  };
  /** this-- */
  inline UnicodeStringIterator operator--( int ) {
    UnicodeStringIterator original = UnicodeStringIterator(*this);
    this->dec();
    return original;
  };
  inline UnicodeStringIterator operator-( size_t n ) const {
    // TODO: implement it in a better way? Is there any?
    UnicodeStringIterator other = UnicodeStringIterator(*this);
    for ( n; n > 0; --n )
      --other;
    return other;
  };
  inline UnicodeStringIterator& operator-=( size_t n ) {
    // TODO: implement it in a better way? Is there any?
    for ( n; n > 0; --n )
      --(*this);
    return *this;
  };
  /** Returns a Utf8CharRef to nth position */
  inline const Utf8CharRef operator[]( size_t n ) const {
    auto op = *this + n;
    return op.get();
  }

  /**
   * Returns the string being iterated
   */
  inline const UnicodeString& str() const {
    return ref_.str();
  }
  /**
   * Returns currently pointed char
   */
  inline const Utf8CharRef operator*() const {
    return this->get();
  }
  /**
   * Returns currently pointed char
   */
  inline const Utf8CharRef* operator->() const {
    return &ref_;
  }
  /**
   * Returns currently pointed char
   */
  inline const Utf8CharRef get() const {
    return ref_;
  };
  /**
   * Returns a pointer to current char in string
   */
  inline const char* ptr() const {
    return ref_.fc();
  }

  /** Returns current position, in chars */
  inline size_t posc() const { return posc_; };

  /** Returns a copy of this iterator */
  inline UnicodeStringIterator copy() const { return UnicodeStringIterator(*this); };
};


/**
 * Represents an Unicode String
 *
 * Uses s standard string in utf8 for internal storage.
 * Why utf8? http://utf8everywhere.org/
 *
 * Default constructors assume the source strings to be in UTF8
 * Specifying a StrEncoding may help clarifing doubtful situations
 *
 * This can provide "transparent" compatibility to ANSI, by simply replacing
 * non-ansi chars with a question mark '?'.
 *
 * To avoid confusion, some constructors are named.
 *
 * @author Bodom, 12-08-2015
 */
class UnicodeString
{
  //friend class UnicodeStringIterator;
  friend class Utf8CharRef;

protected:
  /** The internally stored value, as UTF8 */
  std::string value_;
  /** The length, in characters */
  size_t length_;
  /** Tells if this is verified ASCII-compatible, enables some optimizatins */
  bool ascii_;

public:
  typedef std::string::size_type size_type;
  typedef UnicodeStringIterator const_iterator;

  static const size_t npos = std::string::npos;

  /**
   * Types for the trim() method
   */
  enum class TrimTypes
  {
    TRIM_LEFT,
    TRIM_RIGHT,
    TRIM_BOTH
  };

  /** Constructs an empty string */
  inline UnicodeString() : value_(""), length_(0), ascii_(true) {};
  /** Copy constrctor */
  inline UnicodeString( const UnicodeString& str )
    : value_(str.value_), length_(str.length_), ascii_(str.ascii_) {};
  /** Constructs from a single char in a string */
  inline UnicodeString( const Utf8CharRef& chr )
    : value_(""), length_(1), ascii_(chr.getByteLen() == 1)
  {
    for ( u8 i = 0; i < chr.getByteLen(); ++i )
      value_ += chr.getByteAt(i);
  };

  UnicodeString( const char32_t chr );
  UnicodeString( const StrEncoding enc, const char* str );
  UnicodeString( const StrEncoding enc, const char* str, size_t nbytes );
  UnicodeString( const StrEncoding enc, const std::string& str );
  UnicodeString( const StrEncoding enc, const std::string& str, size_t pos, size_t len = npos );

  //   ------------------------------ BACKWARD-COMPATIBILITY CONSTRUCTORS --------------------

  inline UnicodeString( const char* str ) : UnicodeString( StrEncoding::UTF8, str ) {};
  inline UnicodeString( const std::string& str ) : UnicodeString( StrEncoding::UTF8, str ) {};
  inline UnicodeString( const char* str, size_t nbytes )
    : UnicodeString( StrEncoding::UTF8, str, nbytes ) {};

  //   ------------------------------- OPERATORS ----------------------------------------------

  inline UnicodeString& operator=( const UnicodeString& s ) {
    this->value_ = s.value_;
    this->length_ = s.length_;
    this->ascii_ = s.ascii_;
    return *this;
  };
  inline UnicodeString& operator=( const Utf8CharRef& c ) {
    this->value_.assign(c.fc(), c.getByteLen());
    this->length_ = 1;
    this->ascii_ = ( c.getByteLen() == 1 );
    return *this;
  };
  inline UnicodeString& operator=( const UnicodeStringIterator& c ) {
    return *this = *c;
  };
  inline UnicodeString& operator=( const char32_t c ) {
    (*this) = UnicodeString(c);
    return *this;
  };

  inline UnicodeString& operator+=( const UnicodeString& s ) {
    this->value_ += s.value_;
    this->length_ += s.length_;
    this->ascii_ = this->ascii_ && s.ascii_;
    return *this;
  };
  inline UnicodeString& operator+=( const Utf8CharRef& c ) {
    this->value_.append(c.fc(), c.getByteLen());
    this->length_ += 1;
    this->ascii_ = this->ascii_ && ( c.getByteLen() == 1 );
    return *this;
  };
  inline UnicodeString& operator+=( const UnicodeStringIterator& c ) {
    return *this += *c;
  };
  inline UnicodeString& operator+=( const char32_t c ) {
    u8 len = Utf8Util::appendToStringAsUtf8(this->value_, c);
    this->length_ += 1;
    this->ascii_ = this->ascii_ && ( len == 1 );
    return *this;
  };

  inline UnicodeString operator+( const UnicodeString& s ) const {
    UnicodeString res(*this);
    res += s;
    return res;
  };
  inline UnicodeString operator+( const Utf8CharRef& c ) const {
    UnicodeString res(*this);
    res += c;
    return res;
  };
  inline UnicodeString operator+( const UnicodeStringIterator& c ) const {
    return *this + *c;
  };
  inline UnicodeString operator+( const char32_t c ) const {
    UnicodeString res(*this);
    res += c;
    return res;
  };

  inline bool operator==( const UnicodeString& s ) const {
    return this->value_ == s.value_;
  }
  inline bool operator!=( const UnicodeString& s ) const {
    return this->value_ != s.value_;
  }
  inline bool operator>( const UnicodeString& s ) const {
    return this->value_ > s.value_;
  }
  inline bool operator<( const UnicodeString& s ) const {
    return this->value_ < s.value_;
  }
  inline bool operator>=( const UnicodeString& s ) const {
    return this->value_ >= s.value_;
  }
  inline bool operator<=( const UnicodeString& s ) const {
    return this->value_ <= s.value_;
  }
  /**
   * Returns an iterator to nth char of this string
   */
  inline Utf8CharRef operator[]( size_t pos) const {
    passert_always_r( pos < length_, "Requesting iterator over string end" );
    UnicodeStringIterator it = this->begin();
    it += pos;
    return it.get();
  }

  /**
   * Converts to an ANSI string, same as calling Unicode.asAnsi(true)
   *
   * @warning converison may lead to loss of precision
   */
  //explicit operator std::string() const { return asAnsi(true); };

  //   ------------------------------- PSEUDO-OPERATORS ---------------------------------------

  UnicodeString& assign( const StrEncoding enc, const char* s );
  inline UnicodeString& assign( const StrEncoding enc, const std::string s ) {
    this->reserveb(s.size());
    return this->assign(enc, s.c_str());
  };
  UnicodeString& append( const StrEncoding enc, const char* s );
  inline UnicodeString& append( const StrEncoding enc, const std::string s ) {
    this->reserveb(value_.size() + s.size());
    return this->append(enc, s.c_str());
  };
  UnicodeString concat( const StrEncoding enc, const char* s ) const;
  inline UnicodeString concat( const StrEncoding enc, const std::string s ) const {
    return this->concat(enc, s.c_str());
  };

  //   ------------------------------- ITERATING ----------------------------------------------

  inline const_iterator begin() const { return const_iterator(*this, 0, 0); };
  inline const_iterator end() const { return const_iterator(*this, length_, value_.size()); };

  //   ------------------------------- REDEFINED BASE CLASS FUNCTIONS -------------------------
  //inline UnicodeString substr( size_t pos = 0, size_t len = npos ) { return substr(pos, len); };

  //   ------------------------------- CAPACITY -----------------------------------------------

  /** Returns length of this string, in characters */
  inline size_t lengthc() const { return length_; };
  /**
   * Returns storage capacity currently allocated for this string, in bytes
   *
   * @see std::string::capacity()
   */
  inline size_t capacityb() const { return value_.capacity(); };
  /**
   * Request a change in capacity, in bytes
   *
   * @see std::string::reserve()
   */
  inline void reserveb( size_t n = 0 ) { value_.reserve(n); };
  /** Erase the content of this string, which becomes empty */
  inline void clear() { value_.clear(); length_ = 0; ascii_ = true; };
  /** Test if string is empty */
  inline bool empty() const { return value_.empty(); };
  /**
   * Requests the string to reduce its capacity to fit its size
   *
   * @see std::string::shrink_to_fit()
   */
  inline void shrink_to_fit() { value_.shrink_to_fit(); };

  //   ------------------------------- STRING OPERATIONS --------------------------------------

  size_t find_first_of( const UnicodeString& list, size_t pos = 0) const;
  size_t find_first_not_of( const UnicodeString& list, size_t pos = 0) const;
  size_t find_last_of( const UnicodeString& list, size_t pos = npos) const;
  size_t find_last_not_of( const UnicodeString& list, size_t pos = npos) const;
  void trim( const UnicodeString& crSet, TrimTypes type );
  size_t find( const UnicodeString& str, size_t pos = 0 ) const;
  size_t find( const char32_t chr, size_t pos = 0 ) const;
  void reverse();
  void remove( const UnicodeString& rm );
  UnicodeString substr( size_t pos = 0, size_t len = npos ) const;
  UnicodeString& replace( size_t pos, size_t len, const UnicodeString& str );
  int compare( const UnicodeString& str ) const;
  int compare( size_t pos, size_t len, const UnicodeString& str ) const;
  int compare( size_t pos, size_t len, const UnicodeString& str, size_t subpos,
    size_t sublen = npos ) const;

  //   ------------------------------- MISCELLANEOUS ------------------------------------------
  void toLower();
  void toUpper();

  bool asAnsi( std::string* outStr ) const;
  std::string asAnsi( const bool failsafe = true ) const;
  std::string asAscii( const bool failsafe = true ) const;
  /** Returns true when this string is ascii-compatible */
  inline bool isAscii() const { return ascii_; };
  /**
   * Returns internal utf8-encoded representation of this object
   */
  inline const std::string& utf8() const { return value_; };

  /** Parses this string, interpreting its content as an integral number */
  inline unsigned long intval() const {
    try {
      return std::stoi(value_);
    } catch ( std::logic_error ) {
      return 0L;
    }
  };

  /** Parses this string, interpreting its content as floating point number */
  inline double dblval() const {
    try {
      return std::stod(value_);
    } catch ( std::logic_error ) {
      return 0.0;
    }
  }

  size_t sizeEstimate() const;
};


/**
 * Actually just a standard UnicodeString with 1-length
 */
class UnicodeChar : public UnicodeString
{
  friend class UnicodeString;
  friend class Utf8CharValidator;

private:
  /**
   * Unsafe/unchecked construct from single-char string
   *
   * @warning do not use! This is meant for Utf8CharValidator only
   */
  inline explicit UnicodeChar( std::string& str ) : UnicodeString() {
    this->value_ = str;
    this->length_ = 1;
    this->ascii_ = ( str.size() == 1 );
  }

public:
  UnicodeChar( const char32_t chr ) : UnicodeString( chr ) {};
  inline operator char32_t() const { return (*this)[0].asChar32(); };
};


/**
 * Validates an utf8 multi-byte character
 * Usage: Instantiate it, then add bytes until the char is complete or you get an error,
 *        call getChar() to read your byte or just use Utf8Util::getNextCharFromStr()
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
  UnicodeChar getChar();
  void reset();

private:
  /** The character being validated */
  std::string char_;
  /** Number of bytes to expect next */
  u8 bytesNext_;
  /** True if the byte is completed */
  bool completed_;
};


}
}
