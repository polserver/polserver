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

void FileContents::read()
{
  FILE* fp = fopen( filename_.c_str(), "rb" );
  if ( fp == NULL )
  {
    reportNotFoundError();
    throw std::runtime_error( "Error opening file" );
  }

  char buf[1024];
  size_t pos = 0;
  Utf8CharValidator val = Utf8CharValidator();

  while ( !ferror( fp ) && !feof( fp ) )
  {
    size_t nread = fread( buf, 1, sizeof buf, fp );
    if ( nread ) {
      contents_->reserveb(contents_->utf8().size() + nread);

      for ( size_t i = 0; i < nread; ++i ) {
        ++pos;
        val.reset();

        auto res = val.addByte(buf[i]);
        while ( res == Utf8CharValidator::AddByteResult::MORE && i < nread ) {
          ++i;
          ++pos;
          res = val.addByte(buf[i]);
        }

        if ( res != Utf8CharValidator::AddByteResult::DONE ) {
          ERROR_PRINT << "Error reading '" << filename_ << "': invalid utf8 byte at pos "
            << pos << ".\n";
          throw std::runtime_error( "Error opening file" );
        }

        *contents_ += val.getChar();
      }
    }
  }

  fclose( fp );
}

/**
 * Reports when file has not been found
 */
void SourceFileContents::reportNotFoundError() const
{
  ERROR_PRINT << "Unable to open '" << filename_ << "' for reading.\n";
}

void IncludeFileContents::reportNotFoundError() const
{
  ERROR_PRINT << "Unable to read include file '" << filename_ << "'\n";
}

void ModuleFileContents::reportNotFoundError() const
{
  ERROR_PRINT << "Unable to find module " << modulename_ << "\n"
               << "\t(Filename: " << filename_ << ")\n";
}

/**
 * Returns a reference to the internal content
 */
const std::shared_ptr<UnicodeString>& FileContents::contents() const
{
  return contents_;
}

/**
 * Replaces the internal content with a copy of the given one
 */
void FileContents::set_contents( const UnicodeString& str )
{
  *contents_ = str;
}

}
}
