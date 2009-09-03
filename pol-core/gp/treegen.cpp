/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include <math.h>

#include "bscript/eprog.h"
#include "clib/random.h"

#include "tree.h"
#include "gphelp.h"

long n_GIndividual;
long n_GPopulation;
long n_GTree;

GIndividual::GIndividual() :
    alg(NULL),
    raw_fitness_known(false),
    raw_fitness(0.0),
    standard_fitness(0.0),
    adjusted_fitness(0.0),
    normalized_fitness(0.0)
{
    ++n_GIndividual;
}

GIndividual::GIndividual( GTree* alg ) :
    alg(alg),
    raw_fitness_known(false),
    raw_fitness(0.0),
    standard_fitness(0.0),
    adjusted_fitness(0.0),
    normalized_fitness(0.0)
{
    ++n_GIndividual;
}

GIndividual::GIndividual( const GIndividual& ind ) :
    alg( ind.alg ),
    raw_fitness(0.0),
    standard_fitness(0.0),
    adjusted_fitness(0.0),
    normalized_fitness(0.0)
{
    ++n_GIndividual;
    if (alg)
        alg = alg->clone();
}

GIndividual& GIndividual::operator=( const GIndividual& ind )
{
    if (&ind != this)
    {
        if (ind.alg)
            alg = ind.alg->clone();
        else
            alg = NULL;

        raw_fitness = ind.raw_fitness;
        standard_fitness = ind.standard_fitness;
        adjusted_fitness = ind.adjusted_fitness;
        normalized_fitness = ind.normalized_fitness;
    }
    return *this;
}

GIndividual::~GIndividual()
{
    --n_GIndividual;
    delete alg;
    alg = NULL;
}

GPopulation::GPopulation(GenDef& gd) :
    gd(gd)
{
    ++n_GPopulation;
}

GPopulation::~GPopulation()
{
    --n_GPopulation;
    while (!individuals.empty())
    {
        delete individuals.back();
        individuals.pop_back();
    }
}
void GPopulation::create_random_population( int size )
{
    while (size--)
    {
        individuals.push_back( new GIndividual(gd.create_random_tree( 6, true )) );
    }
}

const GIndividual* GPopulation::choose_individual() const
{
    if (gd.selection_method == GenDef::SELECTION_TOURNAMENT)
    {
        const GIndividual* a1 = individuals[ random_int( individuals.size() ) ];
        const GIndividual* a2 = individuals[ random_int( individuals.size() ) ];
        if (gd.use_parsimony && 
            a1->raw_fitness >= gd.parsimony_threshhold &&
            a2->raw_fitness >= gd.parsimony_threshhold)
        {
            if (fabs( a1->raw_fitness - a2->raw_fitness ) <= 1.0)
            {
                if ( a1->alg->num_children() < a2->alg->num_children() )
                    return a1;
                else
                    return a2;
            }
        }

        if (a1->raw_fitness > a2->raw_fitness)
            return a1;
        else
            return a2;
    }
    else // fitness proportionate
    {
        double f = static_cast<double>(random_float( 1.0 ));
        for( unsigned i = 0; i < individuals.size(); i++ )
        {
            f -= individuals[i]->normalized_fitness;
            if (f <= 0.0f)
                return individuals[i];
        }
    }
    return individuals[0];
}

void GPopulation::perform_reproduction( GPopulation* newpop ) const
{
    const GIndividual* ind = choose_individual();
    GIndividual* newind = new GIndividual( ind->alg->clone() );
    
    newind->raw_fitness_known = true;
    newind->raw_fitness = ind->raw_fitness;

    newpop->individuals.push_back( newind );
}

void GPopulation::perform_crossover( GPopulation* newpop ) const
{
    const GIndividual* father = choose_individual();
    const GIndividual* mother = choose_individual();

    GIndividual* son = new GIndividual( father->alg->clone() );
    GIndividual* daughter = new GIndividual( mother->alg->clone() );
    
    GTree** from_son = son->alg->choose_random_tree( &son->alg );
    GTree** from_daughter = daughter->alg->choose_random_tree( &daughter->alg );

    GTree* tmp = *from_daughter;

    *from_daughter = *from_son;
    *from_son = tmp;
   
    if (son->alg->max_depth() <= 17)
        newpop->individuals.push_back( son );
    else
        delete son;

    if (daughter->alg->max_depth() <= 17)
        newpop->individuals.push_back( daughter );
    else
        delete daughter;
}

