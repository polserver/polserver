/** @file
 *
 * @par History
 * - 2015/08/12 Bodom:     created this file, added Unicode and UTF8 classes
 */


#include "unicode.h"


namespace Pol {
namespace Clib {

/**
 * Create an utf8 char from a char32_t
 *
 * @param c the char, in the range 0x000000 - 0x10FFFFFF
 * @throws UnicodeCastFailedException
 */
Utf8Char::Utf8Char( const char32_t c )
{
  appendToStringAsUtf8( bytes_, c );
}

/**
  * Returns the length of given character when represented in UTF8
  *
  * @return 1 to 4 or 0 if out of range
  */
u8 Utf8Char::getCharByteLen( const char32_t c )
{
  if( c < 0x80 ) {
    // up to 7 bits, ASCII-compatibility, 1 byte long result
    return 1;
  }
  if( c < 0x800 ) {
    // up to 11 bits, 2 byte long result
    return 2;
  }
  if( c < 0x10000 ) {
    // up to 16 bits, 3 byte long result
    return 3;
  }
  if( c < 0x110000 ) {
    // up to 21 bits, 4 byte long result
    return 4;
  }
  // Out of UTF8 allowed range
  return 0;
}

/**
  * Returns the length of given character when represented in UTF8
  *
  * @return 1 to 3
  */
u8 Utf8Char::getCharByteLen( const char16_t c )
{
  if( c < 0x80 ) {
    // up to 7 bits, ASCII-compatibility, 1 byte long result
    return 1;
  }
  if( c < 0x800 ) {
    // up to 11 bits, 2 byte long result
    return 2;
  }
  // up to 16 bits, 3 byte long result
  return 3;
}

/**
 * Appends given char to given string as UTF8 bytes
 *
 * @patam str std::string, the string to append to
 * @param c the char, in the range 0x000000 - 0x10FFFFFF
 * @throws UnicodeCastFailedException
 * @return number of appended bytes
 */
u8 Utf8Char::appendToStringAsUtf8( std::string& str, const char32_t c )
{
  u8 bl = Utf8Char::getCharByteLen(c);

  switch ( bl ) {
  case 1:
    str.push_back(static_cast<const char>(c));
    return 1;

  case 2:
    // up to 11 bits, 2 byte long result
    str.push_back(static_cast<char>(0xC0 | ( c >> 6 ))); // first byte: 110XXXXX
    str.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // second byte: 10XXXXXX
    return 2;

  case 3:
    // up to 16 bits, 3 byte long result
    str.push_back(static_cast<char>(0xE0 | ( c >> 12 ))); // first byte: 1110XXXX
    str.push_back(static_cast<char>(0x80 | ( (c >> 6) & 0x3f ))); // second byte: 10XXXXXX
    str.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // third byte:  10XXXXXX
    return 3;

  case 4:
    // up to 21 bits, 4 byte long result
    str.push_back(static_cast<char>(0xF0 | ( c >> 18 ))); // first byte: 11110XXX
    str.push_back(static_cast<char>(0x80 | ( (c >> 12) & 0x3f ))); // second byte: 10XXXXXX
    str.push_back(static_cast<char>(0x80 | ( (c >> 6) & 0x3f ))); // third byte: 10XXXXXX
    str.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // fourth byte:  10XXXXXX
    return 4;
  }

  // Out of range
  throw UnicodeCastFailedException();
}

/**
 * Appends given char to given string as UTF8 bytes
 *
 * @patam str std::string, the string to append to
 * @param c the char
 * @return number of appended bytes
 */
u8 Utf8Char::appendToStringAsUtf8( std::string& str, const char16_t c )
{
  u8 bl = Utf8Char::getCharByteLen(c);

  switch ( bl ) {
  case 1:
    str.push_back(static_cast<const char>(c));
    return 1;

  case 2:
    // up to 11 bits, 2 byte long result
    str.push_back(static_cast<char>(0xC0 | ( c >> 6 ))); // first byte: 110XXXXX
    str.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // second byte: 10XXXXXX
    return 2;

  case 3:
    // up to 16 bits, 3 byte long result
    str.push_back(static_cast<char>(0xE0 | ( c >> 12 ))); // first byte: 1110XXXX
    str.push_back(static_cast<char>(0x80 | ( (c >> 6) & 0x3f ))); // second byte: 10XXXXXX
    str.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // third byte:  10XXXXXX
    return 3;
  }

  // Must never reach this point
  passert_always_r( false, "Bug in Utf8Char, please report this bug on the forums." );
}

/**
 * Returns number of bytes represented by this utf8 char (1 to 4)
 */
u8 Utf8Char::getByteLen() const
{
  passert_always_r( bytes_.size(), "Empty Utf8Char, please report this bug on the forums." );

  if( static_cast<const unsigned char>(bytes_[0]) < 0x80 )
    return 1;

  u8 ret = 0;
  for( int i = 7; i >= 0; i-- )
    if( bytes_[0] & ( 1 << i ) )
      ret++;
    else
      return ret;

  // Must never reach this point
  passert_always_r( false, "Bug in Utf8Char, please report this bug on the forums." );
}

