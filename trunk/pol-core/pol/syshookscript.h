/*
History
=======


Notes
=======

*/

#ifndef SYSHOOKSCRIPT_H
#define SYSHOOKSCRIPT_H

#include <string>
#include <vector>

#include "scrdef.h"
#include "uoexec.h"

class Package;
class ExportScript
{
public:
    ExportScript( const Package* pkg, std::string scriptname );
    ExportScript( const ScriptDef& isd );
    bool Initialize();
    const std::string& scriptname() const;
    bool FindExportedFunction( const std::string& name, unsigned args, unsigned& PC ) const;
    bool FindExportedFunction( const char* name, unsigned args, unsigned& PC ) const;

    bool call( unsigned PC,
               BObjectImp* p0 ); // throw()
    bool call( unsigned PC,
               BObjectImp* p0, BObjectImp* p1 ); // throw()
    bool call( unsigned PC,
               BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 ); // throw()
    bool call( unsigned PC,
               BObjectImp* p0, BObjectImp* p1, BObjectImp* p2, BObjectImp* p3 ); // throw()

    std::string call_string( unsigned PC,
                             BObjectImp* p0, BObjectImp* p1 ); // throw()
    std::string call_string( unsigned PC,
                                              BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 ); // throw()

    long call_long( unsigned PC, BObjectImp* p0 ); // throw()
	long call_long( unsigned PC, BObjectImp* p0, BObjectImp* p1 ); // throw()
    
    BObjectImp* call( unsigned PC, BObjectImp* p0, std::vector<BObjectRef>& pmore );
    BObject call( unsigned PC, BObjectImp* p0, BObjectImpRefVec& pmore );

	BObject call_object( unsigned PC, BObjectImp* p0, BObjectImp* p1 );
	BObject call_object( unsigned PC, BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 );

    friend class SystemHook;

private:
    ScriptDef sd;
    UOExecutor uoexec;
};

#endif
