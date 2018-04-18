/** @file
 *
 * @par History
 * - 2015/08/12 Bodom:     created this file, added Unicode and UTF8 classes
 */


#include "unicode.h"


namespace Pol {
namespace Clib {

// ------------------------------ Utf8Util ------------------------------------


/**
 * Reads next complete character from string pointer
 *
 * @param str The char pointer to read from, will advance it to the last byte
 *            of read char
 * @param bool Error: will set this to true on error
 * @return next valid full UtfChar, or UNICODE_REPL on error
 */
UnicodeChar Utf8Util::getNextCharFromStrAndAdvancePointer(const char*& str, bool& error)
{
  error = false;

  Utf8CharValidator val = Utf8CharValidator();
  Utf8CharValidator::AddByteResult res = val.addByte(*str);
  str++;

  while ( res == Utf8CharValidator::AddByteResult::MORE ) {
    res = val.addByte(*str);
    str++;
  }

  if ( res == Utf8CharValidator::AddByteResult::DONE )
    return val.getChar();

  error = true;
  return UnicodeChar(UNICODE_REPL);
}

/** @see getNextCharFromStr(char, bool) */
UnicodeChar Utf8Util::getNextCharFromStrAndAdvancePointer(const char*& str) {
  bool error;
  return getNextCharFromStrAndAdvancePointer(str, error);
};

/**
 * Parses given first utf8 byte to return total number of utf8 bytes in this char
 * (including the given one)
 *
 * @return 1-4 on success, 0 on failure
 */
inline u8 Utf8Util::getByteCountFromFirstByte(const char byte) {
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

/**
  * Returns the length of given character when represented in UTF8
  *
  * @return 1 to 4 or 0 if out of range
  */
inline u8 Utf8Util::getCharByteLen( const char32_t c )
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
inline u8 Utf8Util::getCharByteLen( const char16_t c )
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
u8 Utf8Util::appendToStringAsUtf8( std::string& str, const char32_t c )
{
  u8 bl = Utf8Util::getCharByteLen(c);

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
u8 Utf8Util::appendToStringAsUtf8( std::string& str, const char16_t c )
{
  u8 bl = Utf8Util::getCharByteLen(c);

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


// ------------------------------ Utf8CharRef ---------------------------------

/** Constructs the char from given string
  *
  * @param str The referenced string as bytes
  * @param posb The referenced 0-based starting position inside the string, in bytes
  * @param posc The referenced 0-based starting position inside the string, in chars
  */
Utf8CharRef::Utf8CharRef( const UnicodeString& str, size_t posb, size_t posc )
  : str_(&str), posb_(posb), posc_(posc)
{
  updateLen();
}

/** Returns currently pointed first char */
const char* Utf8CharRef::fc() const {
  return str_->value_.c_str() + posb_;
}

/** Updates the len_ based on current pos */
inline void Utf8CharRef::updateLen() {

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if( str_->ascii_ ) {
    len_ = 1;
    return;
  }
#endif

  len_ = Utf8Util::getByteCountFromFirstByte( *fc() );
  passert_always_r( len_ > 0 && len_ < 5,
    "Bug in Utf8CharRef::updateLen(), please report this bug on the forums." );
}

/** Returns 0-pos based byte insde this character */
inline char Utf8CharRef::getByteAt(u8 idx) const {
  passert_always_r( idx < len_,
    "Bug in Utf8CharRef::getByteAt(), please report this bug on the forums." );
  return str_->value_[posb_ + idx];
}

 /**
 * Returns an UTF32 representation of this utf8 char
 *
 * @return The u32 value
 */
char32_t Utf8CharRef::asChar32() const
{
  if( len_ == 1 )
    return getByteAt(0);

  char32_t out = 0;
  // Copy bits from first byte
  for( s8 i = 7-1-len_; i >= 0; i-- )
    if( getByteAt(0) & ( 1 << i ) )
      out |= 1 << ( (6 * ( len_ - 1 ) ) + i );
  // Copy bits from remaining bytes
  for( u8 bn = 1; bn < len_; bn++ )
    for( s8 i = 5; i >= 0; i-- )
      if( getByteAt(bn) & ( 1 << i ) )
        out |= 1 << ( (6 * ( len_ - bn - 1)) + i );

  return out;
}

/**
 * Returns an UO UTF16 representation of this utf8 char, when possible
 *
 * @param failsafe When true, will replace invalid chars with
 *                 UNICODE_REPL replacement character instead of throwing
 * @return The char value
 * @throws UnicodeCastFailedException
 */
char16_t Utf8CharRef::asChar16( const bool failsafe ) const
{
  char32_t out = asChar32();
  if( out > 0xFFFF ) {
    if( failsafe )
      return UNICODE_REPL;
    else
      throw UnicodeCastFailedException();
  }
  return static_cast<char16_t>(out);
}

/**
 * Returns an ANSI representation of this utf8 char, when possible
 *
 * @param failsafe When true, will replace invalid chars with ASCII_REPL instead of throwing
 * @return The char value
 * @throws UnicodeCastFailedException
 */
char Utf8CharRef::asAnsi( const bool failsafe ) const
{
  char32_t out = asChar32();
  if( out > 0xFF ) {
    if( failsafe )
      return ASCII_REPL;
    else
      throw UnicodeCastFailedException();
  }
  return static_cast<char>(out);
}

/**
 * Returns an ASCII representation of this utf8 char, when possible
 *
 * @param failsafe When true, will replace invalid chars with ASCII_REPL instead of throwing
 * @return The char value
 * @throws UnicodeCastFailedException
 */
char Utf8CharRef::asAscii( const bool failsafe ) const
{
  char32_t out = asChar32();
  if( out > 0x7F ) {
    if( failsafe )
      return ASCII_REPL;
    else
      throw UnicodeCastFailedException();
  }
  return static_cast<char>(out);
}


// -------------------------- UnicodeStringIterator ---------------------------

/**
 * Increment this by one, used in the ++ operator
 */
void UnicodeStringIterator::inc() {
  if ( ref_.len_ == 0 )
    throw std::runtime_error("Incrementing invalid len UnicodeStringIterator");

  if ( ref_.posc_ == ref_.str_->lengthc() )
    throw std::runtime_error("Incrementing UnicodeStringIterator over null terminator");

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ref_.str_->isAscii() ) {
    ++ref_.posc_;
    ++ref_.posb_;
    return;
  }
#endif

  ++ref_.posc_;
  ref_.posb_ += ref_.len_;
  ref_.updateLen();
};

/**
 * Decrement this by one, used in the -- operator
 */
void UnicodeStringIterator::dec() {
  if ( ref_.len_ == 0 )
    throw std::runtime_error("Decrementing invalid len UnicodeStringIterator");

  if ( ref_.posc_ == 0 )
    throw std::runtime_error("Decrementing UnicodeStringIterator before string start");

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ref_.str_->isAscii() ) {
    --ref_.posc_;
    --ref_.posb_;
    return;
  }
#endif

  do
  {
    --ref_.posb_;
  } while ( ! Utf8Util::isFirstByte(*ptr()) );

  --ref_.posc_;
  ref_.updateLen();
};


// ------------------------------ UnicodeString -------------------------------


/**
 * Constructs from a simple char
 */
UnicodeString::UnicodeString( const char32_t chr )
  : UnicodeString()
{
  (*this) += chr;
}

/**
 * Constructs from a UO-UTF16 string
 */
UnicodeString::UnicodeString( const std::u16string& str )
  : UnicodeString()
{
  this->append(str);
}

/**
 * Constructs from a UTF32 string
 */
UnicodeString::UnicodeString( const std::u32string& str )
  : UnicodeString()
{
  this->append(str);
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
 * Constructs by taking a given amount of bytes from char pointer
 */
UnicodeString::UnicodeString( const StrEncoding enc, const char* str, size_t nbytes )
  : UnicodeString()
{
  this->reserveb(nbytes);
  this->append(enc, std::string(str, nbytes));
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
  * Sets this string from the content of UO-UTF16 string
  */
UnicodeString& UnicodeString::assign( const std::u16string& str )
{
  this->clear();
  this->append(str);
  return *this;
}

/**
  * Sets this string from the content of UTF32 string
  */
UnicodeString& UnicodeString::assign( const std::u32string& str )
{
  this->clear();
  this->append(str);
  return *this;
}

/**
 * Appends a C string to this. Replace sinvalid bytes with UNICODE_REPL
 */
UnicodeString& UnicodeString::append( const StrEncoding enc, const char* s )
{
  switch ( enc )
  {
  case StrEncoding::ANSI:
    while ( *s++ != '\0' ) {
      u8 bl = Utf8Util::appendToStringAsUtf8( value_, *s );
      this->length_++;
      if ( bl > 1 )
        this->ascii_ = false;
    }
    break;

  case StrEncoding::UTF8:
    while ( *s != '\0' ) {
      UnicodeChar uc = Utf8Util::getNextCharFromStrAndAdvancePointer(s);

      this->value_ += uc.value_;
      this->length_++;
      this->ascii_ = this->ascii_ && uc.ascii_;
    }
    break;
  }

  return *this;
}

/**
 * Appends a UO-UTF16 string
 */
UnicodeString& UnicodeString::append( const std::u16string& str )
{
  for ( const auto& chr : str ) {
    u8 bl = Utf8Util::appendToStringAsUtf8( value_, chr );
    this->length_++;
    if ( bl > 1 )
      this->ascii_ = false;
  }

  return *this;
}

/**
 * Appends a UO-UTF32 string
 */
UnicodeString& UnicodeString::append( const std::u32string& str )
{
  for ( const auto& chr : str ) {
    u8 bl = Utf8Util::appendToStringAsUtf8( value_, chr );
    this->length_++;
    if ( bl > 1 )
      this->ascii_ = false;
  }

  return *this;
}

/**
  * Appends an ANSI string to a copy of this string
  */
UnicodeString UnicodeString::concat( const StrEncoding enc, const char* s ) const
{
  UnicodeString res(*this);
  res.append(enc, s);
  return res;
}

/**
  * Appends an UO-UTF16 string to a copy of this string
  */
UnicodeString UnicodeString::concat( const std::u16string& str ) const
{
  UnicodeString res(*this);
  res.append(str);
  return res;
}

/**
  * Appends a UTF32 string to a copy of this string
  */
UnicodeString UnicodeString::concat( const std::u32string& str ) const
{
  UnicodeString res(*this);
  res.append(str);
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
#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ ) {
    *outStr += value_;
    return true;
  }
#endif

  for( auto it : *this ) {
    char c = it.asAnsi();
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
#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ )
    return value_;
#endif

  std::string ret;

  for( auto it : *this ) {
    char c = it.asAnsi(failsafe);
    if( c )
      ret += c;
    else
      throw UnicodeCastFailedException();
  }

  return ret;
}

/**
 * Returns an ASCII string representation of this object
 *
 * @param failsafe When true, will replace invalid chars with "?" instead of throwing
 * @return the ASCII string
 * @throws CastFailedException
 */
std::string UnicodeString::asAscii( const bool failsafe ) const
{
#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ )
    return value_;
#endif

  std::string ret;

  for( auto it : *this ) {
    char c = it.asAscii(failsafe);
    if( c )
      ret += c;
    else
      throw UnicodeCastFailedException();
  }

  return ret;
}

/**
 * Returns a copy of this object encoded as a string of char32
 */
std::u32string UnicodeString::asChar32String() const
{
  std::u32string ret;
  ret.reserve(length_);

  for ( const auto& chr: *this )
    ret += chr.asChar32();

  return ret;
}

/**
 * In-place convert this string to lowercase
 */
void UnicodeString::toLower()
{
  UnicodeString res;

  for ( const auto& chr : *this )
    res += UnicodeData::toLower( chr.asChar32() );

  *this = res;
}

/**
 * In-place convert this string to uppercase
 */
void UnicodeString::toUpper()
{
  UnicodeString res;

  for ( const auto& chr : *this )
    res += UnicodeData::toUpper( chr.asChar32() );

  *this = res;
}

/**
 * @see std::string::find_first_of
 */
size_t UnicodeString::find_first_of( const UnicodeString& list, size_t pos) const
{
  if ( pos >= length_ )
    return npos;

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ && list.ascii_ )
    return value_.find_first_of( list.value_, pos );
#endif

  for ( auto mine = this->begin() + pos; mine != this->end(); ++mine ) {
    for ( const auto& comp : list )
      if ( *mine == comp )
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

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ && list.ascii_ )
    return value_.find_first_not_of( list.value_, pos );
#endif

  for ( auto mine = this->begin() + pos; mine != this->end(); ++mine ) {
    bool found = false;

    for ( const auto& comp : list )
      if ( *mine == comp ) {
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

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ && list.ascii_ )
    return value_.find_last_of( list.value_, pos );
#endif

  for ( auto mine = this->begin() + pos; ; --mine ) {
    for ( const auto& comp : list )
      if ( *mine == comp )
        return pos;

    pos--;
    if ( mine.posc() == 0 )
      break;
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
    pos = length_ - 1 ;

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ && list.ascii_ )
    return value_.find_last_not_of( list.value_, pos );
#endif

  for ( auto mine = this->begin() + pos; ; --mine ) {
    bool found = false;

    for ( const auto& comp : list )
      if ( *mine == comp ) {
        found = true;
        break;
      }

    if ( ! found )
      return pos;

    pos--;
    if ( mine.posc() == 0 )
      break;
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
      *this = this->substr( startpos );
    else
      value_.clear();
  }

  if ( type == TrimTypes::TRIM_RIGHT || type == TrimTypes::TRIM_BOTH ) {
    // Find the first character position from reverse
    size_t endpos = this->find_last_not_of( crSet, npos );
    if ( npos != endpos )
      *this = this->substr( 0, endpos + 1 );
    else
      value_.clear();
  }
}

/**
 * Like string::find but position is in characters
 *
 * @see string::find
 */
size_t UnicodeString::find( const UnicodeString& str, size_t pos ) const
{
  if ( pos >= length_ )
    return npos;

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ && str.ascii_ )
    return value_.find(str.value_, pos);
#endif

  auto it = this->begin() + pos;
  size_t fposb = value_.find(str.value_, it.posb());

  if ( fposb == npos )
    return npos;

  // Convert byte-based position to char-based position
  for ( ; it != this->end(); ++it )
    if ( it.posb() == fposb )
      return it.posc();

  passert_always_r(false, "BUG in UnicodeString::find(): Couldn't convert posb to posc");
}

/**
 * Like string::find but position is in characters
 *
 * @see string::find
 */
size_t UnicodeString::find( const char32_t chr, size_t pos ) const
{
  return this->find(UnicodeChar(chr), pos);
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
    for ( int i = 0; i < it->getByteLen(); ++i )
      reversed.append(it->fc(), it->getByteLen());
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
  if ( pos >= length_ )
    throw std::out_of_range("Substr out of range");

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ )
    return UnsafeConstructFromAscii( value_.substr( pos, len ) );
#endif

  UnicodeString ret;
  if ( pos == length_ || len == 0)
    return ret;

  // When len is too big, do not reserve too much
  ret.value_.reserve( std::min(len, value_.size()) );

  for ( const auto& it : *this ) {
    if ( it.posc() < pos )
      continue;

    ret += it;

    len--;
    if ( len == 0 )
      break;
  }

  return ret;
}

/**
 * @see std::string::replace
 */
UnicodeString& UnicodeString::replace( size_t pos, size_t len, const UnicodeString& str )
{
  if ( pos > length_ )
    throw std::out_of_range("Replace pos out of range");

#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ && str.ascii_ ) {
    value_.replace( pos, len, str.value_ );
    return *this;
  }
#endif

  UnicodeString res = UnicodeString();
  for ( const auto& it : *this ) {
    if ( it.posc() < pos ) {
      // Replacement not started yet
      res += it;
      continue;
    }
    if ( it.posc() == pos ) {
      // Replacement point reached, paste the whole replacement here
      res += str;
      continue;
    }
    if ( it.posc() - pos < len ) {
      // Ignoring characters being replaced
      continue;
    }
    // replacement done, copying the rest from old string
    res += it;
  }

  *this = res;
  return *this;
}

/** @see std::string::compare */
int UnicodeString::compare( const UnicodeString& str ) const
{
  return this->value_.compare(str.value_);
}
/** @see std::string::compare */
int UnicodeString::compare( size_t pos, size_t len, const UnicodeString& str ) const
{
#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ && str.ascii_ )
    return value_.compare( pos, len, str.value_ );
#endif

  return this->substr(pos, len).compare( str );
}
/** @see std::string::compare */
int UnicodeString::compare( size_t pos, size_t len, const UnicodeString& str, size_t subpos,
  size_t sublen ) const
{
#if UNICODE_ENABLE_ASCII_OPTIMIZATIONS
  if ( ascii_ && str.ascii_ )
    return value_.compare( pos, len, str.value_, subpos, sublen );
#endif

  return this->substr(pos, len).compare( str.substr(subpos, sublen) );
}

/**
 * Returns an esteem of the amount of memory currently allocated by this object
 */
size_t UnicodeString::sizeEstimate() const
{
  return value_.capacity() * sizeof(char) + sizeof(size_t) + sizeof(bool);
}


// ------------------------------ Utf8CharValidator ---------------------------


Utf8CharValidator::Utf8CharValidator()
{
  reset();
}

/**
 * Resets the validator to its initial state, preparing to read next byte
 */
void Utf8CharValidator::reset()
{
  char_.clear();
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
    u8 totBytes = Utf8Util::getByteCountFromFirstByte(byte);
    if( totBytes == 1 ) {
      // this is a single byte character, nothing special to do
      char_.push_back(byte);
      completed_ = true;
      return AddByteResult::DONE;
    } else if( totBytes == 0 ) {
      // Start char is invalid
      return AddByteResult::INVALID;
    } else {
      // this is the start of a multibyte utf8 char
      bytesNext_ = totBytes - 1;
      char_.push_back(byte);
      return AddByteResult::MORE;
    }
  } else {
    // waiting for next byte in a multibyte char, it must be 10XXXXXX
    if( ( byte & 0xC0 ) != 0x80 )
      return AddByteResult::INVALID;

    bytesNext_--;
    char_.push_back(byte);

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
UnicodeChar Utf8CharValidator::getChar()
{
  if( ! completed_ )
    throw std::runtime_error("Trying to read incomplete char");

  return UnicodeChar(char_);
}

}
}
