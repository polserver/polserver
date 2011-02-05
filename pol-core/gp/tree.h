/*
History
=======

Notes
=======

*/

#ifndef TREE_H
#define TREE_H

#include <iosfwd>
#include <string>
#include <vector>

#include "../bscript/token.h"

extern int n_GIndividual;
extern int n_GPopulation;
extern int n_GTree;

class GTerminal;
class GTree;
class GFunction;

class EScriptProgram;

class GFunctionDef
{
public:
    GFunctionDef( const std::string& name,
                  int n_parameters );
    virtual ~GFunctionDef() {}

    std::string name;
    int n_parameters;

    virtual void printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const = 0;
    virtual void emit( EScriptProgram& prog, const GTree* ) const = 0;
};

class GTerminalDef
{
public:
    GTerminalDef( const std::string& name );
    virtual ~GTerminalDef() {}
    std::string name;

    virtual GTerminal* create_terminal();
    virtual GTerminal* create_terminal( std::istream& is );
    virtual void emit( EScriptProgram& prog, const GTerminal* term ) const = 0;
};


class GPredicateDef : public GFunctionDef
{
public:
    GPredicateDef( const std::string& name, const std::string& if_expr );

    std::string if_expr;
    virtual void printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const;
    virtual void emit( EScriptProgram& prog, const GTree* ) const;
};

class GMultiStatementFunctionDef : public GFunctionDef
{
public:
    GMultiStatementFunctionDef( const std::string& name,
                  int n_parameters );

    virtual void printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const;
    virtual void emit( EScriptProgram& prog, const GTree* ) const;
};

class GIfThenElseFunctionDef : public GFunctionDef
{
public:
    GIfThenElseFunctionDef();

    virtual void printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const;
    virtual void emit( EScriptProgram& prog, const GTree* ) const;
};

class GWhileExprDoFunctionDef : public GFunctionDef
{
public:
    GWhileExprDoFunctionDef();

    virtual void printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const;
    virtual void emit( EScriptProgram& prog, const GTree* ) const;

};

class GTokenFunctionDef : public GFunctionDef
{
public:
    GTokenFunctionDef( const std::string& name, 
                       unsigned n_parameters,
                       const Token& tkn );

    virtual void printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const;
    virtual void emit( EScriptProgram& prog, const GTree* ) const;

    Token tkn;
};

class GIfTokenThenElseFunctionDef : public GFunctionDef
{
public:
    GIfTokenThenElseFunctionDef( const std::string& name, 
                                 const Token& tkn );

    virtual void printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const;
    virtual void emit( EScriptProgram& prog, const GTree* ) const;

    Token tkn;
};


class GTokenTerminalDef : public GTerminalDef
{
public:
    GTokenTerminalDef( const std::string& name, const Token& tkn );
    GTokenTerminalDef();
    GTokenTerminalDef( const GTokenTerminalDef& );

    virtual void printSourceOn( std::ostream& os, int indentlevel, const GTree* ) const;
    virtual void emit( EScriptProgram& prog, const GTerminal* term ) const;

    Token tkn;
};

class GDoubleTerminalDef : public GTerminalDef
{
public:
    GDoubleTerminalDef( double i_minimum, double i_maximum ) : 
        GTerminalDef( "double" ), 
        _minimum(i_minimum),
        _maximum(i_maximum)
        {}

    virtual GTerminal* create_terminal();
    virtual GTerminal* create_terminal( std::istream& is );
    virtual void emit( EScriptProgram& prog, const GTerminal* term ) const;

    double _minimum;
    double _maximum;
};

class GTree
{
public:
    GTree();
    GTree( const GTree& );  // placeholder, not implemented
    virtual ~GTree();

    virtual GTree* clone() const = 0;

    void clone_nodes_onto( GTree* newtree ) const;

    GTree** choose_random_tree( GTree** );
    int num_children() const;
    int max_depth() const;

    virtual void writeOn( std::ostream& os ) const;

    virtual void printOn( std::ostream& os, int indentlevel ) const;
    void printOn( std::ostream& os ) const;
    virtual void printSourceOn( std::ostream& os, int indentlevel ) const;
    virtual void printSourceOn( std::ostream& os ) const;
    virtual void emit( EScriptProgram& prog ) const = 0;
public:
    std::vector<GTree*> nodes; // parameters
private:
};


