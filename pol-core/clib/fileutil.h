/*
History
=======

Notes
=======

*/

#ifndef CLIB_FILEUTIL_H
#define CLIB_FILEUTIL_H

bool IsDirectory( const char* dir );
void MakeDirectory( const char* dir );
bool FileExists( const char* filename );
bool FileExists( const std::string& filename );
int filesize(const char *fname);
unsigned int GetFileTimestamp( const char* fname );
void RemoveFile( const std::string& fname );
std::string FullPath( const char* filename );
std::string GetTrueName( const char* filename );
std::string GetFilePart( const char* filename );
std::string normalized_dir_form( const std::string& str );
int make_dir(const char *dir); // recursive

#endif
