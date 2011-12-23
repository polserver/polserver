/*
History
=======

Notes
=======

*/

#ifndef DUALBUF_H
#define DUALBUF_H

#include <iostream>

class dualbuf : public std::streambuf
{
public:
    dualbuf( std::streambuf* buf1, std::streambuf* buf2 );
    dualbuf();
    virtual ~dualbuf();

    void setbufs( std::streambuf* buf1, std::streambuf* buf2 );
    void install( std::ostream* os );
    void uninstall();

    virtual int overflow( int c );
    virtual int sync();
    virtual std::streamsize xsputn( const char *str, int size );
private:
    std::streambuf* buf1_;
    std::streambuf* buf2_;

    std::ostream* installed_ostream_;
    std::streambuf* saved_rdbuf_;
};

#endif
