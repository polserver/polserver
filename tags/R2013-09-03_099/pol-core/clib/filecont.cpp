/*
History
=======

Notes
=======

*/

#include "stl_inc.h"

#include <stdio.h>

#include "filecont.h"

FileContents::FileContents( const char* filename )
{
    FILE *fp = fopen( filename, "rb" );
    if (fp == NULL)
    {
        cerr << "Unable to open '" << filename << "' for reading." << endl;
        throw runtime_error( "Error opening file" );
    }

    char buf[1024];
    while (!ferror(fp) && !feof(fp))
    {
        size_t nread = fread( buf, 1, sizeof buf, fp );
        if (nread)
            contents_.append( buf, nread );
    }

    fclose(fp);
}

const char* FileContents::contents() const
{
    return contents_.c_str();
}

void FileContents::set_contents( const string& str )
{
    contents_ = str;
}

