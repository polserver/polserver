/** @file
 *
 * @par History
 */


#include "filecont.h"

#include "logfacility.h"
#include "strutil.h"
#include <cstdio>

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
FileContents::FileContents( const char* filename, bool suppress_error_print )
{
  FILE* fp = fopen( filename, "rb" );
  if ( fp == nullptr )
  {
    if ( !suppress_error_print )
      ERROR_PRINTLN( "Unable to open '{}' for reading.", filename );
    throw std::runtime_error( "Error opening file" );
  }

  char buf[1024];
  bool bom_handled( false );
  while ( !ferror( fp ) && !feof( fp ) )
  {
    size_t nread = fread( buf, 1, sizeof buf, fp );
    if ( nread )
      contents_.append( buf, nread );
    if ( !bom_handled && contents_.size() >= 3 )
    {
      bom_handled = true;
      remove_bom( &contents_ );
    }
  }
  fclose( fp );
}

/**
 * Returns a pointer to the file content
 */
const char* FileContents::contents() const
{
  return contents_.c_str();
}

/**
 * Replaces the internal content with a copy of the given one
 */
void FileContents::set_contents( const std::string& str )
{
  contents_ = str;
}
}  // namespace Clib
}  // namespace Pol
