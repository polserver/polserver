/*
History
=======
2005/07/01 Shinigami: added ConfigFile::_modified (stat.st_mtime) to detect cfg file modification

Notes
=======

*/

#ifndef CLIB_CFGFILE_H
#define CLIB_CFGFILE_H

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#define CFGFILE_USES_IOSTREAMS 0

#if CFGFILE_USES_IOSTREAMS
#	include <fstream>
#else
#	include <stdio.h>
#endif

#include <set>
#include <string>
#include <vector>

#include "maputil.h"


class ConfigElemBase;
class ConfigElem;
class VectorConfigElem;

class ConfigSource
{
public:
    virtual ~ConfigSource() {}
    virtual void display_error( const std::string& msg, 
                        bool show_curline = true,
                        const ConfigElemBase* elem = NULL,
                        bool error = true ) const = 0;
};

class ConfigFile : public ConfigSource
{
public:
    explicit ConfigFile( const char *filename = NULL, const char *allowed_types = NULL );
    explicit ConfigFile( const string& filename, const char *allowed_types = NULL );
    virtual ~ConfigFile();

    void open( const char *i_filename );

    bool read( ConfigElem& elem );        // true=got one, false=end of file
    void readraw( ConfigElem& elem );     // reads 0 or more properties

    const string& filename() const;
    const time_t modified() const;
    unsigned element_line_start() const;


protected:
    void init( const char* i_filename, const char* allowed_types_str );
    friend class ConfigElem;
    friend class ConfigSection;
    bool readline( std::string& strbuf );
    bool read_properties( ConfigElem& elem );
    bool read_properties( VectorConfigElem& elem );
    bool _read( ConfigElem& elem );
    bool _read( VectorConfigElem& elem );
    void display_error( const std::string& msg, 
                        bool show_curline = true,
                        const ConfigElemBase* elem = NULL,
                        bool error = true ) const;
    void display_and_rethrow_exception();
    void register_allowed_type( const char* allowed_type );

private:
    std::string _filename; // saved for exception reporting
    time_t _modified; // used to detect modification
#if CFGFILE_USES_IOSTREAMS
     std::ifstream ifs;
#else
    FILE *fp;
    static char buffer[ 1024 ];
#endif
    int _element_line_start; // what line in the file did this elem start on?
    int _cur_line;

    typedef std::set<string, ci_cmp_pred> AllowedTypesCont;
    AllowedTypesCont allowed_types_;
};

class StubConfigSource : public ConfigSource
{
public:
    virtual void display_error( const std::string& msg, 
                        bool show_curline,
                        const ConfigElemBase* elem,
                        bool error ) const;
};

#endif
