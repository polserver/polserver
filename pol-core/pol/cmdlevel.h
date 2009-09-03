/*
History
=======


Notes
=======

*/

#include <string>
#include <vector>

#include "bscript/bobject.h"

class ConfigElem;
class Package;

class CmdLevel
{
public:
    CmdLevel( ConfigElem& elem, int cmdlevelnum );

    bool matches( const std::string& name ) const;
    void add_searchdir( Package* pkg, const std::string& dir );
    void add_searchdir_front( Package* pkg, const std::string& dir );

    std::string name;
    int cmdlevel;
    
    struct SearchDir
    {
        Package* pkg;
        std::string dir;
    };

    typedef std::vector< SearchDir > SearchList;
    SearchList searchlist;

    typedef std::vector< std::string > Aliases;
    Aliases aliases;
};

typedef std::vector< CmdLevel > CmdLevels;
extern CmdLevels cmdlevels2;

CmdLevel* find_cmdlevel( const char* name );
CmdLevel* FindCmdLevelByAlias( const std::string& str );

ObjArray* GetCommandsInPackage(Package* m_pkg, long cmdlvl_num);


