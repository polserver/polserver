/** @file
 *
 * @par History
 * - 2015/08/12 Bodom:     created this file, added Unicode and UTF8 classes
 */


#include "unicode.h"


namespace Pol {
  namespace Clib {
    /**
     * Create a NULL utf8 char
     */
    Utf8Char::Utf8Char()
    {
      bytes_.push_back(0);
    }

    /**
     * Create an utf8 char from a char32_t
     *
     * @param c the char, in the range 0x000000 - 0x10FFFFFF
     * @throws UnicodeCastFailedException
     */
    Utf8Char::Utf8Char( const char32_t c )
    {
      // Code could be ugly writte in this way, but avoiding loops to make it faster

      if( c < 0x80 )
      {
        // up to 7 bits, ASCII-compatibility, 1 byte long result
        bytes_.push_back(static_cast<const char>(c));
        return;
      }
      else if( c < 0x7FF )
      {
        // up to 11 bits, 2 byte long result
        bytes_.push_back(static_cast<char>(0xC0 | ( c >> 6 ))); // first byte: 110XXXXX
        bytes_.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // second byte: 10XXXXXX
        return;
      }
      else if( c < 0xFFFF )
      {
        // up to 16 bits, 3 byte long result
        bytes_.push_back(static_cast<char>(0xE0 | ( c >> 12 ))); // first byte: 1110XXXX
        bytes_.push_back(static_cast<char>(0x80 | ( (c >> 6) & 0x3f ))); // second byte: 10XXXXXX
        bytes_.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // third byte:  10XXXXXX
        return;
      }
      else if( c < 0x10FFFF )
      {
        // up to 21 bits, 4 byte long result
        bytes_.push_back(static_cast<char>(0xF0 | ( c >> 18 ))); // first byte: 11110XXX
        bytes_.push_back(static_cast<char>(0x80 | ( (c >> 12) & 0x3f ))); // second byte: 10XXXXXX
        bytes_.push_back(static_cast<char>(0x80 | ( (c >> 6) & 0x3f ))); // third byte: 10XXXXXX
        bytes_.push_back(static_cast<char>(0x80 | ( c & 0x3f ))); // fourth byte:  10XXXXXX
        return;
      }

      // Out of range
      throw UnicodeCastFailedException();
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
      for( u8 i = 7; i >= 0; i-- )
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
     * @param failsafe When true, will replace invalid chars with 0xFFFD replacement character instead of throwing
     * @return The char value
     * @throws UnicodeCastFailedException
     */
    char16_t Utf8Char::asUtf16( const bool failsafe ) const
    {
      char32_t out = asUtf32();
      if( out > 0xFFFF )
        if( failsafe )
          return 0xFFFD;
        else
          throw UnicodeCastFailedException();
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
      if( out > 0xFF )
        if( failsafe )
          return '?';
        else
          throw UnicodeCastFailedException();
      return static_cast<char>(out);
    }

    /**
     * Returns internal bytes for direct access
     */
    const Utf8Char::Utf8Bytes* Utf8Char::getBytes() const
    {
      return &bytes_;
    }


    /**
     * Creates a new char from a given char
     */
    UnicodeChar::UnicodeChar( const char16_t c ) :
    val_( c )
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
      * Returns an utf8-encoded representation of this Unicode char
      */
     std::string UnicodeChar::asUtf8() const
     {
       auto bytes = Utf8Char( val_ ).getBytes();
       return std::string( bytes->begin(), bytes->end() );
     }

    /**
     * Returns an ANSI representation of this utf8 char, when possible
     *
     * @param failsafe When true, will replace invalid chars with "?" instead of returning 0
     * @return The char value, 0 when the char has no ANSI representation
     */
    char UnicodeChar::asAnsi( const bool failsafe ) const
    {
      if( val_ > 0xFF )
        if( failsafe )
          return '?';
        else
          return 0;
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
     * Makes an ANSI string representation of this object, when possible
     *
     * @param outStr pointer to the string object that will be appended with the unicode output
     *               when an error is encountered, the string will be only partially filled.
     * @return TRUE on success, FALSE when non ANSI-compatible characters were encountered.
     */
    bool UnicodeString::asAnsi( std::string* outStr ) const
    {
      for( auto it = begin(); it != end(); ++it )
      {
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

      for( auto it = begin(); it != end(); ++it )
      {
        char c = it->asAnsi(failsafe);
        if( c )
          ret += c;
        else
          throw UnicodeCastFailedException();
      }

      return ret;
    }

    /**
     * Returns an utf8-encoded representation of this object
     */
    std::string UnicodeString::asUtf8() const
    {
      std::string ret;

      for( auto it = begin(); it != end(); ++it )
        ret += it->asUtf8();

      return ret;
    }

    /**
     * In-place convert this string to lowercase
     */
    void UnicodeString::toLower()
    {
      for( auto itr = this->begin(); itr != this->end(); ++itr )
        itr->toLower();
    }

    /**lowercase in-place
     * In-place convert this string to uppercase
     */
    void UnicodeString::toUpper()
    {
      for( auto itr = this->begin(); itr != this->end(); ++itr )
        itr->toUpper();
    }

    /**
     * Returns an esteem of the amount of memory currently allocated by this object
     */
    size_t UnicodeString::sizeEstimate() const
    {
      return capacity() * sizeof(char16_t);
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

      if( bytesNext_ == 0 )
      {
        // this is the first byte
        if( static_cast<const unsigned char>(byte) < 0x80 )
        {
          // this is a single byte character, nothing special to do
          char_.bytes_.push_back(byte);
          completed_ = true;
          return AddByteResult::DONE;
        }
        else
        {
          // this is the start of a multibyte utf8 char
          // number of most significant bits set to 1 tells how many bytes is the
          // character composed in total (first byte is this one)
          for( u8 i = 6; i >= 0; i-- )
            if( byte & ( 1 << i ) )
              bytesNext_++;
            else
              break;
           if( bytesNext_ < 1 || bytesNext_ > 3 )
            return AddByteResult::INVALID;

          char_.bytes_.push_back(byte);
          return AddByteResult::MORE;
        }
      } else {
        // waiting for next byte in a multibyte char, it must be 10XXXXXX
        if( ( byte & 0xC0 ) != 0x80 )
          return AddByteResult::INVALID;

        bytesNext_--;
        char_.bytes_.push_back(byte);

        if( bytesNext_ == 0 )
        {
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

  }
}
