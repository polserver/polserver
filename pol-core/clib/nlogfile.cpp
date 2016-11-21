/** @file
 *
 * @par History
 */


#include "stl_inc.h"
#include "clib.h"
#include <time.h>

#include "nlogfile.h"

#include <cstring>
namespace Pol
{
namespace Clib
{
LogFile::LogFile( const char* i_tag ) : ostream( new timestamp_streambuf( i_tag ) )
{
  tsb()->set_streambuf( &fb );
}
LogFile::~LogFile()
{
  close();
  delete rdbuf();
}
timestamp_streambuf* LogFile::tsb()
{
  return static_cast<timestamp_streambuf*>( rdbuf() );
}
bool LogFile::open( const char* filename )
{
  if ( fb.open( filename, ios::out | ios::app ) )
  {
    ( *this ) << "Logfile opened." << endl;
    tsb()->tee_cout = true;
    return true;
  }
  else
  {
    return false;
  }
}
void LogFile::close()
{
  tsb()->tee_cout = false;
  ( *this ) << "Logfile closed." << endl;
}

timestamp_streambuf::timestamp_streambuf( const char* i_tag )
    : sb( NULL ), at_newline( true ), last_timestamp( 0 ), tag( i_tag ), tee_cout( false )
{
}
void timestamp_streambuf::set_streambuf( streambuf* isb )
{
  sb = isb;
}

int timestamp_streambuf::overflow( int ch )
{
  if ( ch != EOF )
  {
    if ( at_newline )
    {
      at_newline = false;
      time_t now = time( NULL );

      char ts_buf[30];

      strftime( ts_buf, sizeof ts_buf, "%m/%d %H:%M:%S", Clib::localtime( &now ) );
      size_t nbytes = strlen( ts_buf );

      if ( tee_cout )
        cout << "[" << ts_buf << " " << tag << "] ";

      if ( now != last_timestamp )
      {
        sb->sputc( '[' );
        sb->sputn( ts_buf, nbytes );  // note no error checking
        sb->sputc( ']' );
        sb->sputc( ' ' );

        last_timestamp = now;
      }
    }
    if ( ch == '\n' )
      at_newline = true;
    if ( tee_cout )
      cout << char( ch );
    return sb->sputc( ch );
  }
  else
  {
    return EOF;
  }
}

int timestamp_streambuf::sync()
{
  return sb->pubsync();
}
}
}