void GPopulation::calculate_fitness_values() const
{
    double adjusted_fitness_sum = 0.0;
    for( Individuals::const_iterator itr = individuals.begin(); itr != individuals.end(); ++itr )
    {
        GIndividual* ind = *itr;
        ind->standard_fitness = gd.max_fitness - ind->raw_fitness;
        ind->adjusted_fitness = 1.0 / (1.0 + ind->standard_fitness);
        adjusted_fitness_sum += ind->adjusted_fitness;
    }

    for( Individuals::const_iterator itr = individuals.begin(); itr != individuals.end(); ++itr )
    {
        GIndividual* ind = *itr;
        ind->normalized_fitness = ind->adjusted_fitness / adjusted_fitness_sum;
    }
}

GPopulation* GPopulation::breed_next_generation( ) const
{
    calculate_fitness_values();
    GPopulation* newpop = new GPopulation( gd );
    while (newpop->individuals.size() < individuals.size())
    {
        int nleft = individuals.size() - newpop->individuals.size();
        if (nleft == 1 || (random_float(1.0) < gd.Pr))   
        {
            perform_reproduction( newpop );
        }
        else                                       
        {
            perform_crossover( newpop );
        }
    }
    return newpop;
}

GFunctionDef::GFunctionDef( const std::string& name, int n_parameters ) :
    name(name),
    n_parameters(n_parameters)
{
}

GPredicateDef::GPredicateDef( const std::string& name, const std::string& predicate ) :
    GFunctionDef( name, 2 ),
    if_expr(if_expr)
{
}

void GPredicateDef::printSourceOn( std::ostream& os, int indentlevel, const GTree* tree ) const
{
    string indent( indentlevel, ' ' );
    os << indent << "if (" << if_expr << ")" << endl;
    
    os << indent << "begin" << endl;

        tree->nodes[0]->printSourceOn( os, indentlevel+1 );

    os << indent << "end else begin" << endl;

        tree->nodes[1]->printSourceOn( os, indentlevel+1 );

    os << indent << "end" << endl;
}

GMultiStatementFunctionDef::GMultiStatementFunctionDef( 
                                                       const std::string& name, int n_parameters ) :
    GFunctionDef( name, n_parameters )
{
}

void GMultiStatementFunctionDef::printSourceOn( std::ostream& os, int indentlevel, const GTree* tree ) const
{
    string indent( indentlevel, ' ' );

    os << indent << "begin" << endl;

    for( int i = 0; i < n_parameters; i++ )
    {
        tree->nodes[0]->printSourceOn( os, indentlevel+1 );
    }

    os << indent << "end" << endl;
}

GTerminalDef::GTerminalDef( const std::string& name ) :
    name(name)
{
}

GTerminal* GTerminalDef::create_terminal()
{
    return new GTerminal( *this );
}
GTerminal* GTerminalDef::create_terminal( istream& is )
{
    return new GTerminal( *this );
}

GFunction::GFunction( const GFunctionDef& funcdef ) :
    funcdef(funcdef)
{
    nodes.resize( funcdef.n_parameters );
}

GTree* GFunction::clone() const
{
    GTree* newtree = new GFunction( funcdef );

    // GFunction constructor creates empty nodes
    for( unsigned i = 0; i < nodes.size(); i++ )
    {
        if (nodes[i])
            newtree->nodes[i] = nodes[i]->clone();
    }

    return newtree;
}

void GFunction::writeOn( std::ostream& os ) const
{
    os << funcdef.name << endl;
    base::writeOn( os );
}

void GFunction::printOn( std::ostream& os, int indentlevel ) const
{
    string indent( indentlevel, ' ' );
    os << indent << funcdef.name << endl;
    base::printOn( os, indentlevel );
}

