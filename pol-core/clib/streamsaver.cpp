#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "../../lib/format/format.h"
#include "message_queue.h"
#include "streamsaver.h"

const std::size_t flush_limit = 500;

/// BaseClass implements only writer operator logic
StreamWriter::StreamWriter() : _writer( new fmt::Writer )
{
}
StreamWriter::~StreamWriter()
{
}

fmt::Writer& StreamWriter::operator()()
{
    if ( _writer->size() >= flush_limit )  // guard against to big objects
    {
        this->flush();
    }
    return *( _writer.get() );
}

/// ofstream implementation (simple non threaded)
OFStreamWriter::OFStreamWriter() : StreamWriter(), _stream()
{
}

OFStreamWriter::OFStreamWriter( std::ofstream* stream )
    : StreamWriter(), _stream( stream )
{
}

OFStreamWriter::~OFStreamWriter()
{
    if ( _writer->size() ) *_stream << _writer->c_str();
}

void OFStreamWriter::init( const std::string& filepath )
{
    _stream->exceptions( std::ios_base::failbit | std::ios_base::badbit );
    _stream->open( filepath.c_str(), std::ios::out );
}

void OFStreamWriter::flush()
{
    if ( _writer->size() )
    {
        *_stream << _writer->c_str();
        _writer.reset( new fmt::Writer );
    }
}

void OFStreamWriter::flush_file()
{
    flush();
    _stream->flush();
}

/// ostream implementation (non threaded)
OStreamWriter::OStreamWriter() : StreamWriter(), _stream()
{
}

OStreamWriter::OStreamWriter( std::ostream* stream )
    : StreamWriter(), _stream( stream )
{
}

OStreamWriter::~OStreamWriter()
{
    if ( _writer->size() ) *_stream << _writer->c_str();
}

void OStreamWriter::init( const std::string& filepath )
{
}

void OStreamWriter::flush()
{
    if ( _writer->size() )
    {
        *_stream << _writer->c_str();
        _writer.reset( new fmt::Writer );
    }
}

void OStreamWriter::flush_file()
{
    flush();
    _stream->flush();
}

/// ofstream implementation with worker thread for file io
ThreadedOFStreamWriter::ThreadedOFStreamWriter()
    : StreamWriter(), _stream(), _msg_queue(), _writethread()
{
    start_worker();
}

ThreadedOFStreamWriter::ThreadedOFStreamWriter( std::ofstream* stream )
    : StreamWriter(), _stream( stream ), _msg_queue(), _writethread()
{
    start_worker();
}
void ThreadedOFStreamWriter::start_worker()
{
    _writethread = std::thread( [this]()
    {
        std::list<WriterPtr> writers;
        // small helper lambda to write into stream
        auto _write_to_stream = [&]( std::list<WriterPtr> & l )
        {
            for ( const auto& _w : l )
            {
                if ( _w->size() )
                    *_stream << _w->c_str();
            }
        };
        try
        {
            while ( 1 )
            {
                writers.clear();
                _msg_queue.pop_wait( &writers );
                _write_to_stream( writers );
            }
        }
        catch ( writer_queue::Canceled& e )
        {
        }
        writers.clear();
        _msg_queue.pop_remaining( &writers );
        _write_to_stream( writers );
        _stream->flush();
    } );
}

ThreadedOFStreamWriter::~ThreadedOFStreamWriter()
{
    flush();
    _msg_queue.cancel();
    _writethread.join();
}

void ThreadedOFStreamWriter::init( const std::string& filepath )
{
    _stream->exceptions( std::ios_base::failbit | std::ios_base::badbit );
    _stream->open( filepath.c_str(), std::ios::out );
}

void ThreadedOFStreamWriter::flush()
{
    if ( _writer->size() )
    {
        _msg_queue.push_move( std::move( _writer ) );
        _writer.reset( new fmt::Writer );
    }
}

void ThreadedOFStreamWriter::flush_file()
{
    flush();
    _stream->flush();
}
