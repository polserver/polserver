#ifndef _STREAMSAVER_H
#define _STREAMSAVER_H

#include <boost/noncopyable.hpp>
#include <iosfwd>
#include <string>
#include <memory>
#include <thread>
#include "../../lib/format/format.h"
#include "message_queue.h"

namespace Pol {
  namespace Clib {
	class StreamWriter : boost::noncopyable
	{
	public:
	  StreamWriter();
	  virtual ~StreamWriter();
	  fmt::Writer& operator()();
	  virtual void init( const std::string &filepath ) = 0;
	  virtual void flush() = 0;
	  virtual void flush_file() = 0;
	protected:
	  std::unique_ptr<fmt::Writer> _writer;
	};

    class FMTStreamWriter : public StreamWriter
    {
    public:
      FMTStreamWriter() : StreamWriter() {};
      virtual ~FMTStreamWriter() {};
      virtual void init( const std::string &filepath ) {};
      virtual void flush() {};
      virtual void flush_file() {};
    };

	class OFStreamWriter : public StreamWriter
	{
	public:
	  OFStreamWriter();
	  OFStreamWriter( std::ofstream *stream );
	  virtual ~OFStreamWriter();
	  virtual void init( const std::string &filepath );
	  virtual void flush();
	  virtual void flush_file();
	private:
	  std::ofstream *_stream;
	};

	class OStreamWriter : public StreamWriter
	{
	public:
	  OStreamWriter();
	  OStreamWriter( std::ostream *stream );
	  virtual ~OStreamWriter();
	  virtual void init( const std::string &filepath );
	  virtual void flush();
	  virtual void flush_file();
	private:
	  std::ostream* _stream;
	};


	class ThreadedOFStreamWriter : public StreamWriter
	{
	  typedef std::unique_ptr<fmt::Writer> WriterPtr;
	  typedef message_queue<WriterPtr> writer_queue;
	public:
	  ThreadedOFStreamWriter();
	  ThreadedOFStreamWriter( std::ofstream *stream );
	  virtual ~ThreadedOFStreamWriter();
	  virtual void init( const std::string &filepath );
	  virtual void flush();
	  virtual void flush_file();
	private:
	  void start_worker();
	  std::ofstream *_stream;
	  writer_queue _msg_queue;
	  std::thread _writethread;
      std::list<WriterPtr> _writers_hold;
      std::string _stream_name;
	};

  }
}
#endif // !_STREAMSAVER_H
