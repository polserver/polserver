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
   * @deprecated
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
 * List of encodings handled by POL
 */
enum class StrEncoding
{
  ANSI,
  UTF8,
};


/**
 * Represents a single UTF8 character
 * @author Bodom, 12-08-2015
 */
class Utf8Char
{
  friend class Utf8CharValidator;

public:
  /**
   * Create a NULL utf8 char
   */
  inline Utf8Char() : bytes_('\0') {};
  Utf8Char( const char32_t c );

  u8 getByteLen() const;
  char32_t asUtf32() const;
  char16_t asUtf16( const bool failsafe = false ) const;
  char asAnsi( const bool failsafe = false ) const;

  /** returns reference to internal bytes */
  inline const std::string& bytes() const { return bytes_; };

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
   * Appends given char to given string as UTF8 bytes
   *
   * @patam str std::string, the string to append to
   * @param c the char
   * @return number of appended bytes
   */
  inline static u8 appendToStringAsUtf8( std::string& str, const char c )
  {
    u8 bl = Utf8Char::getCharByteLen(c);

    if ( bl == 1 ) {
      str.push_back(c);
      return 1;
    }

    // up to 11 bits, 2 byte long result
    str.push_back(static_cast<char>(0xC0 | ( c >> 6 ))); // first byte: 110XXXXX
    str.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // second byte: 10XXXXXX
    return 2;
  };

protected:
  inline bool operator==( const char c ) const { return bytes_.size() == 1 && bytes_[0] == c; };

private:
  std::string bytes_;
};

/**
 * Validates an utf8 multi-byte character
 * Usage: Instantiate it, then add bytes until the char is complete or you get an error,
 *        call getChar() to read your byte or just use getNextByteFromStr()
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

  static Utf8Char getNextCharFromStr(const char*& str, bool& error);
  static u8 getByteCountFromFirstByte(const char byte);
  /** Returns true is first char is an UTF8 first byte */
  inline static bool isFirstByte(const char byte) { return ( byte & 0x80 ) == 0; };

private:
  /** The character being validated */
  Utf8Char char_;
  /** Number of bytes to expect next */
  u8 bytesNext_;
  /** True if the byte is completed */
  bool completed_;
};


/**
 * Iterator for UnicodeString
 *
 * Anything different than ++ is expensive
 */
class UnicodeStringIterator
{
private:
  typedef std::string::const_iterator internal_iter_type;
  /** The parent iterator */
  internal_iter_type itr_;
  /** The string position in characters */
  size_t pos_;

  /** Increment this by one, used in the ++ operator */
  inline void inc() {
    u8 cnt = Utf8CharValidator::getByteCountFromFirstByte(*itr_);
    if ( cnt ) {
      itr_ += cnt;
      pos_++;
    }
  };

  /** Decrement this by one, used in the -- operator */
  inline void dec() {
    do
    {
      itr_--;
    } while ( Utf8CharValidator::isFirstByte(*itr_) );
    pos_--;
  };

public:
  inline UnicodeStringIterator( internal_iter_type itr, size_t pos ) : itr_(itr), pos_(pos) {};
  /** Copy constructor */
  inline UnicodeStringIterator( const UnicodeStringIterator& it ) : itr_(it.itr_), pos_(it.pos_) {};

