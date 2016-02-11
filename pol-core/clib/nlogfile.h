/** @file
 *
 * @par History
 */


#ifndef NLOGFILE_H
#define NLOGFILE_H

#include <iostream>
#include <fstream>
#include <string>
namespace Pol {
  namespace Clib {
	class timestamp_streambuf : public std::streambuf
	{
	public:
	  timestamp_streambuf( const char* tag );
	  void set_streambuf( streambuf* isb );

	protected:
	  virtual int overflow( int ch ) POL_OVERRIDE;
	  virtual int sync() POL_OVERRIDE;

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
  }
}

#endif
