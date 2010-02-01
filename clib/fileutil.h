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
long filesize(const char *fname);
unsigned long GetFileTimestamp( const char* fname );
void RemoveFile( const string& fname );
std::string FullPath( const char* filename );
std::string GetTrueName( const char* filename );
std::string GetFilePart( const char* filename );
string normalized_dir_form( const string& str );
int make_dir(const char *dir); // recursive

#endif
