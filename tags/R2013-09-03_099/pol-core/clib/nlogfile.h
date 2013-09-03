/*
History
=======

Notes
=======

*/

#ifndef NLOGFILE_H
#define NLOGFILE_H

#include <iostream>
#include <fstream>
#include <string>

class timestamp_streambuf : public std::streambuf
{
public:
    timestamp_streambuf( const char* tag );
    void set_streambuf( streambuf* isb );

protected:
    virtual int overflow( int ch );
    virtual int sync();

private:
    streambuf* sb;
    bool at_newline;
    time_t last_timestamp;
    std::string tag;
    bool tee_cout;

    friend class LogFile;
};

class LogFile : public std::ostream
{
public:
    LogFile( const char* tag );
    ~LogFile();

    bool open( const char* filename );
    void close();
protected:
    timestamp_streambuf* tsb();

private:
    filebuf fb;
};

#endif
