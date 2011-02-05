#ifndef CLIB_FILECONT_H
#define CLIB_FILECONT_H

#include <string>

class FileContents
{
public:
    FileContents( const char* filname );

    const char* contents() const;
    void set_contents( const std::string& str );
private:
    std::string contents_;
};

#endif