  inline bool operator==( const UnicodeStringIterator& it ) const {
    return itr_ == it.itr_;
  };
  inline bool operator!=( const UnicodeStringIterator& it ) const {
    return itr_ != it.itr_;
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
  inline UnicodeStringIterator operator+( size_t n ) {
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
  inline UnicodeStringIterator operator-( size_t n ) {
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

  /**
   * Returns currently pointed char
   */
  inline const UnicodeStringIterator& operator*() const {
    return *this;
  };
  /**
   * Returns currently pointed char
   */
  inline const Utf8Char* operator->() const {
    return &(this->get());
  }
  /**
   * Returns currently pointed char
   */
  inline const Utf8Char get() const {
    const char* ptr = itr_._Ptr; // Create a copy because it will be modified
    bool error; // Unchecked because internal UnicodeString value is trusted
    return Utf8CharValidator::getNextCharFromStr(ptr, error);
  };

  /** Returns current position, in chars */
  inline size_t pos() const { return pos_; };
};


/**
 * Represents an Unicode String
 *
 * Uses s standard string in utf8 for internal storage.
 * Why utf8? http://utf8everywhere.org/
 *
 * This is intended to be "transparently" compatible to ANSI, by simply replacing
 * non-ansi chars with a question mark '?'.
 *
 * To avoid confusion, some constructors are named.
 * Default constructors and operators are referred to ANSI strings.
 *
 * @author Bodom, 12-08-2015
 */
class UnicodeString
{
  //friend class UnicodeStringIterator;

private:
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
  /** Constructs from a single char */
  inline UnicodeString( const Utf8Char& chr )
    : value_(chr.bytes()), length_(1), ascii_(chr.bytes().size() == 1) {};

  UnicodeString( const char32_t chr );
  UnicodeString( const StrEncoding enc, const char* str );
  UnicodeString( const StrEncoding enc, const std::string& str );
  UnicodeString( const StrEncoding enc, const std::string& str, size_t pos, size_t len = npos );

  //static UnicodeString fromUtf8( const char* str );
  //static UnicodeString fromUtf8( const std::string& str );

  //explicit UnicodeString( const char* str );
  //explicit UnicodeString( const std::string& str );
  //explicit UnicodeString( const UnicodeString& s, size_type pos, size_type n ) : base(s, pos, n) {};
  //explicit UnicodeString( const std::string& s, size_type pos, size_type n )
  //  : base(UnicodeString(s), pos, n) {};

  //   ------------------------------- OPERATORS ----------------------------------------------

  inline UnicodeString& operator=( const UnicodeString& s ) {
    this->value_ = s.value_;
    this->length_ = s.length_;
    this->ascii_ = s.ascii_;
    return *this;
  };
  inline UnicodeString& operator=( const Utf8Char& c ) {
    this->value_ = c.bytes();
    this->length_ = 1;
    this->ascii_ = ( c.bytes().size() == 1 );
    return *this;
  };
  inline UnicodeString& operator=( const char32_t c ) {
    (*this) = Utf8Char(c);
    return *this;
  };

  inline UnicodeString& operator+=( const UnicodeString& s ) {
    this->value_ += s.value_;
    this->length_ += s.length_;
    this->ascii_ = this->ascii_ && s.ascii_;
    return *this;
  };
  inline UnicodeString& operator+=( const Utf8Char& c ) {
    this->value_ += c.bytes();
    this->length_++;
    this->ascii_ = this->ascii_ && ( c.bytes().size() == 1 );
    return *this;
  };
  inline UnicodeString& operator+=( const char32_t c ) {
    (*this) += Utf8Char(c);
    return *this;
  };

  inline UnicodeString operator+( const UnicodeString& s ) const {
    UnicodeString res(*this);
    res += s;
    return res;
  };
  inline UnicodeString operator+( const Utf8Char& c ) const {
    UnicodeString res(*this);
    res += c;
    return res;
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
  inline Utf8Char operator[]( size_t pos) const {
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
    return this->assign(enc, s.c_str());
  };
  UnicodeString& append( const StrEncoding enc, const char* s );
  inline UnicodeString& append( const StrEncoding enc, const std::string s ) {
    return this->append(enc, s.c_str());
  };
  UnicodeString sum( const StrEncoding enc, const char* s ) const;
  inline UnicodeString sum( const StrEncoding enc, const std::string s ) const {
    return this->sum(enc, s.c_str());
  };

  //   ------------------------------- ITERATING ----------------------------------------------

  inline const_iterator begin() const { return const_iterator(value_.begin(), 0); };
  inline const_iterator end() const { return const_iterator(value_.end(), length_); };

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
  UnicodeString replace( size_t pos, size_t len, const UnicodeString& str );

  //   ------------------------------- MISCELLANEOUS ------------------------------------------
  void toLower();
  void toUpper();

  const wchar_t* asWcharArray() const;
  bool asAnsi( std::string* outStr ) const;
  std::string asAnsi( const bool failsafe = false ) const;
  /**
   * Returns internal utf8-encoded representation of this object
   */
  inline const std::string& utf8() const { return value_; };

  size_t sizeEstimate() const;
};

}
}