void GFunction::printSourceOn( std::ostream& os, int indentlevel ) const
{
    funcdef.printSourceOn( os, indentlevel, this );
}

void GFunction::emit( EScriptProgram& prog ) const
{
    funcdef.emit( prog, this );
}

GTerminal::GTerminal( const GTerminalDef& termdef ) :
    termdef(termdef)
{
}

GTree* GTerminal::clone() const
{
    passert( nodes.size() == 0 );
    
    return new GTerminal(termdef);
}

void GTerminal::writeOn( std::ostream& os ) const
{
    os << termdef.name << endl;
    base::writeOn( os );
}

void GTerminal::printOn( std::ostream& os, int indentlevel ) const
{
    string indent( indentlevel, ' ' );
    os << indent << termdef.name << endl;
    base::printOn( os, indentlevel );
}

void GTerminal::printSourceOn( std::ostream& os, int indentlevel ) const
{
    string indent( indentlevel, ' ' );
    os << indent << termdef.name << endl;
}

void GTerminal::emit( EScriptProgram& prog ) const
{
    termdef.emit( prog, this );
}

GDoubleTerminal::GDoubleTerminal( const GDoubleTerminalDef& termdef ) :
    GTerminal( termdef )
{
    _value = termdef._minimum + (termdef._maximum - termdef._minimum) * random_float(1.0);
}
GDoubleTerminal::GDoubleTerminal( const GDoubleTerminalDef& termdef, double value ) :
    GTerminal( termdef )
{
    _value = value;
}

GTree* GDoubleTerminal::clone() const
{
    return new GDoubleTerminal( static_cast<const GDoubleTerminalDef&>(termdef), _value);
}

void GDoubleTerminal::emit( EScriptProgram& prog ) const
{
    prog.addToken( MakeDoubleToken( _value ) );
}
void GDoubleTerminal::writeOn( std::ostream& os ) const
{
    os << termdef.name << " " << _value << endl;
}
void GDoubleTerminal::printOn( std::ostream& os, int indentlevel ) const
{
    string indent( indentlevel, ' ' );
    os << indent << _value << endl;
}

void GDoubleTerminal::printSourceOn( std::ostream& os, int indentlevel ) const
{
    string indent( indentlevel, ' ' );
    os << indent << _value << endl;
}

void GPredicateDef::emit( EScriptProgram& prog, const GTree* ) const
{
    throw runtime_error( "not done!" );
}

void GMultiStatementFunctionDef::emit( EScriptProgram& prog, const GTree* tree ) const
{
    for( unsigned i = 0; i < tree->nodes.size()-1; i++ )
    {
        tree->nodes[i]->emit( prog );
        prog.addToken( Token( Mod_Basic, TOK_CONSUMER, TYP_UNARY_OPERATOR ) );
    }
    tree->nodes[ tree->nodes.size() - 1 ]->emit( prog );
}

GIfThenElseFunctionDef::GIfThenElseFunctionDef() :
    GFunctionDef( "if-then-else", 3 )
{
}

void GIfThenElseFunctionDef::printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const
{
    throw runtime_error( "not done" );
}

void GIfThenElseFunctionDef::emit( EScriptProgram& prog, const GTree* tree ) const
{
    unsigned if_token_posn;
    unsigned skip_else_posn;
    unsigned else_posn;
    unsigned end_posn;
    
    tree->nodes[0]->emit( prog );
    prog.append( StoredToken( Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, 0 ), &if_token_posn );
    tree->nodes[1]->emit( prog );
    prog.append( StoredToken( Mod_Basic, RSV_GOTO, TYP_RESERVED, 0 ), &skip_else_posn );
    else_posn = prog.tokens.next();
    tree->nodes[2]->emit( prog );
    end_posn = prog.tokens.next();
 
    prog.tokens.atPut1(
          StoredToken(Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, else_posn),
          if_token_posn);

    prog.tokens.atPut1(
          StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, end_posn),
          skip_else_posn);

}

GWhileExprDoFunctionDef::GWhileExprDoFunctionDef() :
    GFunctionDef( "while-do", 2 )
{
}

void GWhileExprDoFunctionDef::printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const
{
    throw runtime_error( "not done" );
}

