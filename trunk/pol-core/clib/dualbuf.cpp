/*
History
=======

Notes
=======

*/

#include "stl_inc.h"
#include "dualbuf.h"

dualbuf::dualbuf( std::streambuf* buf1, std::streambuf* buf2 ) :
    buf1_(buf1),
    buf2_(buf2),
    installed_ostream_(NULL),
    saved_rdbuf_(NULL)
{
}

dualbuf::dualbuf() :
    buf1_(NULL),
    buf2_(NULL),
    installed_ostream_(NULL),
    saved_rdbuf_(NULL)
{
}

dualbuf::~dualbuf()
{
    uninstall();
}

void dualbuf::setbufs( std::streambuf* buf1, std::streambuf* buf2 )
{
    buf1_ = buf1;
    buf2_ = buf2;
}

void dualbuf::install( std::ostream* os )
{
    uninstall();
    
    saved_rdbuf_ = os->rdbuf();
    os->rdbuf( this );
    installed_ostream_ = os;
}

void dualbuf::uninstall()
{
    if (installed_ostream_ != NULL)
    {
        installed_ostream_->rdbuf( saved_rdbuf_ );

        installed_ostream_ = NULL;
        saved_rdbuf_ = NULL;
    }

}

int dualbuf::overflow( int c )
{
    if (c != EOF)
    {
        int r1 = buf1_->sputc(c);
        int r2 = buf2_->sputc(c);
        if ((r1 == EOF) && ( r2== EOF))
            return EOF;
        else
            return c;
    }
    else
    {
        return EOF == 0?1:0;
    }
}

int dualbuf::sync()
{
    int r1 = buf1_->pubsync();
    int r2 = buf2_->pubsync();
    if (r1 == EOF && r2 == EOF)
        return EOF;
    else
        return EOF == 0?1:0;
}

std::streamsize dualbuf::xsputn( const char* str, streamsize size )
{
    std::streamsize n1 = buf1_->sputn( str, size);
    std::streamsize n2 = buf2_->sputn( str, size );
    return (n1>n2) ? n1 : n2;
}
