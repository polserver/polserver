/** @file
 *
 * @par History
 */


#ifndef CLIB_FILEUTIL_H
#define CLIB_FILEUTIL_H

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>


namespace Pol::Clib
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
std::string normalized_dir_form( const std::string& str );
int make_dir( const char* dir );  // recursive
int strip_one( std::string& direc );
// Reads from the current file position to EOF in one fread. Meant for
// bulk-loading whole (or whole-remaining) files instead of many small
// fseek/fread pairs.
[[nodiscard]] std::vector<std::byte> ReadEntireFile( FILE* fp );
}  // namespace Pol::Clib

#endif
