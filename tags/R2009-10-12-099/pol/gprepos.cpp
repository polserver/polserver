/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include "../bscript/fmodule.h"

#include "../gp/gphelp.h"
#include "../gp/tree.h"

#include "gprepos.h"

typedef map<string,GTerminalDef*> TerminalDefs;
typedef map<string,GFunctionDef*> FunctionDefs;

TerminalDefs terminal_defs;
FunctionDefs function_defs;


GIfThenElseFunctionDef f_ifthenelse;
GWhileExprDoFunctionDef f_whileexprdo;
GMultiStatementFunctionDef f_prog2n( "prog2n", 2 );
GMultiStatementFunctionDef f_prog3n( "prog3n", 3 );

FunctionalityModule fm( "gp" );


void add_terminal_def( GTerminalDef* termdef )
{
    terminal_defs.insert( TerminalDefs::value_type( termdef->name, termdef ) );
}

void add_function_def( GFunctionDef* funcdef )
{
    function_defs.insert( FunctionDefs::value_type( funcdef->name, funcdef ) );
}

bool init = false;
void init_gp_repository()
{
    add_function_def( &f_ifthenelse );
    add_function_def( &f_whileexprdo );
    add_function_def( &f_prog2n );
    add_function_def( &f_prog3n );

    static GTokenTerminalDef t_move_forward( "MoveForward", MakeFunctionToken( 0, 0 ) );
    fm.addFunction( "MoveForward", 0 );
    add_terminal_def( &t_move_forward );

    init = true;
}


GTerminalDef* find_terminal( const std::string& name )
{
    if (!init)
    {
        init_gp_repository();
    }

    TerminalDefs::iterator itr = terminal_defs.find( name );
    if (itr != terminal_defs.end())
        return (*itr).second;
    else
        return NULL;
}

GFunctionDef* find_function( const std::string& name )
{
    if (!init)
    {
        init_gp_repository();
    }

    FunctionDefs::iterator itr = function_defs.find( name );
    if (itr != function_defs.end())
        return (*itr).second;
    else
        return NULL;
}
