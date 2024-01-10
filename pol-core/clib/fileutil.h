/** @file
 *
 * @par History
 */


#ifndef CLIB_FILEUTIL_H
#define CLIB_FILEUTIL_H

#include <string>

namespace Pol
{
namespace Clib
{
bool IsDirectory( const char* dir );
void MakeDirectory( const char* dir );
bool FileExists( const char* filename );
bool FileExists( const std::string& filename );
int filesize( const char* fname );
unsigned int GetFileTimestamp( const char* fname );
void RemoveFile( const std::string& fname );
std::string FullPath( const char* filename );
std::string GetTrueName( const char* filename );
std::string GetFilePart( const char* filename );
std::string normalized_dir_form( const std::string& str, const std::string& base = "" );
int make_dir( const char* dir );  // recursive
int strip_one( std::string& direc );
}
}
#endif
