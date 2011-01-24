/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stddef.h>

#include "../clib/clib.h"

#include "options.h"
#include "fmodule.h"

#include "userfunc.h" 

ModuleFunction::ModuleFunction( const char *fname, 
                                int i_nargs,
                                UserFunction* i_uf) :
    name(fname),
    nargs(i_nargs),
	funcidx(-1),
    uf(i_uf),
    used(false)
{
}

ModuleFunction::~ModuleFunction()
{
}

FunctionalityModule::FunctionalityModule( const char *i_modname ) :
    have_indexes(false),	
    modulename( i_modname )
{
}

FunctionalityModule::~FunctionalityModule()
{
    while (!functions.empty())
    {
        delete functions.back();
        functions.pop_back();
    }
    // compiler only:
    while (!owned_userfuncs.empty())
    {
        delete owned_userfuncs.back();
        owned_userfuncs.pop_back();
    }
}

void FunctionalityModule::addFunction( const char *funcname, 
                                       int nparams,
                                       UserFunction* uf )
{
    ModuleFunction* mf = new ModuleFunction( funcname, nparams, uf );
    mf->funcidx = functions.size();
    functions.push_back( mf );

    // compiler only:
    if (uf != NULL)
        owned_userfuncs.push_back( uf );
}

// compiler only:
bool FunctionalityModule::isFunc(const char *funcName, ModuleFunction **pmf, int *funcidx )
{
    FunctionsByName::iterator itr = functionsByName.find( funcName );
    if (itr != functionsByName.end())
    {
		ModuleFunction* mf = (*itr).second;;

        if (!mf->used)
        {
            mf->used = true;
            int old_funcidx = mf->funcidx;
            mf->funcidx = used_functions.size();
            used_functions.push_back( mf );
            if (old_funcidx != mf->funcidx)
            {
                swap( functions[ old_funcidx ], functions[ mf->funcidx ] );
                functions[ old_funcidx ]->funcidx = old_funcidx;
            }
        }
        *pmf = mf;
		*funcidx = mf->funcidx;
        return true;
    }
    else
    {
        return false;
    }

    return false;
}

void FunctionalityModule::fillFunctionsByName()
{
    for( unsigned i = 0; i < functions.size(); ++i )
    {
        ModuleFunction* mf = functions[i];
        functionsByName[ mf->name ] = mf;
    }
}