void GWhileExprDoFunctionDef::emit( EScriptProgram& prog, const GTree* tree) const
{
    unsigned continue_posn;
    unsigned end_posn;
    unsigned test_expr_posn;
    prog.addToken( Token( Mod_Basic, TOK_LONG, TYP_OPERAND ) );
    continue_posn = prog.tokens.next();
    tree->nodes[0]->emit( prog );
    prog.append( StoredToken( Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, 0 ), &test_expr_posn );
    prog.append( StoredToken( Mod_Basic, TOK_CONSUMER, TYP_UNARY_OPERATOR ) );
    tree->nodes[1]->emit( prog );
    prog.append( StoredToken( Mod_Basic, RSV_GOTO, TYP_RESERVED, continue_posn ) );
    end_posn = prog.tokens.next();
    prog.tokens.atPut1( 
            StoredToken( Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, end_posn ),
            test_expr_posn );
}


GIfTokenThenElseFunctionDef::GIfTokenThenElseFunctionDef( const std::string& name, 
                                                         const Token& tkn ) :
    GFunctionDef( name, 2 ),
    tkn(tkn)
{

}

void GIfTokenThenElseFunctionDef::printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const
{
    throw runtime_error( "not done" );
}

void GIfTokenThenElseFunctionDef::emit( EScriptProgram& prog, const GTree* tree ) const
{
    unsigned if_token_posn;
    unsigned skip_else_posn;
    unsigned else_posn;
    unsigned end_posn;
    
    prog.addToken( tkn );
    prog.append( StoredToken( Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, 0 ), &if_token_posn );
    tree->nodes[0]->emit( prog );
    prog.append( StoredToken( Mod_Basic, RSV_GOTO, TYP_RESERVED, 0 ), &skip_else_posn );
    else_posn = prog.tokens.next();
    tree->nodes[1]->emit( prog );
    end_posn = prog.tokens.next();
 
    prog.tokens.atPut1(
          StoredToken(Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, else_posn),
          if_token_posn);

    prog.tokens.atPut1(
          StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, end_posn),
          skip_else_posn);
}



GTokenFunctionDef::GTokenFunctionDef( const std::string& name, 
                       unsigned n_parameters,
                       const Token& tkn ) :
    GFunctionDef( name, n_parameters ),
    tkn(tkn)
{
}

void GTokenFunctionDef::printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const
{
}

void GTokenFunctionDef::emit( EScriptProgram& prog, const GTree* tree) const
{
    for( unsigned i = 0; i < tree->nodes.size(); i++ )
    {
        tree->nodes[i]->emit( prog );
    }
    prog.addToken( tkn );
}


void GDoubleTerminalDef::emit( EScriptProgram& prog, const GTerminal* term ) const
{
    throw logic_error("oops");
}
GTerminal* GDoubleTerminalDef::create_terminal()
{
    return new GDoubleTerminal( *this );
}
GTerminal* GDoubleTerminalDef::create_terminal( istream& is )
{
    double value;
    is >> value;
    return new GDoubleTerminal( *this, value );
}


GTokenTerminalDef::GTokenTerminalDef( const std::string& name, const Token& tkn ) :
    GTerminalDef( name ),
    tkn(tkn)
{
}

void GTokenTerminalDef::printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const
{
}

void GTokenTerminalDef::emit( EScriptProgram& prog, const GTerminal* term ) const
{
    prog.addToken( tkn );
}

void GPopulation::write( const string& filename) const
{
    ofstream ofs( filename.c_str(), ios::trunc );
    ofs << individuals.size() << endl;
    for( unsigned i = 0; i < individuals.size(); i++ )
    {
        individuals[i]->alg->writeOn( ofs );
        ofs << "###END###" << endl;
    }
}

void GPopulation::read( const string& filename )
{
    ifstream ifs( filename.c_str() );
    long size;
    ifs >> size;
    while (size--)
    {
        GTree* tree = gd.read_tree( ifs );
        individuals.push_back( new GIndividual( tree ) );
    }
}

unsigned GPopulation::size() const
{
    return individuals.size();
}
