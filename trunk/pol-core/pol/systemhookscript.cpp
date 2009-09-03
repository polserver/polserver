/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"

#include "bscript/berror.h"

#include "scrsched.h"
#include "syshookscript.h"

ExportScript::ExportScript( const Package* pkg, std::string scriptname )
{
    if (scriptname.find(".ecl") == string::npos)
        scriptname += ".ecl";
    sd.quickconfig( pkg, scriptname );
    
}

ExportScript::ExportScript( const ScriptDef& isd ) :
    sd(isd)
{
}

bool ExportScript::Initialize()
{
    BObject ob( run_executor_to_completion( uoexec, sd ) );
    return ob.isTrue();
}

const std::string& ExportScript::scriptname() const
{
    return sd.name();
}

bool ExportScript::FindExportedFunction( const std::string& name, unsigned args, unsigned& PC ) const
{
    const EScriptProgram* prog = uoexec.prog();
    for( unsigned i = 0; i < prog->exported_functions.size(); ++i )
    {
        if (stricmp( prog->exported_functions[i].name.c_str(), name.c_str() ) == 0)
        {
            if (args != prog->exported_functions[i].nargs)
            {
                cout << "Exported function " << name 
                     << " in script " << scriptname() 
                     << " takes " << prog->exported_functions[i].nargs 
                     << " parameters, expected " << args << endl;
                return false;
            }
            PC = prog->exported_functions[i].PC;
            return true;
        }
    }
    return false;
}

bool ExportScript::FindExportedFunction( const char* name, unsigned args, unsigned& PC ) const
{
    const EScriptProgram* prog = uoexec.prog();
    for( unsigned i = 0; i < prog->exported_functions.size(); ++i )
    {
        if (stricmp( prog->exported_functions[i].name.c_str(), name ) == 0)
        {
            if (args != prog->exported_functions[i].nargs)
            {
                cout << "Exported function " << name 
                     << " in script " << scriptname() 
                     << " takes " << prog->exported_functions[i].nargs 
                     << " parameters, expected " << args << endl;
                return false;
            }
            PC = prog->exported_functions[i].PC;
            return true;
        }
    }
    return false;
}


bool ExportScript::call( unsigned PC, 
                             BObjectImp* p0,
                             BObjectImp* p1,
                             BObjectImp* p2,
                             BObjectImp* p3 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        uoexec.pushArg(p1);
        uoexec.pushArg(p2);
        uoexec.pushArg(p3);
        
        uoexec.exec();
        if (uoexec.error())
            return false;
        if (uoexec.ValueStack.empty())
            return false;
        bool istrue = uoexec.ValueStack.top().get()->isTrue();
        uoexec.ValueStack.pop();
        return istrue;
    }
    catch(std::exception&)//...
    {
        return false;
    }
}

bool ExportScript::call( unsigned PC, 
                             BObjectImp* p0,
                             BObjectImp* p1,
                             BObjectImp* p2 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        uoexec.pushArg(p1);
        uoexec.pushArg(p2);
        
        uoexec.exec();
        if (uoexec.error())
            return false;
        if (uoexec.ValueStack.empty())
            return false;
        bool istrue = uoexec.ValueStack.top().get()->isTrue();
        uoexec.ValueStack.pop();
        return istrue;
    }
    catch(std::exception&)//...
    {
        return false;
    }
}

bool ExportScript::call( unsigned PC, 
                             BObjectImp* p0,
                             BObjectImp* p1 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        uoexec.pushArg(p1);
        
        uoexec.exec();
        if (uoexec.error())
            return false;
        if (uoexec.ValueStack.empty())
            return false;
        bool istrue = uoexec.ValueStack.top().get()->isTrue();
        uoexec.ValueStack.pop();
        return istrue;
    }
    catch(std::exception&)//...
    {
        return false;
    }
}

bool ExportScript::call( unsigned PC, 
                             BObjectImp* p0 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        
        uoexec.exec();
        if (uoexec.error())
            return false;
        if (uoexec.ValueStack.empty())
            return false;
        bool istrue = uoexec.ValueStack.top().get()->isTrue();
        uoexec.ValueStack.pop();
        return istrue;
    }
    catch(std::exception&)//...
    {
        return false;
    }
}
std::string ExportScript::call_string( 
               unsigned PC,
               BObjectImp* p0, BObjectImp* p1 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        uoexec.pushArg(p1);
        
        uoexec.exec();
        if (uoexec.error())
            return "error";
        if (uoexec.ValueStack.empty())
            return "error";
        std::string ret;
        ret = uoexec.ValueStack.top().get()->impptr()->getStringRep();
        uoexec.ValueStack.pop();
        return ret;
    }
    catch(std::exception&)//...
    {
        return "exception";
    }
}