class GFunction : public GTree
{
    typedef GTree base;
public:
    GFunction( const GFunctionDef& funcdef );
    
    virtual GTree* clone() const;
    
    virtual void writeOn( std::ostream& os ) const;
    virtual void printOn( std::ostream& os, int indentlevel ) const;
    virtual void printSourceOn( std::ostream& os, int indentlevel ) const;
    virtual void emit( EScriptProgram& prog ) const;
    
    const GFunctionDef& funcdef;
	GFunction & operator=( const GFunction & ) { return *this; }
};


class GTerminal : public GTree
{
    typedef GTree base;
public:
    GTerminal( const GTerminalDef& termdef );

    virtual GTree* clone() const;
    
    virtual void writeOn( std::ostream& os ) const;
    virtual void printOn( std::ostream& os, int indentlevel ) const;
    void printSourceOn( std::ostream& os, int indentlevel ) const;
    virtual void emit( EScriptProgram& prog ) const;
    const GTerminalDef& termdef;
	GTerminal & operator=( const GTerminal & ) { return *this; }
};

class GDoubleTerminal : public GTerminal
{
    typedef GTerminal base;
public:
    GDoubleTerminal( const GDoubleTerminalDef& termdef );
    GDoubleTerminal( const GDoubleTerminalDef& termdef, double value );

    virtual GTree* clone() const;
    virtual void emit( EScriptProgram& prog ) const;
    virtual void writeOn( std::ostream& os ) const;
    virtual void printOn( std::ostream& os, int indentlevel ) const;
    void printSourceOn( std::ostream& os, int indentlevel ) const;

	GDoubleTerminal & operator=( const GDoubleTerminal & ) { return *this; }

private:
    double _value;
};



class GenDef
{
public:
    GenDef();
    virtual ~GenDef();

    enum SelectionMethod {
        SELECTION_TOURNAMENT,
        SELECTION_FITNESS_PROP
    };

    GTree* create_random_tree( int maxdepth,
                               bool popfull );

    GTree* read_tree( std::istream& );

    void add_terminal( GTerminalDef* terminaldef );
    void add_function( GFunctionDef* functiondef );

    float Pr; // 0 .. 1  -> probability of reproduction
    // Pc is 1-Pr 
    float max_fitness;
    SelectionMethod selection_method;
    bool use_parsimony;
    float parsimony_threshhold; // based on raw fitness 

protected:
    GTree* inner_read_tree( std::istream& is );
    GTree* create_node_by_tag( const std::string& tag, std::istream& is ) const;
    GTree* create_random_terminal();
    
    void create_function_args( GFunction* func, int maxdepth, bool popfull );
    GFunction* create_random_function( int maxdepth, bool popfull );
    GTree* create_random_function_or_terminal( int max_depth, bool popfull );
    GTree* create_random_tree( int maxdepth,
                               bool popfull,
                               bool isroot );

private:
    std::vector<GFunctionDef*> functions;
    std::vector<GTerminalDef*> terminals;
};




class GIndividual
{
public:
    GIndividual();
    GIndividual( GTree* );

    ~GIndividual();

    GTree* alg;
    bool raw_fitness_known;
    double raw_fitness;
    // NOTE, at this time, tournament selection is used, to only raw_fitness
    //   is used.
    mutable double standard_fitness;
    mutable double adjusted_fitness;
    mutable double normalized_fitness;

public:
    GIndividual( const GIndividual& );
    GIndividual& operator=( const GIndividual& );
};

class GPopulation
{
public:
    GPopulation( GenDef& gd );
    ~GPopulation();

    std::string name;
    typedef std::vector<GIndividual*> Individuals;
    Individuals individuals;

    void create_random_population( int size );
    unsigned size() const;

    GPopulation* breed_next_generation() const;
    void perform_reproduction( GPopulation* newpop ) const;
    void perform_crossover( GPopulation* newpop ) const;
    const GIndividual* choose_individual() const;
    void calculate_fitness_values() const;

    void write( const std::string& filename ) const;
    void read( const std::string& filename );
private:
    GPopulation( const GPopulation& );
    GPopulation& operator=( const GPopulation& );

    GenDef& gd;
};




#endif