 /**
 * Returns an UTF32 representation of this utf8 char
 *
 * @return The u32 value
 */
char32_t Utf8Char::asUtf32() const
{
  u8 len = getByteLen();
  if( len <= 1 )
    return bytes_[0];

  char32_t out = 0;
  // Copy bits from first byte
  for( s8 i = 7-1-len; i >= 0; i-- )
    if( bytes_[0] & ( 1 << i ) )
      out |= 1 << ( (6 * ( len - 1 ) ) + i );
  // Copy bits from remaining bytes
  for( u8 bn = 1; bn < len; bn++ )
    for( s8 i = 5; i >= 0; i-- )
      if( bytes_[bn] & ( 1 << i ) )
        out |= 1 << ( (6 * ( len - bn - 1)) + i );

  return out;
}

/**
 * Returns an UTF16 representation of this utf8 char, when possible
 *
 * @param failsafe When true, will replace invalid chars with
 *                 0xFFFD replacement character instead of throwing
 * @return The char value
 * @throws UnicodeCastFailedException
 */
char16_t Utf8Char::asUtf16( const bool failsafe ) const
{
  char32_t out = asUtf32();
  if( out > 0xFFFF ) {
    if( failsafe )
      return 0xFFFD;
    else
      throw UnicodeCastFailedException();
  }
  return static_cast<char16_t>(out);
}

/**
 * Returns an ANSI representation of this utf8 char, when possible
 *
 * @param failsafe When true, will replace invalid chars with "?" instead of throwing
 * @return The char value
 * @throws UnicodeCastFailedException
 */
char Utf8Char::asAnsi( const bool failsafe ) const
{
  char32_t out = asUtf32();
  if( out > 0xFF ) {
    if( failsafe )
      return '?';
    else
      throw UnicodeCastFailedException();
  }
  return static_cast<char>(out);
}

/**
 * Creates a new char from a given char
 */
UnicodeChar::UnicodeChar( const char16_t c ) : val_( c )
{
}

/**
 * Returns minumum number of bytes this char can fit in
 */
u8 UnicodeChar::getByteLen() const
{
  if( val_ > 0xFF )
    return 2;
  return 1;
}

/**
 * Returns an ANSI representation of this utf8 char, when possible
 *
 * @param failsafe When true, will replace invalid chars with "?" instead of returning 0
 * @return The char value, 0 when the char has no ANSI representation
 */
char UnicodeChar::asAnsi( const bool failsafe ) const
{
  if( val_ > 0xFF ) {
    if( failsafe )
      return '?';
    else
      return 0;
  }
  return static_cast<char>(val_);
}

/**
 * Convert this char to lowercase, if possible
 */
void UnicodeChar::toLower()
{
  wchar_t c = towlower(val_);
  if( c > 0 && c <= 0xFFFF )
    val_ = static_cast<char16_t>(c);
}

/**
 * Convert this char to uppercase, if possible
 */
void UnicodeChar::toUpper()
{
  wchar_t c = towupper(val_);
  if( c > 0 && c <= 0xFFFF )
    val_ = static_cast<char16_t>(c);
}

/**
 * Constructs from a simple char
 */
UnicodeString::UnicodeString( const char32_t chr )
  : UnicodeString()
{
  (*this) += chr;
}
/**
 * Constructs from a C string
 */
UnicodeString::UnicodeString( const StrEncoding enc, const char* str )
  : UnicodeString()
{
  this->append(enc, str);
}
/**
 * Constructs from a std string
 */
UnicodeString::UnicodeString( const StrEncoding enc, const std::string& str )
  : UnicodeString()
{
  this->append(enc, str);
}
/**
 * Constructs from part of a std string
 * (string is split indiscriminately: pos and len are interpreted as bytes)
 */
UnicodeString::UnicodeString( const StrEncoding enc, const std::string& str, size_t pos,
  size_t len ) : UnicodeString()
{
  this->append( enc, std::string(str, pos, len) );
}

/**
 * Sets this string from the content of C string
 * Truncates the string if invalid bytes are found
 */
UnicodeString& UnicodeString::assign( const StrEncoding enc, const char* s )
{
  this->clear();
  this->append(enc, s);
  return *this;
}

/**
 * Appends a C string to this. Truncates the string if invalid bytes are found.
 */
UnicodeString& UnicodeString::append( const StrEncoding enc, const char* s )
{
  switch ( enc )
  {
  case StrEncoding::ANSI:
    while ( *s++ != '\0' ) {
      u8 bl = Utf8Char::appendToStringAsUtf8( value_, *s );
      this->length_++;
      if ( bl > 1 )
        this->ascii_ = false;
    }
    break;

  case StrEncoding::UTF8:
    while ( *s++ != '\0' ) {
      bool error;
      Utf8Char uc = Utf8CharValidator::getNextCharFromStr(s, error);

      if (error)
        break;

      this->value_ += uc.bytes();
      this->length_++;
      if (uc.bytes().length() > 1)
        this->ascii_ = false;
    }
    break;
  }

  return *this;
}

/**
  * Appends an ANSI string to a copy of this string
  */
UnicodeString UnicodeString::sum( const StrEncoding enc, const char* s ) const
{
  UnicodeString res(*this);
  res.append(enc, s);
  return res;
}

/**
 * Makes an ANSI string representation of this object, when possible
 *
 * @param outStr pointer to the string object that will be appended with the unicode output
 *               when an error is encountered, the string will be only partially filled.
 * @return TRUE on success, FALSE when non ANSI-compatible characters were encountered.
 */
bool UnicodeString::asAnsi( std::string* outStr ) const
{
  for( auto it : *this ) {
    char c = it->asAnsi();
    if( c )
      *outStr += c;
    else
      return false;
  }
  return true;
}

/**
 * Returns an ANSI string representation of this object
 *
 * @param failsafe When true, will replace invalid chars with "?" instead of throwing
 * @return the ANSI string
 * @throws CastFailedException
 */
std::string UnicodeString::asAnsi( const bool failsafe ) const
{
  std::string ret;

  for( auto it : *this ) {
    char c = it->asAnsi(failsafe);
    if( c )
      ret += c;
    else
      throw UnicodeCastFailedException();
  }

  return ret;
}

/**
 * In-place convert this string to lowercase
 *
 * @todo locale support. Actually only nasic chars are converted
 */
void UnicodeString::toLower()
{
  boost::to_lower(value_);
}

/**
 * In-place convert this string to uppercase
 *
 * @todo locale support. Actually only nasic chars are converted
 */
void UnicodeString::toUpper()
{
  boost::to_upper(value_);
}

/**
 * @see std::string::find_first_of
 */
size_t UnicodeString::find_first_of( const UnicodeString& list, size_t pos) const
{
  if ( pos >= length_ )
    return npos;

  for ( auto mine = this->begin() + pos; mine != this->end(); ++mine ) {
    for ( auto comp = list.begin(); comp != list.end(); ++comp )
      if ( mine == comp )
        return pos;

    pos++;
  }
  return npos;
}

/**
 * @see std::string::find_first_not_of
 */
size_t UnicodeString::find_first_not_of( const UnicodeString& list, size_t pos) const
{
  if ( pos >= length_ )
    return npos;

  for ( auto mine = this->begin() + pos; mine != this->end(); ++mine ) {
    bool found = false;

    for ( auto comp = list.begin(); comp != list.end(); ++comp )
      if ( mine == comp ) {
        found = true;
        break;
      }

    if ( ! found )
      return pos;

    pos++;
  }
  return npos;
}

/**
 * @see std::string::find_last_of
 */
size_t UnicodeString::find_last_of( const UnicodeString& list, size_t pos) const
{
  if ( length_ == 0 )
    return npos;
  if ( pos == npos || pos > length_ )
    pos = length_ -1 ;

  for ( auto mine = this->end() - 1; mine != this->begin(); --mine ) {
    for ( auto comp = list.begin(); comp != list.end(); ++comp )
      if ( mine == comp )
        return pos;

    pos--;
  }
  return npos;
}

/**
 * @see std::string::find_last_not_of
 */
size_t UnicodeString::find_last_not_of( const UnicodeString& list, size_t pos) const
{
  if ( length_ == 0 )
    return npos;
  if ( pos == npos || pos > length_ )
    pos = length_ -1 ;

  for ( auto mine = this->end() - 1; mine != this->begin(); --mine ) {
    bool found = false;

    for ( auto comp = list.begin(); comp != list.end(); ++comp )
      if ( mine == comp ) {
        found = true;
        break;
      }

    if ( ! found )
      return pos;

    pos--;
  }
  return npos;
}

/**
  * Trims this string: removes whitespaces at the beginning/ending/both
  *
  * @param crSet string containing the list of characters to consider whitespace
  * @param type enum, the trim type
  */
void UnicodeString::trim( const UnicodeString& crSet, TrimTypes type )
{
  if ( type == TrimTypes::TRIM_LEFT || type == TrimTypes::TRIM_BOTH ) {
    // Find the first character position after excluding leading blank spaces
    size_t startpos = this->find_first_not_of( crSet, 0 );
    if ( npos != startpos )
      value_ = value_.substr( startpos );
    else
      value_.clear();
  }

  if ( type == TrimTypes::TRIM_RIGHT || type == TrimTypes::TRIM_BOTH ) {
    // Find the first character position from reverse
    size_t endpos = this->find_last_not_of( crSet, npos );
    if ( npos != endpos )
      value_ = value_.substr( 0, endpos + 1 );
    else
      value_.clear();
  }
}

/**
 * @see string::find
 */
size_t UnicodeString::find( const UnicodeString& str, size_t pos ) const
{
  return value_.find(str.value_, pos);
}

/**
 * Like string::find, but needs to know encoding
 */
size_t UnicodeString::find( const char32_t chr, size_t pos ) const
{
  return this->find(Utf8Char(chr), pos);
}

/**
 * Reverse this string inplace
 */
void UnicodeString::reverse()
{
  std::string reversed;
  reversed.reserve(value_.size());

  auto it = this->end();
  do {
    --it;
    reversed += it->bytes();
  } while ( it != this->begin() );

  value_ = reversed;
}

/**
  * Looks for a substring and deletes it if found
  */
void UnicodeString::remove( const UnicodeString& rm )
{
  auto pos = value_.find(rm.value_);
  if (pos != std::string::npos)
    value_.erase( pos, rm.value_.size() );
}

/**
 * @see std::string::substr
 */
UnicodeString UnicodeString::substr( size_t pos, size_t len ) const
{
  if ( pos > length_ )
    throw std::out_of_range("Substr out of range");

  UnicodeString ret = UnicodeString();
  if ( pos == length_ || len == 0)
    return ret;

  bool started = false;
  for ( auto it : *this ) {
    if ( it.pos() >= pos )
      started = true;

    if ( started )
      ret += it.get();

    len--;
    if ( len == 0 )
      break;
  }

  return ret;
}

/**
 * @see std::string::replace
 */
UnicodeString UnicodeString::replace( size_t pos, size_t len, const UnicodeString& str )
{
  if ( pos > length_ )
    throw std::out_of_range("Replace pos out of range");

  UnicodeString res = UnicodeString();
  for ( auto it : *this ) {
    if ( it.pos() < pos ) {
      // Replacement not started yet, copy form old string
      res += it.get();
      continue;
    }
    if ( it.pos() == pos ) {
      // Replacement point reached, paste the whole replacement here
      res += str;
      continue;
    }
    if ( it.pos() - pos < len ) {
      // Ignoring characters being replaced
      continue;
    }
    // replacement done, copying the rest from old string
    res += it.get();
  }

  return res;
}

/**
 * Returns an esteem of the amount of memory currently allocated by this object
 */
size_t UnicodeString::sizeEstimate() const
{
  return value_.capacity() * sizeof(char) + sizeof(size_t) + sizeof(bool);
}


Utf8CharValidator::Utf8CharValidator()
{
  reset();
}

/**
 * Resets the validator to its initial state, preparing to read next byte
 */
void Utf8CharValidator::reset()
{
  char_.bytes_.clear();
  bytesNext_ = 0;
  completed_ = false;
}

/**
 * Adds a byte to the current character
 *
 * @return AddByteResult (INVALID when the byte is not valid,
 *         MORE when the byte is valid, but will need more,
 *         DONE 0 when char is complete)
 */
Utf8CharValidator::AddByteResult Utf8CharValidator::addByte(const char byte)
{
  if( completed_ )
    throw std::runtime_error("Trying to add byte to completed char");

  if( bytesNext_ == 0 ) {
    // this is the first byte
    u8 totBytes = getByteCountFromFirstByte(byte);
    if( totBytes == 1 ) {
      // this is a single byte character, nothing special to do
      char_.bytes_.push_back(byte);
      completed_ = true;
      return AddByteResult::DONE;
    } else if( totBytes == 0 ) {
      // Start char is invalid
      return AddByteResult::INVALID;
    } else {
      // this is the start of a multibyte utf8 char
      bytesNext_ = totBytes - 1;
      char_.bytes_.push_back(byte);
      return AddByteResult::MORE;
    }
  } else {
    // waiting for next byte in a multibyte char, it must be 10XXXXXX
    if( ( byte & 0xC0 ) != 0x80 )
      return AddByteResult::INVALID;

    bytesNext_--;
    char_.bytes_.push_back(byte);

    if( bytesNext_ == 0 ) {
      completed_ = true;
      return AddByteResult::DONE;
    }
    return AddByteResult::MORE;
  }
}

/**
 * Returns the current char, when completed
 */
Utf8Char Utf8CharValidator::getChar()
{
  if( ! completed_ )
    throw std::runtime_error("Trying to read incomplete char");

  return char_;
}

/**
 * Reads next complete character from string pointer
 *
 * @param str The char pointer to read from, will advance it to the last byte
 *            of read char
 * @param bool Error: will set this to true on error
 * @return next valid full UtfChar, or \0 on error
 */
Utf8Char Utf8CharValidator::getNextCharFromStr(const char*& str, bool& error)
{
  error = false;

  if ( *str == '\0' ) {
    error = true;
    return Utf8Char();
  }

  Utf8CharValidator val = Utf8CharValidator();
  Utf8CharValidator::AddByteResult res = val.addByte(*str);

  while ( res == Utf8CharValidator::AddByteResult::MORE ) {
    str++;
    val.addByte(*str);
  }

  if ( res == Utf8CharValidator::AddByteResult::DONE )
    return val.getChar();

  error = true;
  return Utf8Char();
}

/**
 * Parses given first utf8 byte to return total number of utf8 bytes in this char
 * (including the given one)
 *
 * @return 1-4 on success, 0 on failure
 */
u8 Utf8CharValidator::getByteCountFromFirstByte(const char byte) {
  if ( (byte & 0x80) == 0 )    // 0xxxxxxx
    return 1;

  if ( (byte & 0xe0) == 0xc0 ) // 110xxxxx
    return 2;

  if ( (byte & 0xf0) == 0xe0 ) // 1110xxxx
    return 3;

  if ( (byte & 0xf8) == 0xf0 ) // 11110xxx
    return 4;

  return 0; // invalid
}

}
}