std::string ExportScript::call_string( 
               unsigned PC,
               BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        uoexec.pushArg(p1);
        uoexec.pushArg(p2);
        
        uoexec.exec();
        if (uoexec.error())
            return "error";
        if (uoexec.ValueStack.empty())
            return "error";
        std::string ret;
        ret = uoexec.ValueStack.top().get()->impptr()->getStringRep();
        uoexec.ValueStack.pop();
        return ret;
    }
    catch(std::exception&)//...
    {
        return "exception";
    }
}

long ExportScript::call_long( 
               unsigned PC,
               BObjectImp* p0 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        
        uoexec.exec();
        if (uoexec.error())
            return 0;
        if (uoexec.ValueStack.empty())
            return 0;
        
        long ret;
        BObjectImp* imp = uoexec.ValueStack.top().get()->impptr();
        if (imp->isa( BObjectImp::OTLong ))
        {
            BLong* pLong = static_cast<BLong*>(imp);
            ret = pLong->value();
        }
        else
        {
            ret =  0;
        }
        uoexec.ValueStack.pop();
        
        return ret;
    }
    catch(std::exception&)//...
    {
        return 0;
    }
}

long ExportScript::call_long( 
               unsigned PC,
               BObjectImp* p0, BObjectImp* p1 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
		uoexec.pushArg(p1);
        
        uoexec.exec();
        if (uoexec.error())
            return 0;
        if (uoexec.ValueStack.empty())
            return 0;
        
        long ret;
        BObjectImp* imp = uoexec.ValueStack.top().get()->impptr();
        if (imp->isa( BObjectImp::OTLong ))
        {
            BLong* pLong = static_cast<BLong*>(imp);
            ret = pLong->value();
        }
        else
        {
            ret =  0;
        }
        uoexec.ValueStack.pop();
        
        return ret;
    }
    catch(std::exception&)//...
    {
        return 0;
    }
}

BObjectImp* ExportScript::call( unsigned PC, 
                                    BObjectImp* p0, 
                                    std::vector<BObjectRef>& pmore )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        unsigned n = pmore.size();
        for( unsigned i = 0; i < n; ++i )
        {   // push BObjectRef so params can be pass-by-ref
            uoexec.pushArg( pmore[i] ); 
        }
        
        uoexec.exec();
        if (uoexec.error())
            return new BError( "Error during execution" );
        if (uoexec.ValueStack.empty())
            return new BError( "There was no return value??" );
        BObjectImp* ret = uoexec.ValueStack.top()->impptr()->copy();
        uoexec.ValueStack.pop();
        return ret;
    }
    catch(std::exception&)//...
    {
        return new BError( "Exception during execution" );
    }
}

BObject ExportScript::call( unsigned PC, 
                                BObjectImp* p0, 
                                BObjectImpRefVec& pmore )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        unsigned n = pmore.size();
        for( unsigned i = 0; i < n; ++i )
        { 
            uoexec.pushArg( pmore[i].get() ); 
        }
        
        uoexec.exec();
        if (uoexec.error())
            return BObject( new BError( "Error during execution" ) );
        if (uoexec.ValueStack.empty())
            return BObject( new BError( "There was no return value??" ) );
        BObjectImp* ret = uoexec.ValueStack.top()->impptr()->copy();
        uoexec.ValueStack.pop();
        return BObject(ret);
    }
    catch(std::exception&)//...
    {
        return BObject( new BError( "Exception during execution" ) );
    }
}

BObject ExportScript::call_object( 
               unsigned PC,
               BObjectImp* p0, BObjectImp* p1 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        uoexec.pushArg(p1);
        
        uoexec.exec();
        if (uoexec.error())
            return BObject( new BError( "Error during execution" ) );
        if (uoexec.ValueStack.empty())
            return BObject( new BError( "There was no return value??" ) );
        BObjectImp* ret = uoexec.ValueStack.top()->impptr()->copy();
        uoexec.ValueStack.pop();
        return BObject(ret);
    }
    catch(std::exception&)//...
    {
        return BObject( new BError( "Exception during execution" ) );
    }
}

BObject ExportScript::call_object( 
               unsigned PC,
               BObjectImp* p0, BObjectImp* p1,BObjectImp* p2 )
{
    try
    {
        uoexec.initForFnCall( PC );

        uoexec.pushArg(p0);
        uoexec.pushArg(p1);
		uoexec.pushArg(p2);
        
        uoexec.exec();
        if (uoexec.error())
            return BObject( new BError( "Error during execution" ) );
        if (uoexec.ValueStack.empty())
            return BObject( new BError( "There was no return value??" ) );
        BObjectImp* ret = uoexec.ValueStack.top()->impptr()->copy();
        uoexec.ValueStack.pop();
        return BObject(ret);
    }
    catch(std::exception&)//...
    {
        return BObject( new BError( "Exception during execution" ) );
    }
}
