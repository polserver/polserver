/*
History
=======

2009/08/25 Shinigami: STLport-5.2.1 fix: Log and Log2 changed little bit

Notes
=======

*/

#include "stl_inc.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "logfile.h"

FILE *logfile = NULL;
static string namebase;
static string log_filename;
static time_t last_timestamp;
static struct tm open_tm;
static bool rollover;

int OpenLogFileName( const char* i_namebase, bool i_rollover )
{
    namebase = i_namebase;
    rollover = i_rollover;
    log_filename = namebase + ".log";
    logfile = fopen( log_filename.c_str(), "a+t" );
    if (logfile) 
    {
        time_t now = time(NULL);
        open_tm = *localtime( &now );

        Log( "Logfile opened.\n" );
        return 0;
    }
    else 
    {
        return -1;
    }
}
int OpenLogFile(void)
{
	return OpenLogFileName( "logfile", false );
}

void CloseLogFile(void)
{
    Log( "Closing logfile.\n" );
    if (logfile) 
    {
        fclose(logfile);
        logfile = NULL;
    }
}

void Log( const char *fmt, ... )
{
    if (logfile)
    {
        va_list va;
        time_t now = time(NULL);
        if (now != last_timestamp)
        {
            char buffer[30];

            struct tm* tm_now = localtime( &now );
            if (rollover && (tm_now->tm_mday != open_tm.tm_mday || 
                             tm_now->tm_mon != open_tm.tm_mon))
            {
                // roll the log file over
                strftime( buffer, sizeof buffer, "%Y-%m-%d", &open_tm );
                string archive_name = namebase + "-" + buffer + ".log";
                fclose( logfile );
                
                // whether the rename succeeds or fails, the action is the same.
                rename( log_filename.c_str(), archive_name.c_str() );

                // moved.  open the new file
                logfile = fopen( log_filename.c_str(), "a+t" );
                if (!logfile)
                    return;
                open_tm = *tm_now;
            }

            strftime( buffer, sizeof buffer, "%m/%d %H:%M:%S", tm_now );
            fprintf( logfile, "[%s] ", buffer );
            last_timestamp = now;
        }
        va_start(va,fmt);
        vfprintf( logfile, fmt, va );
        va_end(va);
        fflush( logfile );
    }
}

void Log2( const char *fmt, ... )
{
    if (logfile)
    {
        va_list va;
        time_t now = time(NULL);
        if (now != last_timestamp)
        {
            char buffer[30];

            struct tm* tm_now = localtime( &now );
            if (rollover && (tm_now->tm_mday != open_tm.tm_mday || 
                             tm_now->tm_mon != open_tm.tm_mon))
            {
                // roll the log file over
                strftime( buffer, sizeof buffer, "%Y-%m-%d", &open_tm );
                string archive_name = namebase + "-" + buffer + ".log";
                fclose( logfile );
                
                // whether the rename succeeds or fails, the action is the same.
                rename( log_filename.c_str(), archive_name.c_str() );

                // moved.  open the new file
                logfile = fopen( log_filename.c_str(), "a+t" );
                if (!logfile) 
                    return;
                open_tm = *tm_now;
            }

            strftime( buffer, sizeof buffer, "%m/%d %H:%M:%S", tm_now );
            fprintf( logfile, "[%s] ", buffer );
            last_timestamp = now;
        }
        va_start(va,fmt);
        vfprintf( logfile, fmt, va );
        va_end(va);
        fflush( logfile );
    }
    va_list va;
    va_start( va, fmt );
    vfprintf( stdout, fmt, va );
    va_end( va );
}

