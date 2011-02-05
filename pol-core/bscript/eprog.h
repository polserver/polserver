/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#ifndef BSCRIPT_EPROG_H
#define BSCRIPT_EPROG_H

#include <iosfwd>
#include <string>
#include <vector>

#include "options.h"
#include "symcont.h"
#include "token.h"
#include "fmodule.h"

#include "../clib/refptr.h"

#include "executortype.h"

class CompilerContext;
class Executor;

class Instruction
{
public:


    Instruction( ExecInstrFunc f ) : token(), func(f), cycles(0)
    {
    }
    Instruction() : token(), func(0), cycles(0)
    {
    }
    
	Token token;
    ExecInstrFunc func;
    mutable unsigned int cycles;
};

struct EPDbgInstr
{
    unsigned int blockidx;
    unsigned int flags;
    unsigned int fileidx;
    unsigned int line;
    unsigned int srclineidx;
    unsigned int functionidx;
};

struct EPDbgBlock
{
    unsigned parentblockidx;
    unsigned parentvariables;
    vector< std::string > localvarnames;
};

struct EPExportedFunction
{
    std::string name;
    unsigned nargs;
    unsigned PC;
};

struct EPDbgFunction
{
    std::string name;
    unsigned firstPC;
    unsigned lastPC;
};

struct BSCRIPT_SECTION_HDR;

class EScriptProgram : public ref_counted
{
public:
    EScriptProgram();
    void addToken( const Token& token );
    void append( const StoredToken& stoken );
    void append( const StoredToken& stoken, unsigned* posn );
    void append( const StoredToken& stoken, const CompilerContext& ctx );
    void append( const StoredToken& stoken, const CompilerContext& ctx, unsigned* posn );
    void erase();

    std::string name;
    unsigned nglobals;
    void clear_modules();
    vector< FunctionalityModule* > modules;
    StoredTokenContainer tokens;
    SymbolContainer symbols;
    bool haveProgram;
    unsigned expectedArgs;

    void dump( std::ostream& os );
    void dump_casejmp( std::ostream& os, const Token& token );
    int write( const char *fname );
    int read( const char *fname );
    int read_dbg_file();
    int read_progdef_hdr( FILE *fp );
	int read_module( FILE* fp );
    int read_globalvarnames( FILE* fp );
    int read_exported_functions( FILE* fp, BSCRIPT_SECTION_HDR* hdr );
    int _readToken(Token& token, unsigned position) const;
    int create_instructions();

    int write_dbg( const char *fname, bool gen_txt );

    // compiler only:
        vector<std::string> sourcelines;
        vector<std::string> fileline;
        unsigned program_PC;
        std::string program_decl;
        vector<std::string> function_decls;

    vector<EPExportedFunction> exported_functions;

    // executor only:
    unsigned short version;
    vector<Instruction> instr;
    unsigned int invocations;
    u64 instr_cycles; // FIXME need an enable-profiling flag

    class Package const * pkg;

    // debug data:
    bool debug_loaded;
    vector< std::string > globalvarnames;
    vector< EPDbgBlock > blocks;
    vector< EPDbgFunction > dbg_functions;
    unsigned savecurblock;
    unsigned curblock;
    unsigned curfile;
    unsigned curline;
    bool statementbegin;
    vector< std::string > dbg_filenames;
    
// per instruction:
    vector< unsigned > dbg_filenum;
    vector< unsigned > dbg_linenum;
    vector< unsigned > dbg_ins_blocks;
    vector< bool > dbg_ins_statementbegin;
    void setcontext( const CompilerContext& ctx );
    void setstatementbegin();

    void enterfunction();
    void leavefunction();
    void enterblock();
    void leaveblock();
    void addlocalvar( const std::string& localvarname );
    void addfunction( std::string name, unsigned firstPC, unsigned lastPC );
    void update_dbg_pos( const Token& tkn );
    void add_ins_dbg_info();

    int add_dbg_filename( const std::string& filename );
    std::string dbg_get_instruction( unsigned atPC ) const;

    unsigned varcount( unsigned block );
    unsigned parentvariables( unsigned parent );
private:
    friend class EScriptProgramCheckpoint;
    ~EScriptProgram();
    friend class ref_ptr<EScriptProgram>;
};

class EScriptProgramCheckpoint
{
public:
    explicit EScriptProgramCheckpoint( const EScriptProgram& );
    void commit( const EScriptProgram& prog ); 
    void rollback( EScriptProgram& prog ) const; 

    unsigned module_count;
    unsigned tokens_count;
    unsigned symbols_length;
    unsigned sourcelines_count;
    unsigned fileline_count;
};
#endif
