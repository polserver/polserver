/*
History
=======
2006/09/16 Shinigami: fixed Memory Overwrite Bug in convertArrayToUC

Notes
=======

*/

#ifndef CLIB_UNICODE_H
#define CLIB_UNICODE_H

#include <string>


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../pol/utype.h"
namespace Pol {
  namespace Clib {
	//std::string toascii( const std::wstring& wstr );
	//std::wstring tounicode( const std::string& str );

	void unicode_copy_string( wchar_t* dst, size_t dstsize, const char* src );

	bool convertArrayToUC( Bscript::ObjArray*& in_text, u16* out_wtext,
						   size_t textlen, bool ConvToBE = false, bool nullterm = true );
	bool convertUCtoArray( const u16* in_wtext, Bscript::ObjArray*& out_text,
						   size_t textlen, bool ConvFromBE = false );

  }

  /**
   * Trown when conversion fails
   */
  class UnicodeCastFailedException: public std::exception
  {
  };

  /**
   * Represents a single POL Unicode Character, in the range 0x0000 - 0xFFFF
   */
  class UnicodeChar
  {
  public:
    /**
     * This triviall default constructible
     * @warning Using this constructor leaves the internal character container uninitialized
     */
    UnicodeChar() = default;
    UnicodeChar( const char16_t c );

    //   ------------------------------- OPERATORS ----------------------------------------------
    inline bool operator==( const char16_t c ) const { return val_ == c; };
    inline bool operator==( const UnicodeChar &c ) const { return val_ == c.val_; };
    inline bool operator<=( const UnicodeChar &c ) const { return val_ <= c.val_; };
    inline bool operator>=( const UnicodeChar &c ) const { return val_ >= c.val_; };
    inline bool operator<( const UnicodeChar &c ) const { return val_ < c.val_; };
    inline bool operator>( const UnicodeChar &c ) const { return val_ > c.val_; };
    inline UnicodeChar& operator+=( const UnicodeChar &c ) { val_ += c.val_; return *this; };
    inline UnicodeChar& operator-=( const UnicodeChar &c ) { val_ -= c.val_; return *this; };

    //operator char16_t();
    //operator wchar_t();

    u8 getByteLen() const;
    char32_t asUtf32() const;
    /** Returns an UTF16 representation */
    inline char16_t asUtf16( ) const { return val_; };
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
  class Unicode : public std::basic_string<UnicodeChar>
  {
  private:
    typedef std::basic_string<UnicodeChar> base;

  public:
    Unicode() { };
    Unicode( const char* s ) { *this += s; };
    Unicode( const std::string& s ) { *this += s; };
    Unicode( const std::string& s, size_t pos, size_t n ) { base(Unicode(s), pos, n); };

    //   ------------------------------- OPERATORS ----------------------------------------------
    inline Unicode& operator=( const char16_t c )  { UnicodeChar uc = UnicodeChar(c); assign(&uc); return *this; };
    inline Unicode& operator+=( const char16_t c ) { UnicodeChar uc = UnicodeChar(c); append(&uc); return *this; };
    inline Unicode& operator=( const UnicodeChar& c )  { assign(&c); return *this; };
    inline Unicode& operator+=( const UnicodeChar& c ) { append(&c); return *this; };
    inline Unicode& operator=( const Unicode& s )  { assign(s); return *this; };
    inline Unicode& operator+=( const Unicode& s ) { append(s); return *this; };
    inline Unicode& operator=( const char* s )  { while( *s++ != '\0' ){ UnicodeChar uc = UnicodeChar(*s); assign(&uc); } return *this; };
    inline Unicode& operator+=( const char* s ) { while( *s++ != '\0' ){ UnicodeChar uc = UnicodeChar(*s); append(&uc); } return *this; };
    inline Unicode operator+( const char16_t c ) const { Unicode res(*this); res += c; return res; };
    inline Unicode operator+( const UnicodeChar& c ) const { Unicode res(*this); res += c; return res; };
    inline Unicode operator+( const Unicode& s ) const { Unicode res(*this); res += s; return res; };
    inline Unicode operator+( const char* s ) const { Unicode res(*this); res += s; return res; };

    inline bool Unicode::operator==( const char16_t c ) const { return size() == 1 && front() == c; };

    int compare( const char* s ) const;
    int compare( size_t pos, size_t len, const char* s ) const;

    Unicode substr( size_type pos = 0, size_type count = npos ) const;
    void toLower();
    void toUpper();

    const wchar_t* asWcharArray() const;
    bool asAnsi( std::string* outStr ) const;
    std::string asAnsi( const bool failsafe = false ) const;
  };

  /**
   * Represents a single UTF8 character
   */
  class Utf8Char
  {
  friend class Utf8CharValidator;

  public:
    u8 getByteLen() const;
    char32_t asUtf32() const;
    char16_t asUtf16( const bool failsafe = false ) const;
    char asAnsi( const bool failsafe = false ) const;

  protected:
    Utf8Char();
    Utf8Char( const char c );
    inline bool operator==( const char c ) const { return bytes_.size() == 1 && bytes_[0] == c; };

  private:
    std::vector<char> bytes_;
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
    enum AddByteResult
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
#endif
