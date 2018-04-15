/** @file
 *
 * @par History
 */


#include "filecont.h"

#include <cstdio>

#include "logfacility.h"

namespace Pol
{
namespace Clib
{
/**
 * Creates the instance and reads the whole file content into it
 *
 * @param filename Full path of the file to be read
 * @throws std::runtime_error When something goes wrong when reading the file
 */
FileContents::FileContents( const char* filename )
{
  FILE* fp = fopen( filename, "rb" );
  if ( fp == NULL )
  {
    ERROR_PRINT << "Unable to open '" << filename << "' for reading.\n";
    throw std::runtime_error( "Error opening file" );
  }

  char buf[1024];
  size_t pos = 0;
  while ( !ferror( fp ) && !feof( fp ) )
  {
    size_t nread = fread( buf, 1, sizeof buf, fp );
    if ( nread ) {
      std::string utf8;
      utf8.reserve(nread);
      Utf8CharValidator val = Utf8CharValidator();

      for ( size_t i = 0; i < nread; ++i ) {
        ++pos;

        auto res = val.addByte(buf[i]);
        while ( res == Utf8CharValidator::AddByteResult::MORE && i < nread ) {
          ++i;
          ++pos;
          res = val.addByte(buf[i]);
        }

        if ( res != Utf8CharValidator::AddByteResult::DONE ) {
          ERROR_PRINT << "Error reading '" << filename << "': invalid utf8 byte at pos " << pos << ".\n";
          throw std::runtime_error( "Error opening file" );
        }

        contents_ += val.getChar();
      }
    }
  }

  fclose( fp );
}

/**
 * Returns a reference to the internal content
 */
const UnicodeString& FileContents::contents() const
{
  return contents_;
}

/**
 * Replaces the internal content with a copy of the given one
 */
void FileContents::set_contents( const UnicodeString& str )
{
  contents_ = str;
}

}
}
