/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"


#include "../clib/random.h"
#include "tree.h"

GenDef::GenDef() :
    Pr( 0.10f ),
    max_fitness( 1000.0f ),
    selection_method( SELECTION_FITNESS_PROP ),
    use_parsimony( false ),
    parsimony_threshhold( 0.0f )
{
}

GenDef::~GenDef()
{
}

void GenDef::add_terminal( GTerminalDef* termdef )
{
    terminals.push_back( termdef );
}

void GenDef::add_function( GFunctionDef* funcdef )
{
    functions.push_back( funcdef );
}

GTree* GenDef::create_random_terminal()
{
    int choice = random_int( terminals.size() );

    GTerminalDef* td = terminals[ choice ];

    return td->create_terminal();
}

void GenDef::create_function_args( GFunction* f, int maxdepth, bool popfull )
{
    for( int i = 0; i < f->funcdef.n_parameters; i++ )
    {
        f->nodes[ i ] = create_random_tree( maxdepth-1, popfull, false );
    }
}

GFunction* GenDef::create_random_function( int maxdepth, bool popfull )
{    
    int choice;
    
    choice = random_int(functions.size());

    GFunction* f = new GFunction( *functions[choice] );

    create_function_args( f, maxdepth, popfull );
    
    return f;
}

GTree* GenDef::create_random_function_or_terminal( int maxdepth, bool popfull )
{
    int choice = random_int( functions.size() + terminals.size() );
    if (choice < int(functions.size()))
    {
        GFunction* f = new GFunction( *functions[choice] );
        create_function_args( f, maxdepth, popfull );
        return f;
    }
    else
    {
        return terminals[ choice - functions.size() ]->create_terminal();
    }
}

GTree* GenDef::create_random_tree( int maxdepth,
                                   bool popfull,
                                   bool isroot )
{
    if (maxdepth <= 0)        
    {
        GTree* t = create_random_terminal();
        return t;
    }
    else if (isroot || popfull) // full pop, or root, get functions only.
    {
        GFunction* f = create_random_function( maxdepth-1, popfull );
        return f;
    }
    else
    {
        GTree* t = create_random_function_or_terminal( maxdepth-1, popfull );
        return t;
    }
}

GTree* GenDef::create_random_tree( int maxdepth, bool popfull )
{
    return create_random_tree( maxdepth, popfull, true /* isroot */ );
}

GTree* GenDef::create_node_by_tag( const string& tag, istream& is ) const
{
    for( unsigned i = 0; i < functions.size(); i++ )
    {
        if (functions[i]->name == tag)
            return new GFunction( *functions[i] );
    }
    for( unsigned i = 0; i < terminals.size(); i++ )
    {
        if (terminals[i]->name == tag)
            return terminals[i]->create_terminal(is);
    }
    throw runtime_error( string( "Unknown terminal or function: " ) + tag );
}

GTree* GenDef::inner_read_tree( istream& is )
{
    string tag;
    is >> tag;

    GTree* tree = create_node_by_tag( tag, is );
    for( unsigned i = 0; i < tree->nodes.size(); i++ )
    {
        tree->nodes[i] = inner_read_tree( is );
    }
    return tree;
}

GTree* GenDef::read_tree( istream& is )
{
    GTree* tree = inner_read_tree( is );

    string endtag;
    is >> endtag;
    if (endtag != "###END###")
        throw runtime_error( endtag + " read instead of ###END### looking for end-of-program" );
    
    return tree;
}
