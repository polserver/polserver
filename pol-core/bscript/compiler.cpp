/*
History
=======
2005/07/26 Shinigami: if you use 'Include ":blah:blubb";' eCompile will search for
                      ":blah:blubb.inc" and ":blah:include/blubb.inc". It will use
                      first file found. If both files exist eCompile will print a Warning.
2005/07/28 Shinigami: Assignment inside Condition Check will produce Warning on -v5 only
2005-09-07 Folko    : No longer warn about unused variables in BASIC style for loops
2005/09/08 Shinigami: Will warn about unused variables in BASIC style for loops on -v5 only
2005/09/25 Shinigami: BugFix inside FileCheck for multiple include of same File
                      e.g.: inside scripts\textcmd\test\textcmd.src:
                              Include "../../../pkg/std/housing/include/test";
                              Include ":housing:test";
                              Include ":housing:include/test";
                      will be handled as same file now
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef WIN32
#	pragma warning(disable:4786)
#endif

#include <string.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __unix__
#	include <unistd.h>
#endif

#include "../clib/clib.h"
#include "../clib/filecont.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "../plib/pkg.h"

#include "compctx.h"
#include "compilercfg.h"
#include "modules.h"
#include "tokens.h"
#include "symcont.h"
#include "operator.h"
#include "token.h"
#include "verbtbl.h"
#include "parser.h"
#include "userfunc.h"
#include "compiler.h"
#include "../clib/xmain.h"

#include "objmembers.h"

bool Compiler::check_filecase_;
int Compiler::verbosity_level_;

extern int include_debug;

string getpathof( const string fname )
{
    string tmp = fname;
    string::size_type pos = tmp.find_last_of( "\\/") ;
    if (pos == string::npos)
        return "./";
    else
        return tmp.substr( 0, pos+1 );
}


#if 1


bool Scope::varexists( const string& varname, unsigned& idx ) const
{
    for( int i = variables_.size()-1; i >= 0; --i )
    {
         if (stringicmp( varname, variables_[i].name ) == 0)
         {
             variables_[i].used = true;
             idx = i;
             return true;
         }
    }
    return false;
}

bool Scope::varexists( const string& varname ) const
{
    for( int i = variables_.size()-1; i >= 0; --i )
    {
         if (stringicmp( varname, variables_[i].name ) == 0)
         {
             return true;
         }
    }
    return false;
}

BlockDesc& Scope::pushblock()
{
    blockdescs_.push_back( BlockDesc() );
    return blockdescs_.back();
}

void Scope::popblock()
{
    BlockDesc& bd = blockdescs_.back();

    while (bd.varcount--)
    {
        Variable& bk = variables_.back();
        if (!bk.used && compilercfg.DisplayWarnings)
        {
            cout << "Warning: local variable '" << bk.name << "' not used." << endl;
            cout << bk.ctx;
        }
        variables_.pop_back();
    }

    blockdescs_.pop_back();
}

void Scope::addvar( const string& varname, const CompilerContext& ctx, bool warn_on_notused )
{
    for( unsigned i = variables_.size()-blockdescs_.back().varcount; i < variables_.size(); ++i )
    {
        if (stringicmp( varname, variables_[i].name ) == 0)
        {
           throw runtime_error( "Variable " + varname + " is already in scope." );
        }
    }
    Variable newvar;
    newvar.name = varname;
    newvar.ctx = ctx;
    newvar.used = !warn_on_notused;
    variables_.push_back( newvar );
    blockdescs_.back().varcount++;
}

void Scope::addvalue()
{
    blockdescs_.back().valcount++;
}

Expression::~Expression()
{
    while (!tokens.empty())
    {
        Token* tkn = tokens.back();
        tokens.pop_back();
        delete tkn;
    }


    while (!TX.empty())
    {
        Token* tkn = TX.top();
        TX.pop();
        delete tkn;
    }


    while (!CA.empty())
    {
        Token* tkn = CA.front();
        CA.pop();
        delete tkn;
    }
}

void Expression::eat( Expression& expr )
{
	while (!expr.CA.empty())
	{
		CA.push( expr.CA.front() );
		expr.CA.pop();
	}
}

void Expression::eat2( Expression& expr )
{
	while (!expr.tokens.empty())
	{
		CA.push( expr.tokens.front() );
		expr.tokens.erase( expr.tokens.begin() );
	}
}

Token* optimize_long_operation( Token* left, Token* oper, Token* right )
{
    long lval = 0;
    switch( oper->id )
    {
        case TOK_ADD:
            lval = left->lval + right->lval;
            break;
        case TOK_SUBTRACT:
            lval = left->lval - right->lval;
            break;
        case TOK_MULT:
            lval = left->lval * right->lval;
            break;
        case TOK_DIV:
            if (right->lval == 0)
                throw runtime_error( "Program would divide by zero" );
            lval = left->lval / right->lval;
            break;
        
        case TOK_EQUAL:
            lval = (left->lval == right->lval);
            break;
        case TOK_NEQ:
            lval = (left->lval != right->lval);
            break;
        case TOK_LESSTHAN:
            lval = (left->lval < right->lval);
            break;
        case TOK_LESSEQ:
            lval = (left->lval <= right->lval);
            break;
        case TOK_GRTHAN:
            lval = (left->lval > right->lval);
            break;
        case TOK_GREQ:
            lval = (left->lval >= right->lval);
            break;

        case TOK_AND:
            lval = (left->lval && right->lval);
            break;
        case TOK_OR:
            lval = (left->lval || right->lval);
            break;

		case TOK_BSRIGHT:
			lval = (left->lval >> right->lval );
			break;
		case TOK_BSLEFT:
			lval = (left->lval << right->lval );
			break;
        case TOK_BITAND:
            lval = (left->lval & right->lval );
            break;
        case TOK_BITOR:
            lval = (left->lval | right->lval );
            break;
        case TOK_BITXOR:
            lval = (left->lval ^ right->lval );
            break;

        default:
            return NULL;

            break;
    }

    Token* ntoken = new Token(*left);
    ntoken->lval = lval;
    return ntoken;
}

Token* optimize_double_operation( Token* left, Token* oper, Token* right )
{
    Token* ntoken = NULL;
    switch( oper->id )
    {
        case TOK_ADD:
            ntoken = new Token(*left);
            ntoken->dval = left->dval + right->dval;
            break;
        case TOK_SUBTRACT:
            ntoken = new Token(*left);
            ntoken->dval = left->dval - right->dval;
            break;
        case TOK_MULT:
            ntoken = new Token(*left);
            ntoken->dval = left->dval * right->dval;
            break;
        case TOK_DIV:
            ntoken = new Token(*left);
            if (right->dval == 0.0)
                throw runtime_error( "Program would divide by zero" );
            ntoken->dval = left->dval / right->dval;
            break;
        
        default:
            break;
    }
    return ntoken;
}


Token* optimize_string_operation( Token* left, Token* oper, Token* right )
{
    Token* ntoken = NULL;
    switch( oper->id )
    {
        case TOK_ADD:
            {
                ntoken = new Token(*left);
                string combined;
                combined = string(left->tokval()) + string(right->tokval());
                ntoken->copyStr( combined.c_str() );
            }
            break;
        
        default:
            break;
    }
    return ntoken;
}

Token* optimize_long_operation( Token* oper, Token* value )
{
    Token* ntoken = NULL;
    switch( oper->id )
    {
        case TOK_UNMINUS:
            ntoken = new Token(*value);
            ntoken->lval = -value->lval;
            break;
        case TOK_LOG_NOT:
            ntoken = new Token(*value);
            ntoken->lval = !value->lval;
            break;
        case TOK_BITWISE_NOT:
            ntoken = new Token(*value);
            ntoken->lval = ~ntoken->lval;
            break;

        default:
            break;
    }
    return ntoken;
}
Token* optimize_double_operation( Token* oper, Token* value )
{
    Token* ntoken = NULL;
    switch( oper->id )
    {
        case TOK_UNMINUS:
            ntoken = new Token(*value);
            ntoken->dval = -value->dval;
            break;
        
        default:
            break;
    }
    return ntoken;
}
Token* optimize_string_operation( Token* oper, Token* value )
{
    return NULL;
}

void Expression::optimize_binary_operations()
{
    for (unsigned i = 0; i < tokens.size(); i++)
    {
        Token* oper = tokens[i];
        if (oper->type != TYP_OPERATOR)
        {
            continue;
        }
        if (i < 2)
            throw runtime_error( "Unbalanced binary operator: " + tostring(*oper));

        Token* left = tokens[i-2];
        Token* right= tokens[i-1];
        if (left->id != right->id)
        {
			// only optimize operations on like operands
            continue;
        }
        if (left->id != TOK_LONG && left->id != TOK_STRING && left->id != TOK_DOUBLE)
        {   
            continue;
        }

        Token* ntoken = NULL;
        switch( left->id )
        {
            case TOK_LONG: 
                ntoken = optimize_long_operation( left, oper, right );
                break;
            case TOK_DOUBLE:
                ntoken = optimize_double_operation( left, oper, right );
                break;
            case TOK_STRING: 
                ntoken = optimize_string_operation( left, oper, right );
                break;


            default: break;
        }
        if (ntoken)
        {
            delete left;
            delete right;
            delete oper;
            tokens[ i-2 ] = ntoken;
            tokens.erase( tokens.begin() + (i-1), tokens.begin() + (i+1) );
        }
    }
}

void Expression::optimize_unary_operations()
{
    for (unsigned i = 0; i < tokens.size(); i++)
    {
        Token* oper  = tokens[i];
        if (oper->type != TYP_UNARY_OPERATOR)
        {      
            continue;
        }
        if (i < 1)
            throw runtime_error( "Unbalanced unary operator: " + tostring(*oper) );

        Token* value = tokens[i-1];

        if (oper->id == TOK_UNPLUS)
        {
            // unary plus does nothing.
            delete oper;
            tokens.erase( tokens.begin() + i, tokens.begin() + i + 1);
            continue;
        }
        if (value->id != TOK_LONG && value->id != TOK_STRING && value->id != TOK_DOUBLE)
        {   
            continue;
        }
        Token* ntoken = NULL;
        switch( value->id )
        {
            case TOK_LONG: 
                ntoken = optimize_long_operation( oper, value );
                break;
            case TOK_DOUBLE:
                ntoken = optimize_double_operation( oper, value );
                break;
            case TOK_STRING: 
                ntoken = optimize_string_operation( oper, value );
                break;
            default: break;
        }
        if (ntoken)
        {
            delete value;
            delete oper;
            tokens[ i-1 ] = ntoken;
            tokens.erase( tokens.begin() + i, tokens.begin() + (i+1) );
        }
    }
}

int Expression::get_num_tokens( int idx ) const
{
    Token* tkn = tokens[idx];
    int children = 0;

    if (tkn->type == TYP_OPERAND) // constant
    {
		// "anonymous" struct definitions inside array/dict/...
		if (tkn->id == INS_ADDMEMBER_ASSIGN) //struct{a:=1}
			children = 2;
		else if (tkn->id == INS_ADDMEMBER2) // struct{a}
			children = 1;
		else
			children = 0; // just myself
    }
	else if (tkn->id == INS_ADDMEMBER_ASSIGN)
	{
		children = 2;
	}
	else if (tkn->id == INS_DICTIONARY_ADDMEMBER)
	{
		children = 3;
	}
    else if (tkn->id == INS_MULTISUBSCRIPT)
    {
        children = 1 + tkn->lval;
    }
    else if (tkn->id == INS_MULTISUBSCRIPT_ASSIGN)
    {
        children = 1 + tkn->lval;
    }
    else if (tkn->type == TYP_OPERATOR) // binary operator
    {
        children = 2;
    }
    else if (tkn->type == TYP_UNARY_OPERATOR)
    {
        children = 1;
    }
    else if (tkn->type == TYP_FUNC)
    {
        children = tkn->userfunc->parameters.size(); 
    }
    else if (tkn->type == TYP_METHOD)
    {
        children = 1+tkn->lval;
    }
    else if (tkn->type == TYP_USERFUNC)
    {
        // the CTRL_JSR_USERFUNC
        // FIXME: TODO: what?
        children = 1;
    }
    else if (tkn->id == CTRL_JSR_USERFUNC)
    {
        // the CTRL_MAKELOCAL + the parameters
        children = 1 + tkn->userfunc->parameters.size();
    }
    else if (tkn->id == CTRL_MAKELOCAL)
    {
        children = 0;
    }
    else
    {
        passert_always( 0 );
    }
    int count = 1;
    for( int i = 0; i < children; ++i )
    {
        count += get_num_tokens( idx-count );
    }
    return count;
}

bool Expression::optimize_token( int i )
{
    Token* oper  = tokens[i];
    if (oper->type == TYP_OPERATOR && oper->id == TOK_ASSIGN)
    {
        int right_idx = i-1;
        int left_idx = right_idx-get_num_tokens(i-1);
        if (right_idx < 0 || left_idx < 0)
        {
            throw runtime_error( "Unbalanced operator: " + tostring(*oper) );
        }
        //Token* right = tokens[ right_idx ];
        Token* left = tokens[ left_idx ];
        if (left->id == TOK_ARRAY_SUBSCRIPT)
        {
            oper->id = INS_SUBSCRIPT_ASSIGN;
            oper->type = TYP_UNARY_OPERATOR;
            oper->lval = left->lval;
            delete left;
            tokens.erase( tokens.begin() + left_idx, tokens.begin() + (left_idx+1) );
            return true;
        }
        else if (left->id == INS_MULTISUBSCRIPT)
        {
            oper->id = INS_MULTISUBSCRIPT_ASSIGN;
            oper->type = TYP_UNARY_OPERATOR;
            oper->lval = left->lval;
            delete left;
            tokens.erase( tokens.begin() + left_idx, tokens.begin() + (left_idx+1) );
            return true;
        }
        else if (left->id == INS_GET_MEMBER)
        {
            oper->id = INS_SET_MEMBER;
            oper->type = TYP_UNARY_OPERATOR;
            oper->copyStr( left->tokval() );
            delete left;
            tokens.erase( tokens.begin() + left_idx, tokens.begin() + (left_idx+1) );
            return true;
        }
        else if (left->id == INS_GET_MEMBER_ID)
        {
            OSTRINGSTREAM os;
            os << left->lval;

            oper->id = INS_SET_MEMBER_ID;
            oper->type = TYP_UNARY_OPERATOR;
            oper->copyStr( OSTRINGSTREAM_STR(os).c_str() );
            oper->lval = left->lval;
            delete left;
            tokens.erase( tokens.begin() + left_idx, tokens.begin() + (left_idx+1) );
            return true;
        }
    }
    else if (oper->id == INS_ASSIGN_CONSUME)
    {
        int right_idx = i-1;
        int left_idx = right_idx-get_num_tokens(i-1);
        //Token* right = tokens[ right_idx ];
        Token* left = tokens[ left_idx ];
        if (left->id == TOK_LOCALVAR || left->id == TOK_GLOBALVAR)
        {
            // FIXME: assigning to a global/local, then consuming. we can do better, for this special case.
            if (left->id == TOK_LOCALVAR)
                oper->id = INS_ASSIGN_LOCALVAR;
            else if (left->id == TOK_GLOBALVAR)
                oper->id = INS_ASSIGN_GLOBALVAR;
            oper->type = TYP_UNARY_OPERATOR;
            oper->lval = left->lval;
            oper->copyStr( left->tokval() );
            delete left;
            tokens.erase( tokens.begin() + left_idx, tokens.begin() + left_idx + 1 );
            return true;
        }
    }
    else if (oper->id == TOK_CONSUMER)
    {
        Token* operand = tokens[ i-1 ];
        if (operand->id == TOK_ASSIGN)
        {
            operand->id = INS_ASSIGN_CONSUME;
            delete oper;
            tokens.pop_back();
            return true;
        }
        else if (operand->id == INS_SUBSCRIPT_ASSIGN)
        {
            operand->id = INS_SUBSCRIPT_ASSIGN_CONSUME;
            delete oper;
            tokens.pop_back();
            return true;
        }
        else if (operand->id == INS_SET_MEMBER)
        {
            operand->id = INS_SET_MEMBER_CONSUME;
            delete oper;
            tokens.pop_back();
            return true;
        }
        else if (operand->id == INS_SET_MEMBER_ID)
        {
            operand->id = INS_SET_MEMBER_ID_CONSUME;
            delete oper;
            tokens.pop_back();
            return true;
        }
    }
    else if (oper->id == TOK_MEMBER)
    {
        Token* operand = tokens[ i-1 ];
        if (operand->id == TOK_STRING /*|| operand->id == INS_CALL_METHOD*/)
        {
            ObjMember* objmemb = getKnownObjMember(operand->tokval());
            if(objmemb != NULL && compilercfg.OptimizeObjectMembers)
            {        
                // merge the member name with the member operator.
                oper->id = INS_GET_MEMBER_ID;
                oper->type = TYP_UNARY_OPERATOR;
                oper->lval = (long)objmemb->id;

                delete operand;
                tokens.erase( tokens.begin() + i-1, tokens.begin() + i );
                return true;
            }
            else
            {
                // merge the member name with the member operator.
                oper->id = INS_GET_MEMBER;
                oper->type = TYP_UNARY_OPERATOR;
                oper->copyStr( operand->tokval() );
                
                delete operand;
                tokens.erase( tokens.begin() + i-1, tokens.begin() + i );
                return true;
            }
        }
        else
        {
            throw std::runtime_error( "Expected an identifier to follow the member (.) operator." );
        }
    }
    return false;
}
void Expression::optimize_assignments()
{
    for (unsigned i = 1; i < tokens.size(); i++)
    {
        if (optimize_token( i ))
            return;
    }
}

/*
	TODO:
		add options to:
			a) disable all optimization
			b) disable optimization on doubles
		OPTIMIZATIONS PERFORMED:
			Constant Long Arithmetic:	(+ - * /)
				5 + 7   ->   5 7 +   ->  12
			Constant Double Arithmetic: (+ - * /)
				5.5 + 6.5  ->  5.5 6.5 +   -> 12.0
			String Concatenation:		(+)
				"hello" + " world" -> "hello" " world" + -> "hello world"
		OPTIMIZATIONS TO BE CONSIDERED:
			More efficient constructs: (+=, -=, *=, /=)
				A := A + 5 -> A A 5 + := -> A 5 +=
			Note, this is harder for things like:
				A[5] := A[5] + 4;
			which requires reversing your way up the expression
			and searching for like operands.

		Optimizations that must take place at a different level:
			0  IFFALSE -> GOTO
			1  IFFALSE -> NOP
			0  IF		-> NOP
			1  IF		-> GOTO
			34: GOTO 56   
			56: GOTO 78		--> 34: GOTO 78  56: GOTO 78
		NOP		-> (null) (compress NOPs)
*/
void Expression::optimize()
{
    unsigned starting_size;
    do 
    {

        starting_size = tokens.size();
        optimize_binary_operations();
        optimize_unary_operations();
        optimize_assignments();

    } while (tokens.size() != starting_size);
}

void Compiler::enterblock( eb_label_ok eblabel, eb_break_ok ebbreak, eb_continue_ok ebcontinue )
{

    program->enterblock();

    BlockDesc& bd = localscope.pushblock();
    bd.varcount = 0;
    bd.label_ok = eblabel;
    bd.break_ok = ebbreak;
    bd.continue_ok = ebcontinue;

	if (bd.label_ok == CanBeLabelled)
    {
        bd.label = latest_label;
        latest_label = "";
    }
    else
    {
        bd.label = "";
    }
}
void Compiler::enterblock( eb_label_ok et )
{
    enterblock( et, et?BreakOk:BreakNotOk, et?ContinueOk:ContinueNotOk );
}

void Compiler::patchblock_breaks( unsigned breakPC )
{
    // now, patch up the GOTO part of BREAK statements.
    // they each have a LEAVE_BLOCK appropriate to where they are.
    const BlockDesc& bd = localscope.blockdesc();
    for( BlockDesc::TokenAddrs::const_iterator itr = bd.break_tokens.begin();
         itr != bd.break_tokens.end();
         ++itr )
    {
        unsigned patchip = *itr;
        patchoffset( patchip, breakPC ); // program->tokens.next()
    }
}

void Compiler::patchblock_continues( unsigned continuePC )
{
    const BlockDesc& bd = localscope.blockdesc();
    for( BlockDesc::TokenAddrs::const_iterator itr = bd.continue_tokens.begin();
         itr != bd.continue_tokens.end();
         ++itr )
    {
        unsigned patchip = *itr;
        patchoffset( patchip, continuePC );
    }

}

void Compiler::emit_leaveblock()
{
    if (localscope.numVarsInBlock())
    {       // local variables were declared in this scope.  We need to kill 'em.
        program->append( StoredToken(Mod_Basic, 
                                    CTRL_LEAVE_BLOCK, 
                                    TYP_CONTROL, 
                                    localscope.numVarsInBlock()));
    }
}

void Compiler::leaveblock( unsigned breakPC, unsigned continuePC ) 
{

    emit_leaveblock();
    patchblock_breaks( breakPC );
    patchblock_continues( continuePC );

    localscope.popblock();
    program->leaveblock();
}

Compiler::Compiler(ostream& iCout) :
    SmartParser(iCout),
    current_file_path(""),
    inExpr(0),
    inFunction(0),
    haveProgram(false),
    compiling_include(false),
    programPos(0),
    nProgramArgs(0),
    program_ctx(),
    program_source(NULL),
    included(),
    referencedPathnames(),
    program( new EScriptProgram )
{
    setQuiet(1);
    err = PERR_NONE;
}

Compiler::~Compiler()
{
    while (!delete_these_arrays.empty())
    {
        char* s = delete_these_arrays.back();
        delete[] s;
        delete_these_arrays.pop_back();
    }
}


bool Compiler::globalexists( const string& varname, 
                             unsigned& idx,
                             CompilerContext* ctx ) const
{
    for( unsigned i = 0; i < globals_.size(); ++i )
    {
         if (stringicmp( varname, globals_[i].name ) == 0)
         {
             idx = i;
             if (ctx)
                 *ctx = globals_[i].ctx;
             return true;
         }
    }
    return false;
}

bool Compiler::varexists( const string& varname ) const
{
    unsigned idx;
    if (localscope.varexists( varname, idx ))
        return true;

    if (globalexists( varname, idx ))
        return true;

    return false;

}

int Compiler::isLegal(Token& token)
{
    if (inExpr && (token.id == TOK_ASSIGN)) 
    {
        if (verbosity_level_ >= 5)
        {
            cout << "Warning! possible incorrect assignment." << endl;
            cout << "Near: " << curLine << endl;
        }
    }

    return 1; // assignments valid everywhere.  back to simple parser
}

struct Candidate {
	Candidate(int module, int funcidx) : module(module), funcidx(funcidx) {}
	int module;
	int funcidx;
    ModuleFunction* modfunc;
}; 

int Compiler::isFunc(Token& token, ModuleFunction **pmf)
{
    typedef vector<Candidate> Candidates;
	Candidates candidates;
	string modulename;
	string funcname;

    if (const char *colon = strchr( token.tokval(), ':'))
	{
		string tmp( token.tokval(), colon );
		if (tmp.length() >= 9)
		{
			cout << "'" << tmp << "' is too long to be a module name." << endl;
			return -1;
		}
		modulename = tmp;
		funcname = string( colon+2 );
	}
	else
	{
		modulename = "";
		funcname = token.tokval();
	}

	for(unsigned i=0; i < program->modules.size(); i++) 
	{
		if (!modulename.empty() && 
			!(modulename == program->modules[i]->modulename)) // STLport doesn't like != here
		{
				continue;
		}

		int funcidx;
        if (program->modules[i]->isFunc( funcname.c_str(), pmf, &funcidx)) 
		{
			candidates.push_back( Candidate(i, funcidx) );
        }
    }

	if (candidates.size() == 0)
	{
		return 0;
	}
	else if (candidates.size() == 1)
	{
		token.module = static_cast<unsigned char>(candidates[0].module); // WAS module,we're using relative now
        token.type = TYP_FUNC;
        token.id = TOK_FUNC;
        token.lval = candidates[0].funcidx;
        token.userfunc = (*pmf)->uf;
        return 1;
	}
	else
	{
		cout << "Function '" << funcname << "' exists in more than module.  It must be qualified." << endl;
		for( Candidates::const_iterator itr = candidates.begin();
		     itr != candidates.end();
			 ++itr )
		{
			cout << "\t" << program->modules[ itr->module ]->modulename << endl;
		}

		return -1;
	}
}

void Compiler::addModule(FunctionalityModule* module)
{
    module->fillFunctionsByName();
    program->modules.push_back( module );
}

int Compiler::isUserFunc( Token& token, UserFunction **f)
{
	if (token.id != TOK_IDENT) return 0;
	passert( token.tokval());

    UserFunctions::iterator itr = userFunctions.find( token.tokval() );
    if (itr != userFunctions.end())
    {
		token.module = Mod_Basic;
		token.type = TYP_USERFUNC;
        token.id = TOK_USERFUNC;
        token.userfunc = &(*itr).second;
		*f = &(*itr).second;
		return 1;
	}
	return 0;
}
int Compiler::getArrayElements( Expression& expr, CompilerContext& ctx )
{
    int res;
    Token token;
    res = peekToken( ctx, token );
    if (res < 0)
        return res;
    if (token.id != TOK_LPAREN)
        return 0;
    getToken( ctx, token );

        // it's valid to have a right-paren immediately following, so check for that:

    res = peekToken( ctx, token );
    if (res < 0)
        return res;
    if (token.id == TOK_RPAREN)
    {
        getToken( ctx, token );
        return 0;
    }


    for (;;)
    {
        res = peekToken( ctx, token );
        if (res < 0)
            return res;
		/*
			if we get an rparen HERE, it means the script has something like
				var x := array ( 2, 3, );
			report this as an error.
		*/
		if (token.id == TOK_RPAREN)
        {
            cout << "Expected expression following comma before right-brace in array initializer list" << endl;
            return -1;
        }
        if (token.id == TOK_COMMA)
        {
            cout << "Unexpected comma in array initializer list" << endl;
            return -1;
        }
        Expression eex;
        res = readexpr( eex, ctx, EXPR_FLAG_COMMA_TERM_ALLOWED | 
                     EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED );
        if (res < 0)
            return res;
        
        expr.eat2( eex );
        
        expr.CA.push( new Token( TOK_INSERTINTO, TYP_OPERATOR ) );

        res = getToken( ctx, token );
        if (res < 0) return res;
        if (token.id == TOK_COMMA)
        {
            continue;
        }
        else if (token.id == TOK_RPAREN)
        {
            return 0;;
        }
        else
        {
            cout << "Token '" << token << "' unexpected in array initializer list" << endl;
            return -1;
        }
    }
}

int Compiler::getNewArrayElements( Expression& expr, CompilerContext& ctx )
{
    int res;
    Token token;
    // the left-brace has already been eaten
    
    // it's valid to have a right-brace immediately following, so check for that:

    res = peekToken( ctx, token );
    if (res < 0)
        return res;
    if (token.id == TOK_RBRACE)
    {
        getToken( ctx, token );
        return 0;
    }
    
    for (;;)
    {
        res = peekToken( ctx, token );
        if (res < 0)
            return res;

        // if we get an rbrace HERE, it means the script has something like
        // var x := array { 2, 3, };
        // report this as an error.
        if (token.id == TOK_RBRACE)
        {
            cout << "Expected expression following comma before right-brace in array initializer list" << endl;
            return -1;
        }
        // we're expecting an expression, not a comma, at this point
        if (token.id == TOK_COMMA)
        {
            cout << "Unexpected comma in array initializer list" << endl;
            return -1;
        }
        Expression eex;
        res = readexpr( eex, ctx, EXPR_FLAG_COMMA_TERM_ALLOWED | 
                     EXPR_FLAG_RIGHTBRACE_TERM_ALLOWED );
        if (res < 0)
            return res;
        
        expr.eat2( eex );
        
        expr.CA.push( new Token( TOK_INSERTINTO, TYP_OPERATOR ) );

        // the element can be followed by a comma, or by a rightbrace. eat either.
        res = getToken( ctx, token );
        if (res < 0) return res;
        if (token.id == TOK_COMMA)
        {
            continue;
        }
        else if (token.id == TOK_RBRACE)
        {
            return 0;
        }
        else
        {
            cout << "Token '" << token << "' unexpected in array initializer list" << endl;
            return -1;
        }
    }
}

int Compiler::getStructMembers( Expression& expr, CompilerContext& ctx )
{
    int res;
    Token token;
    res = peekToken( ctx, token );
    if (res < 0)
        return res;
    if (token.id != TOK_LBRACE)
        return 0;
    getToken( ctx, token );

    // it's valid to have a right-brace immediately following, so check for that:
    res = peekToken( ctx, token );
    if (res < 0)
        return res;
    if (token.id == TOK_RBRACE)
    {
        getToken( ctx, token );
        return 0;
    }


    for (;;)
    {
        res = peekToken( ctx, token );
        if (res < 0)
            return res;
        // if we get an rbrace HERE, it means the script has something like
        // var x := struct { x, y, };
        // report this as an error.
        if (token.id == TOK_RBRACE)
        {
            cout << "Expected expression following comma before right-brace in struct initializer list" << endl;
            return -1;
        }
        if (token.id == TOK_COMMA)
        {
            cout << "Unexpected comma in struct element list" << endl;
            return -1;
        }

        if (token.id == TOK_IDENT || token.id == TOK_STRING)
        {
            Token ident_tkn;
            Parser::getToken( ctx, ident_tkn );
            
            res = peekToken( ctx, token );
            if (token.id == TOK_ASSIGN)
            {
                getToken( ctx, token );
                // something like struct { a := 5 };
                Expression eex;
                res = readexpr( eex, ctx, EXPR_FLAG_COMMA_TERM_ALLOWED | 
                            EXPR_FLAG_RIGHTBRACE_TERM_ALLOWED );
                if (res < 0)
                    return res;

                Token* addmem = new Token( ident_tkn );
                addmem->id = INS_ADDMEMBER_ASSIGN;
                
                expr.eat2( eex );
                expr.CA.push( addmem );

            }
			else if (token.id == TOK_EQUAL1)
			{
				cout << "Unexpected token: '" << token << "'. Did you mean := for assign?" << endl;
				return -1;
			}
            else
            {
                Token* addmem = new Token( ident_tkn );
                addmem->id = INS_ADDMEMBER2;
                expr.CA.push( addmem );
            }
        }
        else
        {
            cout << "Unexpected token in struct initializer list: " << token << endl;
            return -1;
        }


        res = getToken( ctx, token );
        if (res < 0) return res;
        if (token.id == TOK_COMMA)
        {
            continue;
        }
        else if (token.id == TOK_RBRACE)
        {
            return 0;
        }
        else
        {
            cout << "Token '" << token << "' unexpected in struct initializer list" << endl;
            return -1;
        }
    }
    // unreachable
}

int Compiler::getDictionaryMembers( Expression& expr, CompilerContext& ctx )
{
    int res;
    Token token;
    res = peekToken( ctx, token );
    if (res < 0)
        return res;
    if (token.id != TOK_LBRACE)
        return 0;
    getToken( ctx, token );

    // it's valid to have a right-brace immediately following, so check for that:
    res = peekToken( ctx, token );
    if (res < 0)
        return res;
    if (token.id == TOK_RBRACE)
    {
        getToken( ctx, token );
        return 0;
    }


    for (;;)
    {
        res = peekToken( ctx, token );
        if (res < 0)
            return res;
        // if we get an rbrace HERE, it means the script has something like
        // var x := dictionary { "x", "y", };
        // report this as an error.
        if (token.id == TOK_RBRACE)
        {
            cout << "Expected expression following comma before right-brace in dictionary initializer list" << endl;
            return -1;
        }
        if (token.id == TOK_COMMA)
        {
            cout << "Unexpected comma in dictionary element list" << endl;
            return -1;
        }


        // first get the key expression.

        Expression key_expression;
        res = readexpr( key_expression, ctx, 
                        EXPR_FLAG_COMMA_TERM_ALLOWED | 
                        EXPR_FLAG_DICTKEY_TERM_ALLOWED |
                        EXPR_FLAG_RIGHTBRACE_TERM_ALLOWED );
        if (res < 0)
            return res;
        
        expr.eat2( key_expression );

        // if the key is followed by "->", then grab the value
        res = peekToken( ctx, token );
        if (res < 0)
            return res;
        if (token.id == TOK_DICTKEY)
        {
            getToken( ctx, token );
            // get the value expression

            Expression value_expression;
            res = readexpr( value_expression, ctx, EXPR_FLAG_COMMA_TERM_ALLOWED | 
                        EXPR_FLAG_RIGHTBRACE_TERM_ALLOWED );
            if (res < 0)
                return res;
            
            expr.eat2( value_expression );
        }
        else
        {
            // an uninit
            expr.CA.push( new Token( INS_UNINIT, TYP_OPERAND ) );
        }
        
        expr.CA.push( new Token( INS_DICTIONARY_ADDMEMBER, TYP_OPERATOR ) );

        res = getToken( ctx, token );
        if (res < 0) return res;
        if (token.id == TOK_COMMA)
        {
            continue;
        }
        else if (token.id == TOK_RBRACE)
        {
            return 0;
        }
        else
        {
            cout << "Token '" << token << "' unexpected in struct element list" << endl;
            return -1;
        }
    }
    // unreachable
}

int Compiler::getMethodArguments( Expression& expr, CompilerContext& ctx, int& nargs )
{
    int res;
    Token token;
    nargs = 0;
    res = getToken( ctx, token );
    passert( token.id == TOK_LPAREN );
    passert( res == 0 );
    for (;;)
    {
        res = peekToken( ctx, token );
        if (res < 0)
            return res;
        if (token.id == TOK_RPAREN)
        {
            getToken( ctx, token );
            return 0;
        }
        if (token.id == TOK_COMMA)
        {
            cout << "Unexpected comma in array element list" << endl;
            return -1;
        }
        Expression eex;
        res = readexpr( eex, ctx, EXPR_FLAG_COMMA_TERM_ALLOWED | 
                     EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED );
        if (res < 0)
            return res;
        
        expr.eat2( eex );
        
        ++nargs;

        res = getToken( ctx, token );
        if (res < 0) return res;
        if (token.id == TOK_COMMA)
        {
            continue;
        }
        else if (token.id == TOK_RPAREN)
        {
            return 0;;
        }
        else
        {
            cout << "Token '" << token << "' unexpected in array element list" << endl;
            return -1;
        }
    }
    // unreachable
}


/* 
getUserArgs is called from IIP.
	It is being converted from calling IIP (and thus stuffing
	expression data onto ex) to calling readexpr() and
	appending the argument expressions onto the passed
	ex object.

Overall function:
	Read the parameters actually passed.
	Append expressions for parameters to passed expression.
	Append a JSR instruction
 */

/*
class ParamPassed
{
public:
	ParamPassed( const string& name ) : name(name) {}

	string name;
	Expression expr;
};
*/

int Compiler::getUserArgs( Expression& ex, CompilerContext& ctx, bool inject_jsr )
{
	int res;
	Token token;

	typedef map<string, Expression> ParamList;
    ParamList params_passed;
    //vector<string> func_params;

	int any_named = 0;
	UserFunction *userfunc = userfunc_;

	getToken(ctx, token);
	if (token.id != TOK_LPAREN) 
    {
		cout << "Expected '(' after function name '" << userfunc->name << "'" << endl; 
		res = -1;
		err = PERR_MISSLPAREN;
		return -1;
	}
	
	for (;;)
	{
        string varname;
		Token tk;
		
        CompilerContext tctx(ctx);

		res = getToken( tctx, tk );
		if (res < 0) return res;
		if (tk.id == TOK_RPAREN)
		{
			if ( params_passed.size() == 0)
            {
				break;
            }
			else
			{
				cout << "right paren not allowed here\n";
				return -1;
			}
		}

		if (params_passed.size() >= userfunc->parameters.size())
		{
			cout << "Too many parameters passed to " << userfunc->name << endl;
			return -1;
		}

		if (tk.id == TOK_IDENT)
		{
			Token tk2;
			res = getToken( tctx, tk2 );
			if (res < 0) return res;
			if (tk2.id == TOK_ASSIGN)
			{
				any_named = 1;
				varname = tk.tokval();
				ctx = tctx; /* skip past the 'variable :=' part */
			}
			else if (tk2.id == TOK_EQUAL1)
			{
				cout << "Unexpected token: '" << tk2 << "'. Did you mean := for assign?" << endl;
				return -1;
			}
		}
		if (varname == "")
		{
			if (any_named)
			{
				cout << "unnamed args cannot follow named args" << endl;
				return -1;
			}
			varname = userfunc->parameters[ params_passed.size() ].name;
		}
			// FIXME case sensitivity!
		if (params_passed.find( varname ) != params_passed.end())
		{
			cout << "Variable " << varname << " passed more than once to " << userfunc->name << endl;
			return -1;
		}
		
		Expression& arg_expr = params_passed[ varname ];

		res = IIP( arg_expr, 
                   ctx, 
                   EXPR_FLAG_COMMA_TERM_ALLOWED | 
                     EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED );
		if (res < 0) return res;
		
		Token tmp;
		res = peekToken( ctx, tmp );
		if (res) return res;
		if (tmp.id == TOK_COMMA)
		{
			getToken( ctx, tmp );
		}
		else if (tmp.id == TOK_RPAREN)
		{
			break;
		}
		else
		{
			cout << "Token '" << token << "' unexpected (expected comma or right-paren)\n";
			return -1;
		}

	}

    for( UserFunction::Parameters::const_iterator itr = userfunc->parameters.begin();
		 itr != userfunc->parameters.end();
		 ++itr )
	{
		if (params_passed.find( itr->name ) == params_passed.end()) // not passed
		{
			if (itr->have_default)
			{
				ex.CA.push( new Token( itr->dflt_value ) );
			}
			else
			{
				cout << "Function " << userfunc->name << ": Parameter " << itr->name 
					<< " was not passed, and there is no default." << endl;
				return -1;
			}
		}
		else
		{
			Expression& arg_expr = params_passed[ itr->name ];
			ex.eat( arg_expr );
			params_passed.erase( itr->name );
		}
	}

    if (!params_passed.empty())
    {
        for( ParamList::iterator itr = params_passed.begin(); itr != params_passed.end(); ++itr )
        {
            cout << "Parameter '" << (*itr).first << "' passed by name to " 
                 << userfunc->name << ", which takes no such parameter." << endl;
        }

        return -1;
    }
	passert( params_passed.empty() );

	getToken(ctx, token);
	if (token.id != TOK_RPAREN) 
    {
		res = -1;
		err = PERR_MISSRPAREN;
		return -1;
	}

	if (inject_jsr)
    {
        Token* t = new Token( CTRL_MAKELOCAL, TYP_CONTROL );
        t->dbg_filenum = ctx.dbg_filenum;
        t->dbg_linenum = ctx.line;
        ex.CA.push( t );
        t = new Token( Mod_Basic, CTRL_JSR_USERFUNC, TYP_CONTROL, userfunc );
        t->dbg_filenum = ctx.dbg_filenum;
        t->dbg_linenum = ctx.line;
	    ex.CA.push( t );
    }

	return 0;
}

void Compiler::addToken(Token& token)
{
    program->addToken( token );
}

void Compiler::convert_variables( Expression& expr ) const
{
    for( Expression::Tokens::const_iterator itr = expr.tokens.begin();
         itr != expr.tokens.end();
         ++itr )
    {
        Token* tkn = *itr;
        if (tkn->id == TOK_IDENT)
        {
            unsigned idx;
            if (localscope.varexists( tkn->tokval(), idx ))
            {
                tkn->id = TOK_LOCALVAR;
                tkn->lval = idx;
            }
            else if (globalexists( tkn->tokval(), idx ))
            {
                tkn->id = TOK_GLOBALVAR;
                tkn->lval = idx;
            }
        }
    }
}


int Compiler::validate( const Expression& expr, CompilerContext& ctx ) const
{
    for (unsigned i = 0; i < expr.tokens.size(); i++)
    {
        Token* tkn = expr.tokens[i];

        if (tkn->id == TOK_IDENT)
        {
            if (!varexists( tkn->tokval() ))
            {
                cout << "Variable " << tkn->tokval() << " has not been declared"
						<< " on line " << ctx.line
						<< "." << endl;
                return -1;
            }
        }
		else if (tkn->id == TOK_EQUAL1)
		{
			// Single '=' sign? Special error statement (since it could be a typo?)
			cout << "Deprecated '=' found: did you mean '==' or ':='?" << endl;
			return -1;
		}

		if (tkn->deprecated)
		{
			cout << "Warning: Found deprecated "
					<< (tkn->type == TYP_OPERATOR ? "operator " : "token ")
					<< "'" << tkn->tokval() << "'"
					<< " on line " << ctx.line
					<< " of " << ctx.filename << endl;
			// warning only; doesn't bail out.
		}

        if (tkn->type == TYP_OPERATOR)
        {
            int right_idx = i-1;
            if (right_idx < 0)
            {
                throw runtime_error( "Unbalanced operator: " + tostring(*tkn) );
            }

            int left_idx = right_idx-expr.get_num_tokens(i-1);
            if (left_idx < 0)
            {
                throw runtime_error( "Unbalanced operator: " + tostring(*tkn) );
            }
        }

        if (tkn->type == TYP_UNARY_OPERATOR)
        {
            int operand_idx = i-1;
            if (operand_idx < 0)
            {
                throw runtime_error( "Unbalanced operator: " + tostring(*tkn) );
            }
        }
    }

    return 0;
}

bool Compiler::substitute_constant( Token* tkn ) const
{
    Constants::const_iterator srch = constants.find( tkn->tokval() );
    if (srch != constants.end())
    {
        int dbg_filenum = tkn->dbg_filenum;
        int dbg_linenum = tkn->dbg_linenum;
        *tkn = (*srch).second;
        tkn->dbg_filenum = dbg_filenum;
        tkn->dbg_linenum = dbg_linenum;
        return true;
    }
    else
    {
        return false;
    }
}

void Compiler::substitute_constants( Expression& expr ) const
{
    for( Expression::Tokens::const_iterator itr = expr.tokens.begin();
         itr != expr.tokens.end();
         ++itr )
    {
        Token* tkn = *itr;

        if (tkn->id == TOK_IDENT)
            substitute_constant( tkn );
    }
}

int Compiler::readexpr( Expression& expr, 
                        CompilerContext& ctx, 
                        unsigned flags )
{
    int res;
    reinit(expr);
    res = IIP( expr, ctx, flags );
    if (res != 1)
        return res;
    while (!expr.CA.empty())
    {
        Token *token = expr.CA.front();
        expr.CA.pop();
        expr.tokens.push_back( token );
    }
    if ((flags & EXPR_FLAG_CONSUME_RESULT) && !expr.tokens.empty())
    {
        Token* tkn = new Token( TOK_CONSUMER, TYP_UNARY_OPERATOR );
        expr.tokens.push_back( tkn );
    }
    substitute_constants( expr );
    convert_variables( expr );
    expr.optimize();
    res = validate( expr, ctx );
    if (res != 0)
        return -1;
    return 1;
}

void Compiler::inject( Expression& expr )
{
    for( Expression::Tokens::const_iterator itr = expr.tokens.begin();
         itr != expr.tokens.end();
         ++itr)
    {
        addToken( *(*itr) );
    }
}

int Compiler::getExpr(CompilerContext& ctx, unsigned flags, unsigned* exprlen, Expression* pex)
{
    int res;
    if (pex)
    {
        res = readexpr( *pex, ctx, flags );
        if (exprlen != NULL)
            *exprlen = pex->tokens.size();
        inject( *pex );
    }
    else
    {
        Expression ex;
        res = readexpr( ex, ctx, flags );
        if (exprlen != NULL)
            *exprlen = ex.tokens.size();
        inject( ex );
    }
    return res;
}

int Compiler::getExpr2(CompilerContext& ctx, unsigned expr_flags, Expression* pex)
{
	int orig_inExpr;
    int res;
	orig_inExpr = inExpr;
    inExpr = 1;
    res = getExpr(ctx, expr_flags, NULL, pex);
    inExpr = orig_inExpr;
    return res;
}

/*
getExpr3
	get an expression, must be contained in parenthesis.
*/
int Compiler::getExprInParens(CompilerContext& ctx, Expression* pex)
{
    Token token;
    getToken(ctx, token);
    if (token.id != TOK_LPAREN) 
    {
        err = PERR_MISSLPAREN;
        return -1;
    }

	int res = getExpr2(ctx, EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED, pex);
    if (res < 0) return res;

	err = PERR_NONE;
    getToken(ctx, token);
    if (token.id != TOK_RPAREN) 
    {
        err = PERR_MISSRPAREN;
        return -1;
    }
    return 0;
}

int Compiler::getSimpleExpr( CompilerContext& ctx )
{
    Token token;
    int res = peekToken( ctx, token );
    if (res < 0)
        return res;
    if (token.id == TOK_LPAREN)
    {
        res = getExprInParens( ctx );
        if (res < 0)
            return res;
    }
    else if (token.id == TOK_IDENT || 
             token.id == TOK_FUNC ||
             token.id == TOK_USERFUNC || 
             token.id == TOK_ARRAY ||
             token.id == TOK_LBRACE )
    {
        res = getExpr2( ctx, EXPR_FLAG_SINGLE_ELEMENT );
        if (res < 0)
            return res;
    }
    else
    {
        cout << "Expected variable, function or parenthesized expression, got '" << token << "'" << endl;
        return -1;
    }
    return 0;
}

int Compiler::eatToken( CompilerContext& ctx, BTokenId tokenid )
{
    Token token;
    int res = getToken(ctx, token);
    if (res < 0)
    {
        cout << ctx;
        cout << "Error reading token, expected " << Token( tokenid, TYP_RESERVED ) << endl;
        return res;
    }

    if (token.id != tokenid) 
	{
        cout << ctx;
        cout << "Expected " << Token(tokenid,TYP_RESERVED) << ", got " << token << endl;
        return -1;
    }
    return 0;
}

int Compiler::handleDoClause(CompilerContext& ctx, int level)
{
    if (!quiet) cout << "DO clause.." << endl;

	unsigned body_start = program->tokens.next();
    enterblock( CanBeLabelled );

	Token endblock_tkn;
    int res;
    res = readblock(ctx, level, RSV_DOWHILE, NULL, &endblock_tkn );
    if (res < 0) return res;

	emit_leaveblock();

	program->update_dbg_pos( endblock_tkn );
		// continue should jump to where the WHILE expression is evaluated.
	patchblock_continues( program->tokens.next() );
	program->setstatementbegin();

    res = getExpr(ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED );
	if (res < 0) return res;

			// do-while loops until its expression evaluates to false.
    program->append(StoredToken(Mod_Basic, RSV_JMPIFTRUE, TYP_RESERVED, body_start));

		// break should completely exit, of course.
	patchblock_breaks( program->tokens.next() );

	localscope.popblock();
	program->leaveblock();

	return 0;
}


int Compiler::handleRepeatUntil(CompilerContext& ctx, int level)
{
    if (!quiet) cout << "REPEAT clause.." << endl;

    unsigned body_start = program->tokens.next();
    enterblock( CanBeLabelled );

    Token endblock_tkn;
    int res;
    res = readblock( ctx, level, RSV_UNTIL, NULL, &endblock_tkn );
    if (res < 0)
    {
        return res;
    }
    
    emit_leaveblock();
        
    program->update_dbg_pos( endblock_tkn );
        // continue should jump to where the UNTIL expression is evaluated.
    patchblock_continues( program->tokens.next() );
    program->setstatementbegin();
    
    res = getExpr(ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED );
    if (res < 0) return res;

        // repeat-until loops until its expression evaluates to true.
    program->append(StoredToken(Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, body_start));

        // break should completely exit, of course.
    patchblock_breaks( program->tokens.next() );
    
    localscope.popblock();
    program->leaveblock();

	return 0;
}

int Compiler::handleSwitch( CompilerContext& ctx, int level )
{
	int res = getExprInParens(ctx); // (expr) (parens required)
    if (res < 0) return res;

    unsigned default_posn = 0;
    unsigned casecmp_posn = 0;
    vector<unsigned> jmpend;
    vector<unsigned char> caseblock;

    program->append( StoredToken(Mod_Basic, INS_CASEJMP, TYP_RESERVED, 0), &casecmp_posn );

    // overview: 
        // we grab some case OPTIONs, 
        //  then we grab some code.
        //  until we peek-see an OPTION or an ENDCASE
    bool done = false;    
	bool onlydefault = true;
    while (!done)
    {
        bool anycases = false;
        for (;;)
        {
            Token token;
            res = peekToken( ctx, token );
            if (res < 0)
                return res;
            if (token.id == TOK_LONG || token.id == CTRL_LABEL || token.id == TOK_STRING)
            {
            /*
				Okay, some trickery.  First, we'll handle the 'default' label, 'cause
				it isn't tricky.
				The complication is that sometimes a label is actually a constant.
            */
                if (token.id == CTRL_LABEL && 
                    stricmp( token.tokval(), "default" ) == 0)
                {
                    getToken( ctx, token );
                    if (default_posn != 0)
                    {
                        cout << "CASE statement can have only one DEFAULT clause." << endl;
                        return -1;
                    }
                    default_posn = program->tokens.next();
                    anycases = true;
                    continue;
                }

            /*
				A label may be a constant, say CONST_VAR:  If it is, substitute that value.
			*/
                if (token.id == CTRL_LABEL)
                {
                    substitute_constant( &token );
				/*
					A label that wasn't a constant can't be used in a CASE statement.
					If it's followed by a while, etc, it won't be flagged illegal, though.
					this may be a Bad Thing.
				*/
                    if (token.id == CTRL_LABEL)
                    {
                        break;
                    }
                    Token dummy; // don't overwrite the token we just substituted
                    getToken( ctx, dummy );
                }
                else if (token.id == TOK_LONG || token.id == TOK_STRING)
                {
				/*
					If we read a Long or a String, it'll be followed by a colon.
					If it was a label, the colon is gone.
				*/
                    getToken( ctx, token );
                    
                    res = eatToken( ctx, RSV_COLON );
                    if (res < 0) return res;
                }

                anycases = true;
				onlydefault = false;
                if (token.id == TOK_LONG)
                {
                    unsigned short offset = static_cast<unsigned short>(program->tokens.next());
                    unsigned char* tmppch = reinterpret_cast<unsigned char*>(&offset);
                    caseblock.push_back( tmppch[0] );
                    caseblock.push_back( tmppch[1] );
                    caseblock.push_back( CASE_TYPE_LONG ); // FIXME hardcoded
                    tmppch = reinterpret_cast<unsigned char*>(&token.lval);
                    caseblock.push_back( tmppch[0] );
                    caseblock.push_back( tmppch[1] );
                    caseblock.push_back( tmppch[2] );
                    caseblock.push_back( tmppch[3] );
                }
                else if (token.id == CTRL_LABEL || token.id == TOK_STRING)
                {
                    if (strlen( token.tokval() ) >= 254)
                    {
                        cout << "String expressions in CASE statements must be <= 253 characters." << endl;
                        return -1;;
                    }
                    unsigned short offset = static_cast<unsigned short>(program->tokens.next());
                    unsigned char* tmppch = reinterpret_cast<unsigned char*>(&offset);
                    caseblock.push_back( tmppch[0] );
                    caseblock.push_back( tmppch[1] );
                    caseblock.push_back( static_cast<unsigned char>(strlen( token.tokval() )) );
                    const char* str = token.tokval();
                    int len = strlen( str );

                    for( int i = 0; i < len; ++i )
                        caseblock.push_back( str[i] );
                }
            }
            else if (token.id == RSV_ENDSWITCH && anycases) // only accept if OPTIONs exist!
            {
                getToken( ctx, token );
                done = true;
                break;
            }
            else
            {       // something else.  we'll assume a statement.
                break;
            }
        }
        if (done)
            break;


        // we've grabbed the OPTIONs.  Now grab the code, until we get to an OPTION or ENDCASE
        enterblock(CanBeLabelled,BreakOk,ContinueNotOk);
        while (ctx.s[0])
        {
            Token token;
            res = peekToken( ctx, token );
            if (res < 0)
                return res;
            if (token.id == RSV_ENDSWITCH)
            {
				// Only accept ENDCASE if OPTIONs exist!
				if (anycases) {
	                getToken( ctx, token );
		            done = true;
			        break;
				}
            }
            else if (token.id == TOK_LONG)
            {
                break;
            }
            else if (token.id == CTRL_LABEL)
            {
                if (stricmp( token.tokval(), "default" ) == 0)
                    break;

                substitute_constant( &token );
                if (token.id != CTRL_LABEL)
                    break;

            }
            else if (token.id == TOK_STRING)
            {
                break;
            }

			// we're about to grab code.  there needs to have been at least one OPTION, then.
			if (!anycases)
			{
				cout << "CASE statement with no options!" << endl
					 << "Found '" << token.tokval() << "'";
				if ( token.id == CTRL_LABEL )
					cout << " but no such constant is defined." << endl;
				else
					cout << " prematurely." << endl;
				return -1;
			}

			res = getStatement( ctx, level );
            if (res < 0)
                return res;
        }
         // NOTE, the "jump to -> jump to" optimizer will be helpful here, optimizing breaks.
        
        emit_leaveblock();
        patchblock_breaks( program->tokens.next() );

        localscope.popblock();
        program->leaveblock();

        if (!done)
        {
            unsigned jmpend_posn;
            program->append( StoredToken(Mod_Basic,RSV_GOTO,TYP_RESERVED,0), &jmpend_posn );
            jmpend.push_back( jmpend_posn ); 
        }
    }

    //patchblock_breaks( program->tokens.next() );
    //program->leaveblock();

		// if only a 'default' block was defined, print a warning
	if (onlydefault)
	{
		cout << "Warning: CASE block only has a DEFAULT clause defined." << endl;
		cout << "near: " << curLine << endl;
		cout << ctx;
	}

        // if no default specified, pretend 'default' was specified at the end.
    if (default_posn == 0)
        default_posn = program->tokens.next();

        // the default case must go at the end.
//    if (1)
    {
        unsigned char* tmppch = reinterpret_cast<unsigned char*>(&default_posn);
        caseblock.push_back( tmppch[0] );
        caseblock.push_back( tmppch[1] );
        caseblock.push_back( CASE_TYPE_DEFAULT );
    }

    while (!jmpend.empty())
    {
        unsigned posn = jmpend.back();
        jmpend.pop_back();
        patchoffset( posn, program->tokens.next() );
    }

    // now, we have to emit the casecmp block.
    unsigned caseblock_posn;
    unsigned char* casecmp_raw = new unsigned char[ caseblock.size() ];
    for( unsigned i = 0; i < caseblock.size(); ++i )
        casecmp_raw[i] = caseblock[i];
    program->symbols.append( casecmp_raw, caseblock.size(), caseblock_posn );
    delete[] casecmp_raw;
    patchoffset( casecmp_posn, caseblock_posn );
    return 0;
}

int Compiler::handleForEach(CompilerContext& ctx, int level)
{
    Token itrvar;

    int res;
    CompilerContext foreach_ctx( ctx );

    res = getToken( ctx, itrvar );
    if (res < 0)
        return res;
    if (itrvar.id != TOK_IDENT)
    {
        cout << "FOREACH iterator must be an identifier, got " << itrvar << endl;
        return res;
    }

    res = eatToken( ctx, TOK_IN) ;
    if (res < 0)
        return res;

		/*
			The outer block is a hidden block.  It can't be labelled, but the
			inner one can.  This way, 'break' and 'continue' won't touch
			the iterator variable, expression, and counter that we have in
			this hidden block.
		*/

    res = getSimpleExpr( ctx );
    if (res < 0)
        return res;
		/*
			When these are evaluated, the value stack should look like this:
			(result of EXPR)
		*/
    enterblock(CanNotBeLabelled);

    unsigned initforeach_posn;
    program->append( StoredToken( Mod_Basic, INS_INITFOREACH, TYP_RESERVED, 0 ),
                             &initforeach_posn );
			/*
				INITFOREACH creates three local variables, placeholders for the iterator,
				expression, and counter.  Only the iterator can be accessed, for now.
			*/
    program->addlocalvar( itrvar.tokval() );
    localscope.addvar( itrvar.tokval(), foreach_ctx );
    program->addlocalvar( "_" + string(itrvar.tokval()) + "_expr" );
    localscope.addvar( "_" + string(itrvar.tokval()) + "_expr", foreach_ctx, false );
    program->addlocalvar( "_" + string(itrvar.tokval()) + "_counter" );
    localscope.addvar( "_" + string(itrvar.tokval()) + "_iter", foreach_ctx, false );

    
    unsigned iter_posn = program->tokens.next();
    enterblock(CanBeLabelled);
    Token endforeach_token;
    res = readblock(ctx, level, RSV_ENDFOREACH, NULL, &endforeach_token);
    if (res < 0) return res;
    
    emit_leaveblock();
    
    unsigned stepforeach_posn;
    program->update_dbg_pos( endforeach_token );
    program->append( StoredToken( Mod_Basic, INS_STEPFOREACH, TYP_RESERVED, iter_posn ), &stepforeach_posn );
    patchoffset( initforeach_posn, stepforeach_posn );

    patchblock_continues( stepforeach_posn );
    patchblock_breaks( program->tokens.next() );
    localscope.popblock();
    program->leaveblock();

    // FIXME this isn't right - continue needs to refer to one block, while break
    // needs to refer to another!
    // ie continue should use the inner block, while break should use the outer.
    leaveblock(0,0);
    return 0;
}


int Compiler::handleReturn(CompilerContext& ctx)
{
    Token token;
    int res = peekToken(ctx, token);
    if (res) return res;
    if (token.id == TOK_SEMICOLON) 
	{
        getToken(ctx, token);
        unsigned posn = 0;
        program->symbols.append("", posn);
        program->append( StoredToken(Mod_Basic, TOK_STRING, TYP_OPERAND, posn) );
    } 
	else 
	{
        res = getExpr2(ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED);
        if (res<0) return res;
    }

    if (res<0) 
        return res;
    if (inFunction)
    {
        program->append( StoredToken(Mod_Basic, RSV_RETURN, TYP_RESERVED, 0), 0);
    }
    else
    {
        program->append( StoredToken(Mod_Basic, CTRL_PROGEND, TYP_CONTROL, 0), 0);
    }
    return 0;
}

int Compiler::handleExit(CompilerContext& ctx)
{
    Token token;
    getToken(ctx, token);
    if (token.id != TOK_SEMICOLON) 
    {
        cout << "Missing ';'" << endl;
        err = PERR_MISSINGDELIM;
        return -1;
    }

    program->append( StoredToken(Mod_Basic, RSV_EXIT, TYP_RESERVED, 0) );
    return 0;
}

int Compiler::handleBlock(CompilerContext& ctx, int level)
{
    Token token;

    if (!quiet) cout << "BEGIN block.." << endl;
    
    enterblock( CanNotBeLabelled ); 

    while (ctx.s[0]) 
    {
         peekToken(ctx, token);
         if (token.id == RSV_ENDB) break;

         if (getStatement(ctx, level)==-1) return -1;
    }
    if (!ctx.s[0]) return -1;

    getToken(ctx, token);
    if (token.id != RSV_ENDB) 
    {
        return -1;
    }
    
    
    leaveblock( 0, 0 );

    return 0;
}

int Compiler::readFunctionDeclaration( CompilerContext& ctx, 
                                       UserFunction& userfunc
                                        )
{
	Token token;
    Token funcName;
    int res;

    userfunc.ctx = ctx;

    res = getToken(ctx, funcName);
    if (res) return res;
    bool first_time = true;
    if (first_time)
    {
        if (funcName.id != TOK_IDENT) 
	    {
            if (funcName.id == TOK_FUNC)
            {
                cout << "'" << funcName.tokval() << "' is already defined as a function." << endl;
                cout << "Near: " << curLine << endl;
                cout << ctx;
                return -1;
            }
            else
            {
                cout << "Expected an identifier, got " << funcName << " instead." << endl;
                cout << "Near: " << curLine << endl;
                cout << ctx;
                return -1;
            }
        }
    }
	userfunc.name = funcName.tokval();
    Token lparen;
    res = getToken(ctx, lparen);
    if (res) return res;
    if (lparen.id != TOK_LPAREN) 
	{
        return -1;
    }
/*	
	We have eaten the left paren.  Next follows:
	RIGHT_PAREN, OR  ( [refto] varname [:= default ] { COMMA or RIGHT_PAREN })
*/
    peekToken(ctx, token);
	for (;;) {
		Token paramName;
		res = getToken(ctx, token);
		if (res) return -1;
        
        bool pass_by_reference = false;

		if (token.id == TOK_RPAREN) break;
        if (token.id == TOK_REFTO)
        {
            pass_by_reference = true;
            res = getToken(ctx,token);
            if (res) return -1;
        }
		if (token.id != TOK_IDENT) 
        {
			return -1;
		}
		userfunc.parameters.resize( userfunc.parameters.size() + 1 );
        UserParam& param = userfunc.parameters.back();
		param.name = token.tokval();
        param.pass_by_reference = pass_by_reference;
		peekToken(ctx, token);
		if (token.id == TOK_ASSIGN)
		{
			// We have a default argument.
			param.have_default = 1;
			getToken( ctx, token ); // Eat the assignment operator
		
            Expression ex;
            if (readexpr( ex, ctx, EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED|EXPR_FLAG_COMMA_TERM_ALLOWED ) != 1)
            {
                cout << "Error reading expression in const declaration" << endl;
                return -1;
            }

            if (ex.tokens.size() != 1)
            {
                cout << "Const expression must be optimizable" << endl;
                return -1;
            }

            param.dflt_value = *(ex.tokens.back());
			if (param.dflt_value.type != TYP_OPERAND)
			{
				cout << "[" << funcName.tokval() << "]: Only simple operands are allowed as default arguments (" << token << " is not allowed)" << endl;
				return -1;
			}
			peekToken(ctx, token);
		}
		else if (token.id == TOK_EQUAL1)
		{
			cout << "Unexpected token: '" << token << "'. Did you mean := for assign?" << endl;
			return -1;
		}
		else
		{
			param.have_default =0;
		}
		
		if (token.id == TOK_COMMA)  
        {
			getToken(ctx, token);
			continue;
		} 
        else if(token.id == TOK_RPAREN) 
        {
            continue;
        }
		else return -1;
	}
    return 0;
}

int Compiler::handleDeclare(CompilerContext& ctx)
{
    int res;
    Token token;
    res = getToken(ctx, token);
    if (res) return res;
    if (token.id != RSV_FUNCTION) 
    {
        return -1;
    }
    Token funcName;
    res = getToken(ctx, funcName);
    if (res) return res;
    if (funcName.id != TOK_IDENT) 
    {
        throw runtime_error( "Tried to declare a non-identifier" );
        return -1;
    }
    Token lparen;
    res = getToken(ctx, lparen);
    if (res) return res;
    if (lparen.id != TOK_LPAREN) 
    {
        return -1;
    }
    int nParams = 0;
    for (;;)
    {
        res = getToken(ctx, token);
        if (res) return -1;
        if (token.id == TOK_RPAREN) 
        {
            break;
        }
        if (token.id != TOK_IDENT) 
        {
            return -1;
        }
        nParams++;
        peekToken(ctx, token);
        if (token.id == TOK_COMMA) 
        {
            getToken(ctx, token);
            continue;
        } 
        else if(token.id == TOK_RPAREN) 
        {
            continue;
        }
        else 
        {
            return -1;
        }
    }
    getToken(ctx, token);
    if (token.id != TOK_SEMICOLON) 
    {
        err = PERR_MISSINGDELIM;
        return -1;
    }
    cout << "func decl: " << curLine << endl;
    cout << "nParams: " << nParams << endl;
    //addUserFunc(funcName.tokval(), nParams);
    return 0;
}

int Compiler::handleIf(CompilerContext& ctx, int level)
{
    unsigned if_begin;
    unsigned jump_false;
    Token token;
    if_begin = program->tokens.next();
    jump_false = 0;
    if (!quiet) cout << "if clause.." << endl;


    int res = getExprInParens(ctx); // (expr) (parens required)
    if (res < 0) return res;

    unsigned if_token_posn;
    program->append(
         StoredToken(Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, 0),
         &if_token_posn);

        // THEN is optional, currently.
    peekToken(ctx, token);
    if (token.id == RSV_THEN) 
        getToken(ctx, token); // 'then'
    if (!quiet) cout << "then clause.." << endl;

        // get the part we do
    res = getStatement(ctx, level);
    if (res < 0)
        return res;

        // if an ELSE follows, grab the ELSE and following statement
    peekToken(ctx, token);
    if (token.id == RSV_ELSE) 
	{
        unsigned else_token_posn;
              // this GOTO makes execution skip the ELSE part if the IF was true
        program->append(
            StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, 0),
            &else_token_posn);
        jump_false = program->tokens.next();
        getToken(ctx, token);  // eat the else
        if (!quiet) cout << "else clause.." << endl;
        getStatement(ctx, level);
                // now that we know where the ELSE part ends, patch in the address
                // which skips past it.
        program->tokens.atPut1(
             StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, program->tokens.next()),
             else_token_posn);
    } 
	else 
	{
		jump_false = program->tokens.next();
	}
 

    // patch up orig. IF token to skip past if false
    program->tokens.atPut1(
          StoredToken(Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, jump_false),
          if_token_posn);
    return 0;
}
#endif
#if 1 

bool mismatched_end( const Token& token, BTokenId correct )
{
    if (token.id == correct)
    {
        return false;
    }
    else if (token.id == RSV_ENDFOREACH ||
        token.id == RSV_ENDIF ||
      //token.id == RSV_ENDB ||
		token.id == RSV_DOWHILE ||
        token.id == RSV_ENDWHILE ||
        token.id == RSV_UNTIL ||
        token.id == RSV_ENDFOR ||
        token.id == RSV_ENDFUNCTION ||
        token.id == RSV_ENDSWITCH ||
        token.id == RSV_ENDPROGRAM ||
        token.id == RSV_ENDENUM)
    {
        Token t(correct, TYP_RESERVED);
        cout << "Expected " << t << " before " << token << endl;
        return true;
    }
    else
    {
        return false;
    }
}

int Compiler::handleBracketedIf(CompilerContext& ctx, int level)
{
    CompilerContext save_ctx( ctx );

    vector<unsigned> jumpend;

    Token token;
    if (!quiet) cout << "if clause.." << endl;

    token.id = RSV_ST_IF;
    
    EScriptProgramCheckpoint checkpt( *program );
    unsigned jumpend_size = jumpend.size();

    bool discard_rest = false;
    bool discarded_all = true;
    bool included_any_tests = false;
    unsigned last_if_token_posn = static_cast<unsigned>(-1);
    unsigned if_token_posn = static_cast<unsigned>(-1);
    while (token.id == RSV_ST_IF || token.id == RSV_ELSEIF)
    {
        EScriptProgramCheckpoint checkpt_expr( *program );
//dump(cout);
        program->setstatementbegin();
        Expression ex;
        int res = getExprInParens(ctx, &ex); // (expr) (parens required)
        if (res < 0) return res;
//dump(cout);        
        bool patch_if_token = true;
        last_if_token_posn = if_token_posn;
        if (ex.tokens.back()->id == TOK_LOG_NOT)
        {
            if_token_posn = program->tokens.count()-1;
            program->tokens.atPut1(
                 StoredToken(Mod_Basic, RSV_JMPIFTRUE, TYP_RESERVED, 0),
                 if_token_posn);
        }
        else
        {
            program->append(
                 StoredToken(Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, 0),
                 &if_token_posn);
        }

        bool discard_this = discard_rest;
    
        if (!discard_rest && ex.tokens.size() == 1 && ex.tokens[0]->id == TOK_LONG)
        {
            if (ex.tokens[0]->lval)
            {
                discard_rest = true;
            }
            else
            {
                discard_this = true;
            }
            rollback( *program, checkpt_expr ); // don't need the expression or the jump,
                                          // even if we're keeping the block
            patch_if_token = false;
        }
        else
        {
            discarded_all = discard_rest;
            if (!discard_rest)
                included_any_tests = true;
        }



        // THEN is optional, currently.
        peekToken(ctx, token);
        if (token.id == RSV_THEN) 
            getToken(ctx, token); // 'then'
        if (!quiet) cout << "then clause.." << endl;

//dump(cout);
        // get the part we do
        enterblock( CanNotBeLabelled );
        while (ctx.s[0])
        {
            peekToken( ctx, token );
            if (token.id == RSV_ELSEIF || token.id == RSV_ELSE || token.id == RSV_ENDIF)
            {
                break;
            }

            res = getStatement(ctx, level);
            if (res < 0)
            {
                cout << "Error in IF statement starting at " 
                     << save_ctx;

                return res;
            }
        }
        leaveblock( 0, 0);
        if (!ctx.s[0]) return -1;
        
//dump(cout);
        if (!discard_this)
        {
            checkpt.commit( *program );
            jumpend_size = jumpend.size();
        }
//dump(cout);
        //dump(cout);

        // this will be committed only when the next ELSEIF or ELSE is committed
        if (token.id == RSV_ELSEIF || token.id == RSV_ELSE)
        {
            unsigned temp_posn;
            program->update_dbg_pos( token );
            program->append(
                    StoredToken( Mod_Basic, RSV_GOTO, TYP_RESERVED, 0 ),
                    &temp_posn );
            jumpend.push_back( temp_posn );
            
            if (token.id == RSV_ELSEIF)
            {
                readCurLine(ctx);
                savesourceline();
                getToken( ctx, token );
            }
        }
        if (patch_if_token)
        {
            StoredToken tkn;
            program->tokens.atGet1( if_token_posn, tkn );
            tkn.offset = static_cast<unsigned short>(program->tokens.next());
            program->tokens.atPut1( tkn, if_token_posn );
        }

//dump(cout);
        if (discard_this)
        {
            rollback( *program, checkpt );
            while (jumpend.size() > jumpend_size)
                jumpend.pop_back();

            if (last_if_token_posn != static_cast<unsigned>(-1) &&
                last_if_token_posn < program->tokens.count())
            {
                StoredToken tkn;
                program->tokens.atGet1( last_if_token_posn, tkn );
                tkn.offset = static_cast<unsigned short>(program->tokens.next());
                program->tokens.atPut1( tkn, last_if_token_posn );
            }
        }
//dump(cout);
    }

    peekToken(ctx, token);
    if (token.id != RSV_ENDIF && token.id != RSV_ELSE)
    {
        cout << "Expected ELSE or ENDIF after IF statement starting at " << save_ctx;
        cout << "Did not expect: " << token << endl;
        return -1;
    }

    // if an ELSE follows, grab the ELSE and following statement
    if (token.id == RSV_ELSE) 
	{
        getToken(ctx, token);  // eat the else
        if (!quiet) cout << "else clause.." << endl;
        enterblock( CanNotBeLabelled );
        while (ctx.s[0])
        {
            peekToken( ctx, token );
            if (token.id == RSV_ENDIF)
                break;
            if (mismatched_end(token, RSV_ENDIF))
                return -1;

            int res = getStatement(ctx, level);
            if (res < 0)
                return res;
        }
        leaveblock(0,0);
    } 
    // eat the ENDIF
    if (!ctx.s[0]) return -1;
    getToken( ctx, token );
    if (token.id != RSV_ENDIF) return -1;

    if (discard_rest && !included_any_tests)
    {
        rollback( *program, checkpt );
        while (jumpend.size() > jumpend_size)
            jumpend.pop_back();
    }
    

    while (!jumpend.empty())
    {
        unsigned pc = jumpend.back();
        jumpend.pop_back();
        // patch up orig. IF token to skip past if false
        
        program->tokens.atPut1(
              StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, program->tokens.next()),
              pc);
    }

    return 0;
}


int Compiler::readblock( CompilerContext& ctx, 
                         int level, 
                         BTokenId endtokenid, 
                         BTokenId* last_statement_id,
                         Token* pBlockEndToken )
{
    CompilerContext tctx( ctx );
    int res;
    while (ctx.s[0])
    {
        Token token;
        res = peekToken( ctx, token );
        if (res < 0)
            return res;
        if (token.id == endtokenid)
        {
            if (pBlockEndToken != NULL)
                getToken( ctx, *pBlockEndToken );
            else
                getToken( ctx, token ); // eat the end-token
            return 0;
        }
        if (last_statement_id != NULL)
            *last_statement_id = token.id;
        res = getStatement(ctx, level);
        if (res < 0)
            return res;
    }
    cout << "Error in block beginning at " << tctx;
    cout << "End-of-File detected, expected '" << Token( Mod_Basic, endtokenid, TYP_RESERVED ) << "'" << endl;
    return -1;
}

int Compiler::handleBracketedWhile(CompilerContext& ctx, int level)
{
    if (!quiet) cout << "while clause.." << endl;

    unsigned conditional_expr_posn = program->tokens.next();
    int res = getExprInParens(ctx); // (expr) (parens required)
    if (res < 0) return res;

    unsigned test_expr_token_posn;
    program->append(
         StoredToken(Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, 0),
         &test_expr_token_posn);

    enterblock( CanBeLabelled );
    
    Token endblock_tkn;
    res = readblock( ctx, level, RSV_ENDWHILE, NULL, &endblock_tkn );
    if (res < 0)
        return res;
    
    program->update_dbg_pos( endblock_tkn );
    emit_leaveblock();
        
	    // jump back to conditional expression
    program->append(StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED,
         conditional_expr_posn), 0);

        // Control should jump past the loop when the expr evaluates to false.
    unsigned exit_loop_posn = program->tokens.next();
    program->tokens.atPut1( StoredToken(Mod_Basic, 
                                        RSV_JMPIFFALSE, 
                                        TYP_RESERVED, 
                                        exit_loop_posn),
                            test_expr_token_posn);
    
        // continues re-test the expression, which is at the top of the loop.         
    patchblock_continues( conditional_expr_posn );
        // breaks exit the loop, which has been wholly emitted by now.
    patchblock_breaks( program->tokens.count() );

    localscope.popblock();
    program->leaveblock();

    return 0;
}

/*
allowed formats for declare:
	local a;
	local a array;
	local a := 5;
	local a := expr;
	local a, b;
	local a, b := expr, ...;
*/
int Compiler::handleVarDeclare( CompilerContext& ctx, unsigned save_id )
{
    CompilerContext savectx(ctx);
	Token tk_varname, tk_delim;
	int done = 0;
    if (save_id == RSV_GLOBAL && !inGlobalScope())
    {
        cout << "Globals can only be declared at global scope." << endl;
        return -1;
    }
    if (save_id == RSV_LOCAL && inGlobalScope())
    {
        cout << "Locals can only be declared within a block or function." << endl;
        return -1;
    }

    do {
		/*
			formats: varname followed by comma or semicolon 
				varname followed by "array", then by comma/semicolon
				varname followed by ':=', then an initializer, then comma/semicolon.
		*/
        CompilerContext thisctx(ctx);

	    getToken(ctx, tk_varname);
		if (tk_varname.id != TOK_IDENT)
		{
            cout << "Non-identifier declared as a variable: '" << tk_varname.tokval() << "'" << endl;
            cout << "Token: " << tk_varname << endl;
			return -1;
		}

        if (constants.find( tk_varname.tokval() ) != constants.end())
        {
            cout << tk_varname.tokval() << " is already a defined constant." << endl;
            return -1;
        }
		
		// Add this variable to the current block/scope 
		int varindex = 0;
        
        if (inGlobalScope())
        {
            unsigned idx;
            CompilerContext gctx;
            if (!globalexists( tk_varname.tokval(), idx, &gctx ))
            {   
                Variable v;
                v.name = tk_varname.tokval();
                v.used = true;
                v.ctx = savectx;

                varindex = globals_.size();
                globals_.push_back( v );
                program->globalvarnames.push_back( tk_varname.tokval() );
            }
            else
            {
                cout << "Global Variable '" << tk_varname.tokval() << "' is already declared at " << gctx;
                return -1;
            }
        }
        else
        {
            unsigned idx;
            if (compilercfg.DisplayWarnings && globalexists( tk_varname.tokval(), idx ))
            {
                cout << "Warning: Local variable '" << tk_varname.tokval() 
                        << "' hides Global variable of same name." << endl;
                cout << ctx;

            }
            varindex = localscope.numVariables();
            program->addlocalvar( tk_varname.tokval() );

            varindex = localscope.numVarsInBlock();
            localscope.addvar( tk_varname.tokval(), ctx );
        }

		// grab the comma, semicolon, := or array declare token
        getToken( ctx, tk_delim );

		// note save_id is RSV_LOCAL or RSV_GLOBAL
		program->append( StoredToken( Mod_Basic, save_id, TYP_RESERVED, varindex ), thisctx );
		
		if (tk_delim.id == TOK_ARRAY)
		{
			// declaring an array.
            if (compilercfg.DisplayWarnings)
            {
                cout << "Warning! Deprecated array-declaration syntax used." << endl;
                cout << ctx;
            }
			program->append( StoredToken( Mod_Basic, INS_DECLARE_ARRAY, TYP_RESERVED, 0 ) );
			
			getToken( ctx, tk_delim );
		}
		else if (tk_delim.id == TOK_ASSIGN) 
		{
			int res;
		    res = getExpr(ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED|
                             EXPR_FLAG_COMMA_TERM_ALLOWED );
			if (res <= 0) return res;
			program->append( StoredToken(Mod_Basic, TOK_ASSIGN, TYP_OPERATOR,0) );

			getToken( ctx, tk_delim );
		}
	    program->append( StoredToken(Mod_Basic, TOK_CONSUMER, TYP_UNARY_OPERATOR, 0) );

		if (tk_delim.id == TOK_COMMA)
        {
			continue;
        }
		else if (tk_delim.id == TOK_SEMICOLON)
        {
			break;
        }
		else if (tk_delim.id == TOK_EQUAL1)
		{
			cout << "Unexpected token: '" << tk_delim << "'. Did you mean := for assign?" << endl;
			return -1;
		}
		else
        {
            cout << "Unexpected token: " << tk_delim << endl;
			return -1;
        }
	} while (!done);

	return 0;
	
	// insert a consumer to eat the evaluated result from the expr.
    program->append( StoredToken(Mod_Basic, TOK_CONSUMER, TYP_UNARY_OPERATOR, 0) );
	return 0;

}

/*
allowed formats for declaring const:
	const a := expr;
*/
int Compiler::handleConstDeclare( CompilerContext& ctx )
{
	Token tk_varname, tk_assign;
	//int done = 0;


		/*
			formats:	varname followed by comma or semicolon 
						varname followed by "array", then by comma/semicolon
						varname followed by ':=', then an initializer, then comma/semicolon.
        */
	    getToken(ctx, tk_varname);
		if (tk_varname.id != TOK_IDENT)
		{
            cout << "Expected identifier after const declaration" << endl;
			return -1;
		}
        
        if (constants.count( tk_varname.tokval() ))
        {
            cout << "Constant " << tk_varname << " has already been defined." << endl;
            return -1;
        }

        // grab the := token
        getToken( ctx, tk_assign );
        if (tk_assign.id != TOK_ASSIGN)
        {
            cout << "Expected := after identifier in const declaration" << endl;
            return -1;
        }

		Expression ex;
        if (readexpr( ex, ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED ) != 1)
        {
            cout << "Error reading expression in const declaration" << endl;
            return -1;
        }

        if (ex.tokens.size() != 1)
        {
            cout << "Const expression must be optimizable" << endl;
            return -1;
        }

        constants.insert( Constants::value_type( tk_varname.tokval(), *ex.tokens.back() ) );
        delete ex.tokens.back();
        ex.tokens.pop_back();

	return 0;
}

/*
allowed formats for declaring enum
	enum FOO
	const a := expr;
*/
int Compiler::handleEnumDeclare( CompilerContext& ctx )
{
	Token tk_enum_tag;

    // First, grab the enum tag.
    // TODO: validate it isn't already used

    if (getToken( ctx, tk_enum_tag )<0)
    {
        cout << "Error reading enum tag" << endl;
        return -1;
    }

    if (tk_enum_tag.id != TOK_IDENT)
    {
        cout << "Expected an enum tag after 'enum'" << endl;
        return -1;
    }
    
    long next_counter = 0;
    for (;;)
    {
        Token tk_varname, tk_assign;
	   // int done = 0;
        if (getToken( ctx, tk_varname ) < 0)
        {
            cout << "Error reading identifier in enum declaration" << endl;
            return -1;
        }

        if (tk_varname.id == RSV_ENDENUM)
            return 0;
        
        if (tk_varname.id != TOK_IDENT)
        {
            cout << "Expected identifier in enum statement, got " << tk_varname << endl;
            return -1;
        }

        Token tmp;
        // now, the forms.  THis should be followed by a comma, an 'endenum', or a ':='
        if (peekToken( ctx, tmp ) < 0)
        {
            cout << "Error reading token in enum statement" << endl;
            return -1;
        }
        if (tmp.id == TOK_ASSIGN)
        {
            Token tmp;
            getToken( ctx, tmp );
		    Expression ex;
               // FIXME doesn't work if expression is right before enum
            if (readexpr( ex, ctx, EXPR_FLAG_COMMA_TERM_ALLOWED|EXPR_FLAG_ENDENUM_TERM_ALLOWED) != 1)
            {
                cout << "Error reading expression in enum declaration" << endl;
                return -1;
            }
            if (!peekToken( ctx, tmp )) 
            {       // might be a comma, or an endenum
                if (tmp.id == TOK_COMMA)
                    getToken( ctx, tmp );
            }
            if (ex.tokens.size() != 1)
            {
                cout << "Enum expression must be optimizable" << endl;
                return -1;
            }
            Token* tkn = ex.tokens.back();
            if (tkn->id == TOK_LONG)
                next_counter = tkn->lval + 1;
            else
                ++next_counter;
            constants.insert( Constants::value_type( tk_varname.tokval(), *tkn ) );
            delete tkn;
            ex.tokens.pop_back();
        }
        else if (tmp.id == TOK_COMMA)
        {
            getToken( ctx, tmp );
            Token tkn( TOK_LONG, TYP_OPERAND );
            tkn.lval = next_counter++;
            constants.insert( Constants::value_type( tk_varname.tokval(), tkn ) ); 
        }
        else if (tmp.id == RSV_ENDENUM)
        {
            Token tkn( TOK_LONG, TYP_OPERAND );
            tkn.lval = next_counter++;
            constants.insert( Constants::value_type( tk_varname.tokval(), tkn ) );
            // we'll pick this one up next pass
        }
		else if (tmp.id == TOK_EQUAL1)
		{
			cout << "Unexpected token: '" << tmp << "'. Did you mean := for assign?" << endl;
			return -1;
		}
		else
        {
            cout << "Unexpected token " << tmp << " in enum statement" << endl;
            return -1;
        }
    }
}


int Compiler::useModule( const char *modulename )
{

    for( unsigned idx = 0; idx < program->modules.size(); idx++ )
    {
        if (modulename == program->modules[idx]->modulename)
            return 0;
    }

    auto_ptr<FunctionalityModule> compmodl(new FunctionalityModule( modulename ));

    string filename_part = modulename;
	filename_part += ".em";

    string filename_full = current_file_path + filename_part;

    if (verbosity_level_ >= 10)
        cout << "Searching for " << filename_full << endl;

    if (!FileExists( filename_full.c_str() ))
    {
        string try_filename_full = compilercfg.ModuleDirectory + filename_part;
        if (verbosity_level_ >= 10)
            cout << "Searching for " << try_filename_full << endl;
        if (FileExists( try_filename_full.c_str() ))
        {
            if (verbosity_level_ >= 10)
                cout << "Found " << try_filename_full << endl;
            //cout << "Using " << try_filename << endl;
            filename_full = try_filename_full;
        }
    }
    else
    {
        if (verbosity_level_ >= 10)
            cout << "Found " << filename_full << endl;
    }

    char *orig_mt;
    char *mt;
    
    if (getFileContents( filename_full.c_str(), &orig_mt ))
    {
		cout << "Unable to find module " << modulename << endl;
		cout << "\t(Filename: " << filename_full << ")" << endl;
        return -1;
    }
    
    mt = orig_mt;
    CompilerContext mod_ctx( filename_full, program->add_dbg_filename(filename_full), mt );

    string save = current_file_path;
    current_file_path = getpathof( filename_full );

    int res = -1;
    for(;;)
    {
        Token tk_dummy;
        res = peekToken( mod_ctx, tk_dummy );
        if (res < 0)
        {
            cout << "Error reading token in module " << modulename << endl;
            free(orig_mt);
            break;
        }
        else if (res == 1)
        {
            addModule( compmodl.release() );
            free(orig_mt);
            res = 0;
            break;
            
        }
        if (tk_dummy.id == RSV_CONST)
        {
            getToken( mod_ctx, tk_dummy );
            int res = handleConstDeclare(mod_ctx);
            if (res < 0)
                break;
            else
                continue;
        }
        auto_ptr<UserFunction> puserfunc( new UserFunction );
        if (readFunctionDeclaration( mod_ctx, *puserfunc ))
        {
            cout << "Error reading function declaration in module " << modulename << endl;
            free( orig_mt );
            res = -1;
            break;
        }
        
        Token tk_semicolon;
        if (getToken( mod_ctx, tk_semicolon ))
        {
            cout << filename_full << ": Error in declaration for " << puserfunc->name << ":" << endl
                 << "  Expected a semicolon, got end-of-file or error" << endl;

            free( orig_mt );
            res = -1;
            break;
        }
        if (tk_semicolon.id != TOK_SEMICOLON)
        {
            cout << filename_full << ": Error in declaration for " << puserfunc->name << ":" << endl
                 << "  Expected a semicolon, got '" << tk_semicolon << "'" << endl;
            free( orig_mt );
            res = -1;
            break;
        }
    
        UserFunction* uf = puserfunc.release();
        compmodl->addFunction( uf->name.c_str(), 
                               uf->parameters.size(),
                               uf );
    }
    current_file_path = save;
    return res;
}

/*
Format: 
	Use module; 

	Consider: at the beginning of program parsing, peek at the first token.
	While it's a 'Use', handle it.  This way, from looking at the top of a program, you
	can tell what modules it uses - a simple way for someone to guage a script's
	capabilities / risks, from a security standpoint.
*/
int Compiler::handleUse( CompilerContext& ctx )
{
    Token tk_module_name, tk_semicolon;

    if (getToken( ctx, tk_module_name ))
    {
        cout << "Error in USE statement: USE should be followed by a module name." << endl;
        return -1;
    }
	if (tk_module_name.id != TOK_IDENT)
	{
        cout << "Error in USE statement: Expected identifier, got '"
             << tk_module_name << "'" << endl;
		return -1;
	}

    if (getToken( ctx, tk_semicolon ))
    {
        cout << "Error in USE statement (module " << tk_module_name << "): "
             << "Expected ';', got end-of-file or error"
             << endl;
        return -1;
    }
    if (tk_semicolon.id != TOK_SEMICOLON)
    {
        cout << "Error in USE statement (module " << tk_module_name << "): "
             << "Expected ';', got '" << tk_semicolon << "'"
             << endl;
        return -1;
    }

    if (strlen( tk_module_name.tokval() ) > 10)
    {
        cout << "Error in USE statement: Module names must be <= 10 characters" << endl
             << "Module specified was: '" << tk_module_name << "'" 
             << endl;
        return -1;
    }

    return useModule( tk_module_name.tokval() );
}

int Compiler::includeModule( const std::string& modulename )
{
//    cout << "includeModule(" << modulename << "). includes(" << included.size() << "):";
//    for( INCLUDES::const_iterator citr = included.begin(); citr != included.end(); ++citr )
//    {
//        cout << " " << (*citr);
//    }
//    cout << endl;

    string filename_part = modulename;
	filename_part += ".inc";

    string filename_full = current_file_path + filename_part;

    if (filename_part[0] == ':')
    {
        const Package* pkg = NULL;
        string path;
        if (pkgdef_split( filename_part, NULL, &pkg, &path ))
        {
            if (pkg != NULL)
            {
                filename_full = pkg->dir() + path;
                string try_filename_full = pkg->dir() + "include/" + path;

                if (verbosity_level_ >= 10)
                    cout << "Searching for " << filename_full << endl;

                if (!FileExists( filename_full.c_str() ))
                {
                    if (verbosity_level_ >= 10)
                        cout << "Searching for " << try_filename_full << endl;
                    if (FileExists( try_filename_full.c_str() ))
                    {
                        if (verbosity_level_ >= 10)
                            cout << "Found " << try_filename_full << endl;

                        filename_full = try_filename_full;
                    }
                }
                else
                {
                    if (verbosity_level_ >= 10)
                        cout << "Found " << filename_full << endl;

                    if (FileExists( try_filename_full.c_str() ))
                        cout << "Warning: Found '" << filename_full.c_str() << "' and '"
                             << try_filename_full.c_str() << "'! Will use first file!" << endl;
                }
            }
            else
            {
                filename_full = compilercfg.PolScriptRoot + path;

                if (verbosity_level_ >= 10)
                {
                    cout << "Searching for " << filename_full << endl;
                    if (FileExists( filename_full.c_str() ))
                        cout << "Found " << filename_full << endl;
                }
            }
        }
        else
        {
            cout << "Unable to read include file '" << modulename << "'" << endl;
            return false;
        }
    }
    else
    {
        if (verbosity_level_ >= 10)
            cout << "Searching for " << filename_full << endl;

        if (!FileExists( filename_full.c_str() ))
        {
            string try_filename_full = compilercfg.IncludeDirectory + filename_part;
            if (verbosity_level_ >= 10)
                cout << "Searching for " << try_filename_full << endl;
            if (FileExists( try_filename_full.c_str() ))
            {
                if (verbosity_level_ >= 10)
                    cout << "Found " << try_filename_full << endl;

                //cout << "Using " << try_filename << endl;
                filename_full = try_filename_full;
            }
        }
        else
        {
            if (verbosity_level_ >= 10)
                cout << "Found " << filename_full << endl;
        }
    }

	string filename_check = FullPath(filename_full.c_str());
    if (included.count(filename_check))
        return 0;
    included.insert(filename_check);

    referencedPathnames.push_back( filename_full );

    char *orig_mt;
    
    if (getFileContents( filename_full.c_str(), &orig_mt ))
    {
		cout << "Unable to find module " << modulename << endl;
		cout << "\t(Filename: " << filename_full << ")" << endl;
        return -1;
    }
    
    CompilerContext mod_ctx( filename_full, program->add_dbg_filename(filename_full), orig_mt );

    string save = current_file_path;
    current_file_path = getpathof( filename_full );

    int res = compileContext( mod_ctx );

    current_file_path = save;

    free(orig_mt);
    if (res < 0)
        return res;
    else
        return 0;
}
#endif








#if 1
int Compiler::handleInclude( CompilerContext& ctx )
{
    Token tk_module_name, tk_semicolon;

    if (getToken( ctx, tk_module_name ))
    {
        cout << "Error in INCLUDE statement: INCLUDE should be followed by a module name." << endl;
        return -1;
    }
	if (tk_module_name.id != TOK_IDENT && tk_module_name.id != TOK_STRING)
	{
        cout << "Error in INCLUDE statement: Expected identifier, got '"
             << tk_module_name << "'" << endl;
		return -1;
	}

    if (getToken( ctx, tk_semicolon ))
    {
        cout << "Error in INCLUDE statement (module " << tk_module_name << "): "
             << "Expected ';', got end-of-file or error"
             << endl;
        return -1;
    }
    if (tk_semicolon.id != TOK_SEMICOLON)
    {
        cout << "Error in INCLUDE statement (module " << tk_module_name << "): "
             << "Expected ';', got '" << tk_semicolon << "'"
             << endl;
        return -1;
    }

    return includeModule( tk_module_name.tokval() );

}
#endif


int Compiler::insertBreak( const string& label )
{
    // Now, we've eaten the break; or break label; and 'label' contains the label, if any.
    // first, we must find the block level this refers to.
    unsigned numVarsToKill = 0;
    for( int i = localscope.blockdescs_.size()-1; i >= 0; --i )
    {
        BlockDesc& bd = localscope.blockdescs_[i];
        
        numVarsToKill += bd.varcount;

        if (bd.break_ok &&
            (label == "" ||             // we didn't pick, and this is closest
            label == bd.label))       // this is the one we picked
        {
            if (numVarsToKill)
            {       // local variables were declared in this scope.  We need to kill 'em.

                program->append( StoredToken(Mod_Basic, 
                                            CTRL_LEAVE_BLOCK, 
                                            TYP_CONTROL, 
                                            numVarsToKill), 
                                0);
            }

            unsigned goto_posn;
            program->append(
                StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, 0), &goto_posn);
            bd.break_tokens.push_back( goto_posn );
            return 0;
                            
        }
    }
    cout << "Couldn't find an appropriate break point";
    if (label != "")
        cout << " for label " << label;
    cout << "." << endl;

    return -1;
}


#if 1

// break statements come in the following forms:
//      break;
//      break label;
// we'll emit a LEAVE_BLOCK token if necessary, and
// a GOTO which will be patched in when the block
// is completed.
int Compiler::handleBreak( CompilerContext& ctx )
{
    Token tk;
    string label;
    
    if (getToken( ctx, tk ) ||
        ((tk.id != TOK_IDENT) && (tk.id != TOK_SEMICOLON)))
    {
        cout << "break statement: expected 'break;' or 'break label;'" << endl;
        return -1;
    }
    
    if (tk.id == TOK_IDENT)
    {
        label = tk.tokval();
        if (getToken( ctx, tk ) || tk.id != TOK_SEMICOLON)
        {
            cout << "break statement: expected 'break;' or 'break label;'" << endl;
            return -1;
        }
    }
    else
    {
        label = "";
    }

    return insertBreak( label );
}

// continue statements come in the following forms:
//      continue;
//      continue label;
// we'll emit a LEAVE_BLOCK token if necessary, and
// a GOTO which will point at the continuePC of the block.
int Compiler::handleContinue( CompilerContext& ctx )
{
    Token tk;
    string label;
    
    if (getToken( ctx, tk ) ||
         ((tk.id != TOK_IDENT) && (tk.id != TOK_SEMICOLON)))
    {
        cout << "continue statement: expected 'continue;' or 'continue label;'" << endl;
        return -1;
    }
    
    if (tk.id == TOK_IDENT)
    {
        label = tk.tokval();
        if (getToken( ctx, tk ) || tk.id != TOK_SEMICOLON)
        {
            cout << "continue statement: expected 'continue;' or 'continue label;'" << endl;
            return -1;
        }
    }
    else
    {
        label = "";
    }

    // Now, we've eaten the continue; or continue label; and 'label' contains the label, if any.
    // first, we must find the block level this refers to.
    unsigned numVarsToKill = 0;
    for( int i = localscope.blockdescs_.size()-1; i >= 0; --i )
    {
        BlockDesc& bd = localscope.blockdescs_[i];
        numVarsToKill += bd.varcount;

        if (bd.continue_ok &&
            (label == "" ||             // we didn't pick, and this is closest
            label == bd.label))       // this is the one we picked
        {
            if (numVarsToKill)
            {       // local variables were declared in this scope.  We need to kill 'em.

                program->append( StoredToken(Mod_Basic, 
                                            CTRL_LEAVE_BLOCK, 
                                            TYP_CONTROL, 
                                            numVarsToKill), 
                                0);
            }

            unsigned goto_posn;
            program->append(
                StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, 0), &goto_posn);
            bd.continue_tokens.push_back( goto_posn );
            return 0;
                            
        }
    }
    cout << "Couldn't find an appropriate continue point";
    if (label != "")
        cout << " for label " << label;
    cout << "." << endl;

    return -1;
}


// BASIC-style FOR loop:
// FOR I := 1 to 5 DO
// FOR I 
//
// Emitted Code:
//   (starting expr value)
//   (ending expr value)
//   INITFOR
//  statement_part: 
//   (code block)
//   NEXTFOR(statement_part)
//   
//
int Compiler::handleBracketedFor_basic( CompilerContext& ctx )
{
    CompilerContext for_ctx( ctx );
    int res;
    
    Token itrvar, tmptoken;

    res = getToken( ctx, itrvar );
    if (res < 0)
        return res;
    if (itrvar.id != TOK_IDENT)
    {
        cout << "FOR iterator must be an identifier, got " << itrvar << endl;
        return res;
    }

    if (localscope.varexists( itrvar.tokval() ))
    {
        cout << "FOR iterator '" << itrvar << "' hides a local variable." << endl;
        return -1;
    }

    res = eatToken( ctx, TOK_ASSIGN );
    if (res < 0)
        return res;

    
    res = getExpr( ctx, EXPR_FLAG_TO_TERM_ALLOWED );
    if (res < 0)
        return res;

    res = eatToken( ctx, RSV_TO );
    if (res < 0)
        return res;

    res = getExpr2( ctx, EXPR_FLAG_AUTO_TERM_ALLOWED  );
    if (res < 0)
        return res;

    enterblock( CanNotBeLabelled );
    unsigned initfor_posn;
    program->append( StoredToken( Mod_Basic, INS_INITFOR, TYP_RESERVED, 0 ),
                             &initfor_posn );

			/*
				INITFOR creates two local variables, placeholders for the iterator
				and end value.  Only the iterator can be accessed, for now.
			*/
    program->addlocalvar( itrvar.tokval() );
    if (verbosity_level_ >= 5)
		localscope.addvar( itrvar.tokval(), for_ctx);
	else
		localscope.addvar( itrvar.tokval(), for_ctx, false);
    program->addlocalvar( "_" + string(itrvar.tokval()) + "_end" );
    localscope.addvar( "_" + string(itrvar.tokval()) + "_end", for_ctx, false );

    unsigned again_posn = program->tokens.next();
    enterblock( CanBeLabelled );
    Token endblock_tkn;
    res = readblock( ctx, 1, RSV_ENDFOR, NULL, &endblock_tkn );
    if (res < 0)
        return res;

    emit_leaveblock();

    program->update_dbg_pos( endblock_tkn );
    unsigned nextfor_posn;
    program->append( StoredToken( Mod_Basic, INS_NEXTFOR, TYP_RESERVED, again_posn ),
                             &nextfor_posn );

    patchblock_continues( nextfor_posn );
    patchblock_breaks( program->tokens.next() );
    localscope.popblock();
    program->leaveblock();

    leaveblock(0,0);

    patchoffset( initfor_posn, program->tokens.next() );

    return 0;
}

int Compiler::handleFor_c( CompilerContext& ctx )
{
    do {
        Token tkn;
        getToken( ctx, tkn );
        if (tkn.id != TOK_LPAREN)
        {
            cout << "FOR: expected '('" << endl;
            return -1;
        }
    } while (0);
    
    enterblock( CanBeLabelled );

    Expression initial_expr;
    Expression predicate_expr;
    Expression iterate_expr;
    if (readexpr( initial_expr, ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED ) != 1)
        return -1;
    if (readexpr( predicate_expr, ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED ) != 1)
        return -1;
    if (readexpr( iterate_expr, ctx, EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED ) != 1)
        return -1;

    do {
        Token tkn;
        if (getToken( ctx, tkn ) || tkn.id != TOK_RPAREN)
        {
            cout << "FOR: expected '('" << endl;
            return -1;
        }
    } while (0);

	/*
		The c-style 'for' statement gets generated as follows:
			given:	
					for( initial; predicate; iterate ) statement;
					initial;
			again:
					predicate;
					if true goto statement_part;
						break;
			statement_part:
					statement;
			iterate_part:
					iterate;
					goto again;
	*/


    inject( initial_expr );
    program->append( StoredToken(Mod_Basic, TOK_CONSUMER, TYP_UNARY_OPERATOR, 0) );

    unsigned againPC = program->tokens.next();
    inject( predicate_expr );

    unsigned if_posn;
    program->append( StoredToken( Mod_Basic, RSV_JMPIFTRUE, TYP_RESERVED, 0 ), &if_posn );

    insertBreak( "" );
    patchoffset( if_posn, program->tokens.next() );

    int res = getStatement( ctx, 1 );
    if (res < 0)
        return -1;
    unsigned nextPC = program->tokens.next();
    
    inject( iterate_expr );
    program->append( StoredToken(Mod_Basic, TOK_CONSUMER, TYP_UNARY_OPERATOR, 0) );
    
    program->append( StoredToken( Mod_Basic, RSV_GOTO, TYP_RESERVED, againPC ), 0 );

    leaveblock( program->tokens.next(), nextPC );

    return 0;
}
int Compiler::handleBracketedFor_c( CompilerContext& ctx )
{
    do {
        Token tkn;
        getToken( ctx, tkn );
        if (tkn.id != TOK_LPAREN)
        {
            cout << "FOR: expected '('" << endl;
            return -1;
        }
    } while (0);
    
    enterblock( CanBeLabelled );

    Expression initial_expr;
    Expression predicate_expr;
    Expression iterate_expr;
    if (readexpr( initial_expr, ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED ) != 1)
        return -1;
    if (readexpr( predicate_expr, ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED ) != 1)
        return -1;
    if (readexpr( iterate_expr, ctx, EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED ) != 1)
        return -1;

    do {
        Token tkn;
        if (getToken( ctx, tkn ) || tkn.id != TOK_RPAREN)
        {
            cout << "FOR: expected '('" << endl;
            return -1;
        }
    } while (0);

	/*
		The c-style 'for' statement gets generated as follows:
			given:
					for( initial; predicate; iterate ) statement;
					initial;
			again:
					predicate;
					if true goto statement_part;
						break;
			statement_part:
					statement;
			iterate_part:
					iterate;
					goto again;
	*/


    inject( initial_expr );
    program->append( StoredToken(Mod_Basic, TOK_CONSUMER, TYP_UNARY_OPERATOR, 0) );

    unsigned againPC = program->tokens.next();
    inject( predicate_expr );

    unsigned if_posn;
    program->append( StoredToken( Mod_Basic, RSV_JMPIFFALSE, TYP_RESERVED, 0 ), &if_posn );

    enterblock( CanNotBeLabelled );
    Token endblock_tkn;
    int res = readblock( ctx, 1, RSV_ENDFOR, NULL, &endblock_tkn );
    if (res < 0)
        return res;
    leaveblock(0,0);
    
    unsigned continuePC = program->tokens.next();
    
    program->update_dbg_pos( endblock_tkn );
    inject( iterate_expr );
    program->append( StoredToken(Mod_Basic, TOK_CONSUMER, TYP_UNARY_OPERATOR, 0) );
    
    program->append( StoredToken( Mod_Basic, RSV_GOTO, TYP_RESERVED, againPC ), 0 );
    patchoffset( if_posn, program->tokens.next() );

    leaveblock( program->tokens.next(), continuePC );

    return 0;
}

int Compiler::handleFor( CompilerContext& ctx )
{
    Token tkn;
    int res;
    res = peekToken( ctx, tkn );
    if (res)
    {
        cout << "Error in FOR statement";
        return -1;
    }
    
    if (tkn.id == TOK_LPAREN)
        return handleBracketedFor_c( ctx );
    else
        return handleBracketedFor_basic( ctx );
}
void Compiler::emitFileLine( CompilerContext& ctx )
{
    int cnt = program->tokens.count();
    program->fileline.resize( cnt + 1 );
    program->fileline[ cnt ] = ctx.filename + ", Line " + decint( static_cast<unsigned long>(ctx.line) );
}
void Compiler::emitFileLineIfFileChanged( CompilerContext& ctx )
{
    string last;
    if (program->fileline.size())
        last = program->fileline.back();

    if (program->fileline.size() == program->tokens.count()+1 ||
        last.empty() || 
        last.substr( 0, ctx.filename.size() ) != ctx.filename)
    {
        emitFileLine( ctx );
    }
}
void Compiler::savesourceline()
{
        int cnt = program->tokens.count();
        program->sourcelines.resize( cnt + 1 );
        program->sourcelines[ cnt ] = curLine;
}
int Compiler::_getStatement(CompilerContext& ctx, int level)
{
    int res;
    unsigned last_position = 0;

    readCurLine( ctx );

    if (include_debug)
    {
        program->symbols.append(curLine, last_position);

        DebugToken DT;
        DT.sourceFile = curSourceFile;
        DT.offset = ctx.s - ctx.s_begin;
        DT.strOffset = last_position;

        program->symbols.append(&DT, sizeof DT, last_position);
    }
    StoredToken stok(Mod_Basic, CTRL_STATEMENTBEGIN, TYP_CONTROL, last_position);


    // reinit();

    Token token;

    if (peekToken(ctx, token)==1) 
	{
        getToken(ctx, token); // trailing whitespace can hurt.
        return 1; // all done!
	}

    if (token.id != RSV_FUNCTION && token.id != RSV_PROGRAM)
    {
        savesourceline();
        emitFileLineIfFileChanged(ctx);
    }

    if (include_debug)
        program->append(stok, &last_position);
    else
        last_position = program->tokens.count();

	if (token.deprecated)
	{
		cout << "Warning: Found deprecated token "
				<< "'" << token.tokval() << "'"
				<< " on line " << ctx.line
				<< " of " << ctx.filename << endl;
		// warning only; doesn't bail out.
	}

    if (token.type == TYP_RESERVED) 
	{
        getToken(ctx, token);
        switch(token.id) {
            case RSV_OPTION_BRACKETED: 
                cout << "_OptionBracketed is obsolete." << endl; 
                // bracketed_if_ = true; 
                return 0;
            case RSV_DECLARE:       return handleDeclare(ctx);
            
            case RSV_EXPORTED:
            case RSV_FUNCTION:      
                return handleBracketedFunction2(ctx, level, token.type);
            
			case RSV_USE_MODULE:    return handleUse( ctx );
			case RSV_INCLUDE_FILE:  return handleInclude( ctx );

			case RSV_PROGRAM:       return handleProgram( ctx, level+1 );
            case RSV_RETURN:        return handleReturn(ctx);
            case RSV_EXIT:          return handleExit(ctx);

			case RSV_ST_IF:         
                return handleBracketedIf(ctx, level+1);

			case RSV_FOR:           return handleFor( ctx );
			case RSV_FOREACH:       return handleForEach(ctx,level+1);
			case RSV_SWITCH:        return handleSwitch(ctx, level+1);
			case RSV_REPEAT:        return handleRepeatUntil(ctx,level+1);
			case RSV_DO:            return handleDoClause(ctx,level+1);
			case RSV_WHILE:         return handleBracketedWhile(ctx,level+1);

			case RSV_BREAK:         return handleBreak( ctx );
			case RSV_CONTINUE:      return handleContinue( ctx );

			case RSV_VAR:           return handleVarDeclare( ctx, inGlobalScope() ? RSV_GLOBAL : RSV_LOCAL );
            case RSV_CONST:         return handleConstDeclare( ctx );
            case RSV_ENUM:          return handleEnumDeclare( ctx );

	//DEPRECATED:
		//	case RSV_BEGIN:         return handleBlock(ctx, level+1);

			default:
                cout << "Unhandled reserved word: " << token << endl;
                return -1;
//               assert(0);
                break;
        } 
    } 
    else if (token.type == TYP_LABEL) 
    {
        if (!quiet) cout << "Label found! " << token << endl;
        getToken(ctx, token);

        Token precedes;
        res = peekToken( ctx, precedes );
        if (res != 0 || 
              (precedes.id != RSV_SWITCH &&
               precedes.id != RSV_FOREACH &&
               precedes.id != RSV_REPEAT &&
               precedes.id != RSV_WHILE &&
               precedes.id != RSV_DO &&
               precedes.id != RSV_FOR ))
        {
            cout << "Illegal location for label: " << token.tokval() << endl;
            cout << "Labels can only come before DO, WHILE, FOR, FOREACH, REPEAT, and CASE statements." << endl;
            return -1;
        }
        latest_label = token.tokval();
        token.lval = last_position;
        return 0;
    }

    unsigned exprlen;
    res = getExpr(ctx, EXPR_FLAG_SEMICOLON_TERM_ALLOWED|EXPR_FLAG_CONSUME_RESULT, &exprlen);

    if (res < 0)
        return res;
    if (exprlen != 0)
    {
	    StoredToken tmptoken;
	    program->tokens.atGet1( program->tokens.count()-1, tmptoken );
        if (tmptoken.id == TOK_CONSUMER)
        {
    	    program->tokens.atGet1( program->tokens.count()-2, tmptoken );
        }

        if (tmptoken.id == TOK_ASSIGN ||
			tmptoken.id == TOK_PLUSEQUAL ||
			tmptoken.id == TOK_MINUSEQUAL ||
			tmptoken.id == TOK_TIMESEQUAL ||
			tmptoken.id == TOK_DIVIDEEQUAL || 
			tmptoken.id == TOK_MODULUSEQUAL || 
            tmptoken.id == INS_SUBSCRIPT_ASSIGN ||
            tmptoken.id == INS_SUBSCRIPT_ASSIGN_CONSUME ||
            tmptoken.id == INS_MULTISUBSCRIPT_ASSIGN ||
            tmptoken.id == INS_ASSIGN_CONSUME ||
            tmptoken.id == TOK_ADDMEMBER ||
            tmptoken.id == TOK_DELMEMBER ||
            tmptoken.id == TOK_FUNC ||
            tmptoken.id == INS_CALL_METHOD ||
            tmptoken.id == TOK_USERFUNC ||
            tmptoken.id == CTRL_JSR_USERFUNC ||
            tmptoken.id == INS_ASSIGN_LOCALVAR ||
            tmptoken.id == INS_ASSIGN_GLOBALVAR ||
            tmptoken.id == INS_SET_MEMBER ||
            tmptoken.id == INS_SET_MEMBER_CONSUME ||
            tmptoken.id == INS_SET_MEMBER_ID ||
            tmptoken.id == INS_SET_MEMBER_ID_CONSUME ||
            tmptoken.id == INS_CALL_METHOD_ID 
            )
        {
            // ok! These operators actually accomplish something.
        }
        else if (tmptoken.id == TOK_EQUAL1)
        {
            cout << "Warning: Equals test result ignored.  Did you mean := for assign?" << endl;
            cout << "near: " << curLine << endl;
            cout << ctx;
        }
        else
        {
            // warn code has no effect/value lost
            cout << "Warning: Result of operation may have no effect." << endl;
            cout << "Token ID: " << tmptoken.id << endl;
            cout << "near: " << curLine << endl;
            cout << ctx;
        }
    //    cout << "Statement: " << Parser.CA << endl;
    }
    return res;
}

int Compiler::getStatement(CompilerContext& ctx, int level)
{
    ctx.skipws();
    ctx.skipcomments();

    CompilerContext savectx(ctx);
    program->setcontext(savectx);
    program->setstatementbegin();
    int res = 0;
    try
    {
        res = _getStatement( ctx, level );
        if (res < 0)
        {
            cout << "Error compiling statement at ";
            savectx.printOnShort( cout );
        }
    }
    catch( std::exception& ex )
    {
        cout << "Error compiling statement at ";
        savectx.printOnShort( cout );
        cout << ex.what() << endl;
        res = -1;
    }
    return res;

}

int Compiler::handleFunction( CompilerContext& ctx )
{
	int res;
	Token token;
    Token funcName;

	if (inFunction)
	{
		cout << "Can't declare a function inside another function." << endl;
		cout << "(attempt to declare " << funcName << ")" << endl;
		return -1;
	}
	inFunction = 1;

    UserFunction userfunc;

    if (readFunctionDeclaration( ctx, userfunc ))
    {
        return -1;
    }
    userFunctions[ userfunc.name ] = userfunc;
/*
	should be begin, then statements while peektoken != end, then eat end.

	getToken(s, token);
	if (token.type != TYP_DELIMITER || token.id != DELIM_SEMICOLON) {
		err = PERR_MISSINGDELIM;
		return -1;
	}
*/
	// woo-hoo! recursive calls should work.
    // cout << "func decl: " << curLine << endl;
    unsigned posn = 0;

    unsigned skip_goto_posn;
	
	if (include_debug)
    {
		/*
			A bit of explanation: 
			We want to end up with this:
				GOTO (skip)
				STATEMENT BEGIN (function declaration) <-- user func address
				User Function (Foo)
			But right now we have:
				STATEMENT_BEGIN (line info)
					and adding the goto and function would end up adding:
				GOTO (skip)
				(User Function (Foo) will soon go here) <-- user func address
			So, we append a copy of the statement_begin, then put a goto
				in its original position.
		*/

		// STATEMENT
	    StoredToken tmptoken;
	    program->tokens.atGet1( program->tokens.count()-1, tmptoken );
	    program->append( tmptoken, &userfunc.position ); // STATEMENT, STATEMENT

        // skip_goto_posn: a goto is inserted, so prog ctrl will skip over this function
        skip_goto_posn = program->tokens.count()-2;
	    program->tokens.atPut1( StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, 0), skip_goto_posn );
													    /* GOTO, STATEMENT */

        program->symbols.append(userfunc.name.c_str(), posn);
	    program->append( StoredToken(Mod_Basic, TOK_USERFUNC, TYP_USERFUNC, posn ) );
    }
    else
    {
        program->append( StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, 0 ), &skip_goto_posn );
        userfunc.position = posn = program->tokens.count();
    }

    res = getToken(ctx,token);
    if (res) return res;
    if (token.id != RSV_BEGIN) 
    {
        cout << "Error reading function definition for " << userfunc.name << "()" << endl;
        cout << "Expected BEGIN .. END block, got token: '" << token << "'" << endl;
             
        //cout << "Functions must contain a BEGIN .. END block." << endl;
        return -1;
    }

    program->enterfunction();
    enterblock( CanNotBeLabelled );

	for( int i = userfunc.parameters.size()-1; i >= 0 ; --i )
	{
		program->symbols.append( userfunc.parameters[i].name.c_str(), posn );
        program->append( StoredToken(Mod_Basic, 
                                 userfunc.parameters[i].pass_by_reference?INS_POP_PARAM_BYREF:INS_POP_PARAM, 
                                 TYP_OPERATOR, posn), 0 );
        program->addlocalvar( userfunc.parameters[i].name );

        localscope.addvar( userfunc.parameters[i].name, ctx );
	}

	res = handleBlock( ctx, 1 /* level */ );
	if (res < 0) return res;

	StoredToken tmp;
	program->tokens.atGet1( program->tokens.count()-1, tmp );
	if (tmp.id != RSV_RETURN)
	{
		program->symbols.append("", posn);
		program->append( StoredToken(Mod_Basic, TOK_STRING, TYP_OPERAND, posn) );
		program->append( StoredToken(Mod_Basic, RSV_RETURN, TYP_RESERVED, 0) );
	}

    leaveblock( 0,0 );
    program->leavefunction();

	/* now, the skip goto must be patched up with the correct PC address*/
	patchoffset( skip_goto_posn, program->tokens.next() );


    inFunction = 0;
    return 0;
}
#endif









#if 1
int Compiler::handleBracketedFunction( CompilerContext& ctx )
{
    CompilerContext save_ctx( ctx );
	int res;
	Token token;

	if (inFunction)
	{
		cout << "Can't declare a function inside another function." << endl;
		return -1;
	}
	inFunction = 1;

    UserFunction userfunc;

    if (readFunctionDeclaration( ctx, userfunc ))
    {
        return -1;
    }
    userFunctions[ userfunc.name ] = userfunc;
/*
	should be begin, then statements while peektoken != end, then eat end.

    getToken(s, token);
    if (token.type != TYP_DELIMITER || token.id != DELIM_SEMICOLON) {
        err = PERR_MISSINGDELIM;
        return -1;
    }
*/
	/* woo-hoo! recursive calls should work. */
    //cout << "func decl: " << curLine << endl;
    unsigned posn = 0;

    unsigned skip_goto_posn;
	
	if (include_debug)
    {
		/*
			A bit of explanation: 
			We want to end up with this:
				GOTO (skip)
				STATEMENT BEGIN (function declaration) <-- user func address
				User Function (Foo)
			But right now we have:
				STATEMENT_BEGIN (line info)
					and adding the goto and function would end up adding:
				GOTO (skip)
				(User Function (Foo) will soon go here) <-- user func address
			So, we append a copy of the statement_begin, then put a goto
			in its original position.
		*/
													     /* STATEMENT */
	    StoredToken tmptoken;
	    program->tokens.atGet1( program->tokens.count()-1, tmptoken );
	    program->append( tmptoken, &userfunc.position ); /* STATEMENT, STATEMENT */

        /* skip_goto_posn: a goto is inserted, so prog ctrl will skip over this function */
        skip_goto_posn = program->tokens.count()-2;
	    program->tokens.atPut1( StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, 0), skip_goto_posn );
													    /* GOTO, STATEMENT */

        program->symbols.append(userfunc.name.c_str(), posn);
	    program->append( StoredToken(Mod_Basic, TOK_USERFUNC, TYP_USERFUNC, posn ) );
    }
    else
    {
        program->append( StoredToken(Mod_Basic, RSV_GOTO, TYP_RESERVED, 0 ), &skip_goto_posn );
        userfunc.position = posn = program->tokens.count();
    }

    program->enterfunction();
    enterblock( CanNotBeLabelled );

	for( int i = userfunc.parameters.size()-1; i >= 0 ; --i )
	{
		program->symbols.append( userfunc.parameters[i].name.c_str(), posn );
		program->append( StoredToken(Mod_Basic, 
            userfunc.parameters[i].pass_by_reference?INS_POP_PARAM_BYREF:INS_POP_PARAM, 
            TYP_OPERATOR, posn), 0 );

        program->addlocalvar( userfunc.parameters[i].name );
        localscope.addvar( userfunc.parameters[i].name, ctx );
	}
    
    Token endblock_tkn;
	res = readblock( ctx, 1, RSV_ENDFUNCTION, NULL, &endblock_tkn );
	if (res < 0) 
    {
        cout << "Error occurred reading function body for '" << userfunc.name << "'" << endl
             << "Function location: " << save_ctx;
        cout << "Error location: " << endl;
        return res;
    }

    program->update_dbg_pos( endblock_tkn );
	StoredToken tmp;
	program->tokens.atGet1( program->tokens.count()-1, tmp );
	if (tmp.id != RSV_RETURN)
	{
		program->symbols.append("", posn);
		program->append( StoredToken(Mod_Basic, TOK_STRING, TYP_OPERAND, posn) );
		program->append( StoredToken(Mod_Basic, RSV_RETURN, TYP_RESERVED, 0) );
	}

    leaveblock( 0,0 );
    program->leavefunction();

	/* now, the skip goto must be patched up with the correct PC address*/
	patchoffset( skip_goto_posn, program->tokens.next() );


    inFunction = 0;
    return 0;
}

// pass 2 function: just skip past, to the ENDFUNCTION.
int Compiler::handleBracketedFunction2( CompilerContext& ctx, int level, int tokentype )
{
    CompilerContext save_ctx( ctx );
	int res = -1;
	Token tk_funcname;

    if (tokentype == RSV_EXPORTED)
    {
        Token tk_function;
        res = getToken( ctx, tk_function );
        if (res) return res;
        if (tk_function.id != RSV_FUNCTION)
        {
            cout << "Expected 'function' after 'exported'." << endl;
            return -1;
        }
    }

	if (inFunction)
	{
		cout << "Can't declare a function inside another function." << endl;
		return -1;
	}
    getToken( ctx, tk_funcname );
    while (ctx.s[0])
    {
        Token token;
        res = getToken( ctx, token );
        if (res < 0)
            break;
        if (token.id == RSV_ENDFUNCTION)
        {
            // we already grabbed the function body.
            return 0;
        }
    }
    if (!ctx.s[0])
    {
        cout << "End-of-File detected, expected 'ENDFUNCTION'" << endl;
        return -1;
    }

    if (res < 0) 
    {
        cout << "Error occurred reading function body for '" << tk_funcname.tokval() << "'" << endl
             << "Function location: " << save_ctx;
        cout << "Error location: " << endl;
        return res;
    }

    return res;
}

int Compiler::handleProgram( CompilerContext& ctx, int level )
{
    Token tk_progname;
    int res;

    if (haveProgram)
    {
        cout << "'program' function has already been defined." << endl;
        return -1;
    }
    haveProgram = true;
    program->program_decl = curLine;
    program_ctx = ctx;
    const char* program_body_start = ctx.s;
    while (ctx.s[0])
    {
        Token token;
        res = getToken( ctx, token );
        if (res < 0)
            return res;

        if (token.id == RSV_ENDPROGRAM)
        {
            const char* program_body_end = ctx.s;
            int len = program_body_end - program_body_start + 1;
            program_source = new char[ len ];
            delete_these_arrays.push_back( program_source );
            memcpy( program_source, program_body_start, len-1 );
            program_source[len-1] = '\0';
            program_ctx.s = program_ctx.s_begin = program_source;
            return 0;
        }

    }
    cout << "End of file detected, expected 'endprogram'" << endl;
    return -1;
}

int Compiler::handleProgram2( CompilerContext& ctx, int level )
{
    Token tk_progname;
    int res;

    emitFileLine( ctx );
    program->program_PC = program->tokens.count();

    res = getToken( ctx, tk_progname );
    if (res < 0) return res;
    if (tk_progname.id != TOK_IDENT)
    {
        cout << "Error: expected identified after 'program', got '" << tk_progname << "'" << endl;
        return -1;
    }

    program->enterfunction();
    enterblock( CanNotBeLabelled );

    res = eatToken( ctx, TOK_LPAREN );
    if (res < 0) return res;
    for (;;)
    {
        Token token;
        res = getToken( ctx, token );
        if (res < 0) return res;
        if (res > 0)
        {
            cout << "End-of-file reached reading program argument list" << endl;
            return -1;
        }
        if (token.id == TOK_RPAREN)
        {
            break;
        }
        else if (token.id == TOK_IDENT)
        {
            unsigned varpos;
            if (localscope.varexists( token.tokval(), varpos ))
            {
                cout << "Program argument '" << token << "' multiply defined." << endl;
                return -1;
            }
            unsigned posn;
            program->symbols.append( token.tokval(), posn );
            program->append( StoredToken(Mod_Basic, INS_GET_ARG, TYP_OPERATOR, posn), 0 );
            program->addlocalvar( token.tokval() );
            localscope.addvar( token.tokval(), ctx );

            res = peekToken( ctx, token );
            if (res < 0) return res;
            if (token.id == TOK_COMMA)
                getToken( ctx, token );
            ++nProgramArgs;
        }
        else
        {
            cout << "Expected arguments or right-paren in program arglist, got '" << token << "'" << endl;
            return -1;
        }
    }
    program->haveProgram = true;
    program->expectedArgs = nProgramArgs;
    Token endblock_tkn;
    res = readblock( ctx, level, RSV_ENDPROGRAM, NULL, &endblock_tkn );
    if (res < 0) return res;

    program->update_dbg_pos( endblock_tkn );
    leaveblock( 0, 0 );
    program->leavefunction();

    return 0;
}

int Compiler::handleBracketedFunction3( UserFunction& userfunc, CompilerContext& ctx )
{
    unsigned first_PC = program->tokens.count();

    emitFileLine( ctx );
    program->function_decls.resize( program->tokens.count() + 1 );
    program->function_decls[ program->tokens.count() ] = userfunc.declaration;
    CompilerContext save_ctx( ctx );
	int res;
	Token token;

    inFunction = 1;
/*
	should be begin, then statements while peektoken != end, then eat end.

    getToken(s, token);
    if (token.type != TYP_DELIMITER || token.id != DELIM_SEMICOLON) {
        err = PERR_MISSINGDELIM;
        return -1;
    }
*/
	/* woo-hoo! recursive calls should work. */
    //cout << "func decl: " << curLine << endl;
    unsigned posn = 0;

    if (userfunc.exported)
    {
        EPExportedFunction ef;
        ef.name = userfunc.name;
        ef.nargs = userfunc.parameters.size();
        ef.PC = program->tokens.count();
        program->exported_functions.push_back( ef );

        // insert the stub:
        program->append( StoredToken(Mod_Basic,CTRL_MAKELOCAL,TYP_CONTROL,0), ctx );
        program->append( StoredToken(Mod_Basic,CTRL_JSR_USERFUNC, TYP_CONTROL, ef.PC+3), ctx );
        program->append( StoredToken(Mod_Basic,CTRL_PROGEND, TYP_CONTROL, 0), ctx );
    }
    
    userfunc.position = posn = program->tokens.count();


    program->enterfunction();
    enterblock( CanNotBeLabelled );

	for( int i = userfunc.parameters.size()-1; i >= 0 ; --i )
	{
		program->symbols.append( userfunc.parameters[i].name.c_str(), posn );
		program->append( StoredToken(Mod_Basic, 
                                     userfunc.parameters[i].pass_by_reference?INS_POP_PARAM_BYREF:INS_POP_PARAM, 
                                     TYP_OPERATOR, 
                                     posn), 
                         save_ctx,
                         0 );
        program->addlocalvar( userfunc.parameters[i].name );
        localscope.addvar( userfunc.parameters[i].name, ctx );
	}

    BTokenId last_statement_id;
    Token endblock_tkn;
	res = readblock( ctx, 1, RSV_ENDFUNCTION, &last_statement_id, &endblock_tkn );
	if (res < 0) 
    {
        cout << "Error in function '" << userfunc.name << "', " << ctx << endl;
        return res;
    }
    program->update_dbg_pos( endblock_tkn );

	StoredToken tmp;
	program->tokens.atGet1( program->tokens.count()-1, tmp );
	
	/* 
		This used to check to see if the last instruction was a RETURN; 
		however, if that's in an ELSE block, that return won't get executed.
		This means if the function really does end in a return, this will
		generate extra code.  The optimizer will have to take care of this,
		if it can.
		Also note, the "leaveblock" at the end will also generate an instruction
		that will never get executed (NEVER!)
	*/
    if (last_statement_id != RSV_RETURN)
    {
        program->symbols.append(long(0), posn);
	    program->append( StoredToken(Mod_Basic, TOK_LONG, TYP_OPERAND, posn) );
	    program->append( StoredToken(Mod_Basic, RSV_RETURN, TYP_RESERVED, 0) );
    }
	
    // was leaveblock(0,0)
    localscope.popblock();
    program->leaveblock();
    program->leavefunction();

    unsigned last_PC = program->tokens.count()-1;
    program->addfunction( userfunc.name, first_PC, last_PC );

    inFunction = 0;
    return 0;
}

int Compiler::forward_read_function( CompilerContext& ctx )
{
    CompilerContext save_ctx( ctx );
	int res;
	Token token;

    UserFunction userfunc;
    userfunc.declaration = curLine;

    res = getToken( ctx, token );
    if (res) return res;
    if (token.id == RSV_EXPORTED)
    {
        userfunc.exported = true;
        res = getToken( ctx, token );
        if (res) return res;
        if (token.id != RSV_FUNCTION)
        {
            cout << "Expected 'function' after 'exported'" << endl;
            return -1;
        }
    }

    if (readFunctionDeclaration( ctx, userfunc ))
    {
        cout << save_ctx;
        return -1;
    }
    userfunc.ctx = ctx;

    const char *function_body_start = ctx.s;
    while (ctx.s[0])
    {
        Token token;
        res = getToken( ctx, token );
        if (res < 0)
            break;
        if (token.id == RSV_ENDFUNCTION)
        {
            const char* function_body_end = ctx.s;
            int len = function_body_end - function_body_start + 1;
            userfunc.function_body = new char[ len ];
            delete_these_arrays.push_back( userfunc.function_body );
            memcpy( userfunc.function_body, function_body_start, len-1 );
            userfunc.function_body[ len-1 ] = '\0';
            userfunc.ctx.s = userfunc.ctx.s_begin = userfunc.function_body;
            userFunctions[ userfunc.name ] = userfunc;
            res = 0;
            return 0;
        }
    }
    if (!ctx.s[0])
    {
        cout << "End-of-File detected, expected 'ENDFUNCTION'" << endl;
        cout << save_ctx;
        return -1;
    }

    if (res < 0) 
    {
        cout << "Error occurred reading function body for '" << userfunc.name << "'" << endl
             << "Function location: " << save_ctx;
        cout << "Error location: " << endl;
        return res;
    }
    return res;
}


void Compiler::patchoffset( unsigned instruc, unsigned newoffset )
{
    StoredToken tkn;
    
    program->tokens.atGet1( instruc, tkn);
    tkn.offset = static_cast<unsigned short>(newoffset);
    program->tokens.atPut1(tkn, instruc);
}


int Compiler::compileContext(CompilerContext& ctx)
{
    int res = 0;
    //scopes.push( LocalScope() );
    // currentscope = &scopes.top();
    // currentscope = &scope_;


    try 
    {
        while ((res >= 0) && ctx.s[0]) 
	    {
            res = getStatement(ctx, 0);
        }
    }
    catch( std::exception& )
    {
        cout << "Exception detected during compilation." << endl;
        cout << ctx;
        throw;
    }
    
    // currentscope = NULL;
    //scopes.pop();
    //assert( scopes.empty() );

    if (res == -1) 
	{
        if (err || ext_err[0])
        {  
            cout << "Parse Error: " << ParseErrorStr[err];
            if (ext_err[0]) cout << " " << ext_err;
            cout << endl;
            err = PERR_NONE;
            ext_err[0] = '\0';
        }
        else
        {
            cout << "Compilation Error:" << endl;
        }
        if (curLine[0])
        {
            cout << "Near: " << curLine << endl;
        }
        cout << ctx;
		return -1;
    }
    return 0;
}

int Compiler::compile(CompilerContext& ctx)
{
    unsigned dummy = 0;
    curSourceFile = 0;
    // startPos = ctx.s;
    program->symbols.append("", dummy);
    
    // see comment by handleUse: may want to only allow use statements at beginning of program->
    // useModule( "implicit" );
    useModule( "basic" );
    useModule( "basicio" );

    int res = 0;
    res = compileContext( ctx );

    if (res < 0) return res;
    // reinit();

    if (haveProgram)
    {
        // the local frame should be empty, so we can use it.
         //program->append( StoredToken( Mod_Basic, CTRL_MAKELOCAL, TYP_CONTROL, 0 ) );
        try {
            res = handleProgram2( program_ctx, 1 );
        }
        catch(...)
        {
            res = -1;
        }
        if (res < 0)
        {
            cout << "Error detected in program body." << endl;
            cout << "Error occurred at ";
            program_ctx.printOnShort( cout );
                 // << program_ctx;
            return res;
        }

        //program->append( StoredToken( Mod_Basic, CTRL_JSR_USERFUNC, TYP_CONTROL, programPos ) );
    }

    unsigned last_posn;
    StoredToken tkn(Mod_Basic, CTRL_PROGEND, TYP_CONTROL, 0);
    program->append(tkn, &last_posn);

    return 0;
}

// what am I, too good for stdio/ftell? geez...
// rope getline
int Compiler::getFileContents(const char *file, char **iv)
{
#ifdef _WIN32
    // unix does this automatically, duh
 //   if (1 || check_filecase_)
    {
        std::string truename = GetTrueName( file );
        std::string filepart = GetFilePart( file );
        if (truename != filepart && FileExists(file))
        {
            std::cout << "Case mismatch: " << endl
                << "  Specified:  " << filepart << endl
                << "  Filesystem: " << truename << endl;
        }
    }
#endif


    char *s = NULL;

    FILE *fp = fopen( file, "rb" );
    if (fp == NULL)
        return -1;
    
    fseek( fp, 0, SEEK_END );
    long filelen = ftell(fp);
    fseek( fp, 0, SEEK_SET );

    s = (char *) calloc(1,filelen+1);
    if (!s) 
	{
		fclose(fp);
		return -1;
	}

    fread( s, filelen, 1, fp );

    fclose(fp);
    *iv = s;
    return 0;
}

bool Compiler::read_function_declarations_in_included_file( const char *modulename )
{
    string filename_part = modulename;
	filename_part += ".inc";

    string filename_full = current_file_path + filename_part;
 
    if (filename_part[0] == ':')
    {
        const Package* pkg = NULL;
        string path;
        if (pkgdef_split( filename_part, NULL, &pkg, &path ))
        {
            if (pkg != NULL)
            {
                filename_full = pkg->dir() + path;
                string try_filename_full = pkg->dir() + "include/" + path;

                if (verbosity_level_ >= 10)
                    cout << "Searching for " << filename_full << endl;

                if (!FileExists( filename_full.c_str() ))
                {
                    if (verbosity_level_ >= 10)
                        cout << "Searching for " << try_filename_full << endl;
                    if (FileExists( try_filename_full.c_str() ))
                    {
                        if (verbosity_level_ >= 10)
                            cout << "Found " << try_filename_full << endl;

                        filename_full = try_filename_full;
                    }
                }
                else
                {
                    if (verbosity_level_ >= 10)
                        cout << "Found " << filename_full << endl;

                    if (FileExists( try_filename_full.c_str() ))
                        cout << "Warning: Found '" << filename_full.c_str() << "' and '"
                             << try_filename_full.c_str() << "'! Will use first file!" << endl;
                }
            }
            else
            {
                filename_full = compilercfg.PolScriptRoot + path;

                if (verbosity_level_ >= 10)
                {
                    cout << "Searching for " << filename_full << endl;
                    if (FileExists( filename_full.c_str() ))
                        cout << "Found " << filename_full << endl;
                }
            }
        }
        else
        {
            cout << "Unable to read include file '" << modulename << "'" << endl;
            return false;
        }
    }
    else
    {
        if (verbosity_level_ >= 10)
            cout << "Searching for " << filename_full << endl;

        if (!FileExists( filename_full.c_str() ))
        {
            string try_filename_full = compilercfg.IncludeDirectory + filename_part;
            if (verbosity_level_ >= 10)
                cout << "Searching for " << try_filename_full << endl;
            if (FileExists( try_filename_full.c_str() ))
            {
                if (verbosity_level_ >= 10)
                    cout << "Found " << try_filename_full << endl;

                filename_full = try_filename_full;
            }
        }
        else
        {
            if (verbosity_level_ >= 10)
                cout << "Found " << filename_full << endl;
        }
    }

	string filename_check = FullPath(filename_full.c_str());
    if (included.count(filename_check))
        return true;
    included.insert(filename_check);

    char *orig_mt;
    
    if (getFileContents( filename_full.c_str(), &orig_mt ))
    {
        cout << "Unable to read include file '" << filename_full << "'" << endl;
        return false;
    }
    
    CompilerContext mod_ctx( filename_full, program->add_dbg_filename(filename_full), orig_mt );

    string save = current_file_path;
    current_file_path = getpathof( filename_full );

    bool res = inner_read_function_declarations( mod_ctx );

    current_file_path = save;

    free(orig_mt);
    return res;
}

void Compiler::readCurLine( CompilerContext& ctx )
{
    ctx.skipws();
    ctx.skipcomments();

    stracpy(curLine, ctx.s, sizeof curLine);

    char *t;
    t = strchr(curLine, '\r');
    if (t) t[0] = '\0';
    t = strchr(curLine, '\n');
    if (t) t[0] = '\0';
}

bool Compiler::inner_read_function_declarations(const CompilerContext& ctx)
{
    CompilerContext tctx(ctx);
    Token tkn;
    for (;;)
    {
        readCurLine( tctx );

        CompilerContext save_ctx(tctx);
        if (getToken( tctx, tkn ) != 0)
            break;


        if (tkn.id == RSV_CONST)
        {
            if (handleConstDeclare( tctx ))
            {
                cout << "Error in const declaration" << endl;
                cout << tctx;
                return false;
            }
        }
        else if (tkn.id == RSV_ENUM)
        {
            if (handleEnumDeclare( tctx ))
            {
                cout << "Error in enum declaration" << endl;
                cout << tctx;
                return false;
            }
        }
        else if (tkn.id == RSV_FUNCTION || tkn.id == RSV_EXPORTED)
        {
            tctx = save_ctx;
            if (forward_read_function( tctx ))
            {
                cout << "Error reading function" << endl;
                cout << tctx;
                return false;
            }
        }
        else if (tkn.id == RSV_INCLUDE_FILE)
        {
            Token tk_module_name;

            if (getToken( tctx, tk_module_name ) == 0 &&
                ( tk_module_name.id == TOK_IDENT || tk_module_name.id == TOK_STRING) )
            {
                if (!read_function_declarations_in_included_file( tk_module_name.tokval() ))
                {
                    // read.. prints out an error message
                    return false;
                }
            }
        }
        else if (tkn.id == RSV_USE_MODULE)
        {
            Token tk_module_name;
            if (getToken( tctx, tk_module_name ) == 0 &&
                ( tk_module_name.id == TOK_IDENT || tk_module_name.id == TOK_STRING) )
            {
                int res = useModule( tk_module_name.tokval() );
                if (res < 0)
                    return false;
            }
        }
    }
    return true;
}

bool Compiler::read_function_declarations( const CompilerContext& ctx )
{
    bool res = inner_read_function_declarations( ctx );
    included.clear();
    constants.clear();
    program->clear_modules();
    return res;
}

int Compiler::emit_function( UserFunction& uf )
{
    CompilerContext ctx( uf.ctx );
    // cout << "emitting " << uf.name << ": " << program->tokens.next() << endl;
    int res = handleBracketedFunction3( uf, ctx );
    if (res < 0)
    {
        cout << "Error in function '" << uf.name << "'." << endl;
        ctx.printOn( cout );
    }
    return res;
}

void Compiler::patch_callers( UserFunction& uf )
{
    for( unsigned i = 0; i < uf.forward_callers.size(); ++i )
    {
        patchoffset( uf.forward_callers[i], uf.position );
    }
}

int Compiler::emit_functions()
{
    bool any;
    do {
        any = false;
        for( UserFunctions::iterator itr = userFunctions.begin(); itr != userFunctions.end(); ++itr )
        {
            UserFunction& uf = (*itr).second;
            if ((uf.exported || compiling_include || !uf.forward_callers.empty()) && 
                !uf.emitted)
            {
                int res = emit_function( uf );
                if (res < 0 ) return res;
                uf.emitted = any = true;
            }
        }
    } while (any);

    for( UserFunctions::iterator itr = userFunctions.begin(); itr != userFunctions.end(); ++itr )
    {
        UserFunction& uf = (*itr).second;
        if (uf.emitted)
        {
            patch_callers( uf );
        }
        else
        {
            // cout << "not emitted: " << uf.name << endl;
        }
    }
    return 0;
}

void Compiler::rollback( EScriptProgram& prog, const EScriptProgramCheckpoint& checkpoint )
{
    while (prog.modules.size() > checkpoint.module_count)
    {
        delete prog.modules.back();
        prog.modules.pop_back();
    }
    prog.tokens.setcount( checkpoint.tokens_count );
    prog.symbols.setlength( checkpoint.symbols_length );
    while (prog.sourcelines.size() > checkpoint.sourcelines_count)
        prog.sourcelines.pop_back();
    while (prog.fileline.size() > checkpoint.fileline_count)
        prog.fileline.pop_back();
    while (prog.dbg_filenum.size() > checkpoint.tokens_count)
    {
        prog.dbg_filenum.pop_back();
        prog.dbg_linenum.pop_back();
        prog.dbg_ins_blocks.pop_back();
        prog.dbg_ins_statementbegin.pop_back();
        prog.statementbegin = false;
    }

    for( UserFunctions::iterator itr = userFunctions.begin(); itr != userFunctions.end(); ++itr )
    {
        UserFunction& uf = (*itr).second;
        while (!uf.forward_callers.empty() && 
               uf.forward_callers.back() >= checkpoint.tokens_count)
        {
            uf.forward_callers.pop_back();
        }
    }
}
bool is_web_script( const char* file )
{
    const char* ext = strstr( file, ".hsr" );
    if (ext && memcmp( ext, ".hsr", 5 ) == 0)
        return true;
    ext = strstr( file, ".asp" );
    if (ext && memcmp( ext, ".asp", 5 ) == 0)
        return true;
    return false;
}

void preprocess_web_script( FileContents& fc )
{
    string output;
    output = "use http;";
    output += '\n';

    bool reading_html = true;
    bool source_is_emit = false;
    const char* s = fc.contents();
    string acc;
    while (*s)
    {
        if (reading_html)
        {
            if (s[0] == '<' && s[1] == '%')
            {
                reading_html = false;
                if (!acc.empty())
                {
                    output += "WriteHtmlRaw( \"" + acc + "\");\n";
                    acc = "";
                }
                s += 2;
                source_is_emit = (s[0] == '=');
                if (source_is_emit)
                {
                    output += "WriteHtmlRaw( "; 
                    ++s;
                }
            }
            else
            {
                if (*s == '\"')
                    acc += "\\\"";
                else if (*s == '\r')
                    ;
                else if (*s == '\n')
                    acc += "\\n";
                else
                    acc += *s;
                ++s;
            }
        }
        else
        {
            if (s[0] == '%' && s[1] == '>')
            {
                reading_html = true;
                s += 2;
                if (source_is_emit)
                    output += " );\n";
            }
            else
            {
                output += *s++;
            }
        }
    }
    if (!acc.empty())
        output += "WriteHtmlRaw( \"" + acc + "\");\n";
    fc.set_contents( output );
}

 // getcwd
int Compiler::compileFile(const char *in_file)
{
    int res = -1;
    try 
    {
        std::string filepath = FullPath( in_file );
        referencedPathnames.push_back( filepath );
        current_file_path = getpathof( filepath );
        if (verbosity_level_ >= 11)
            cout << "cfp: " << current_file_path << endl;
        FileContents fc( filepath.c_str() );

        if (is_web_script( filepath.c_str() ))
        {
            preprocess_web_script( fc );
        }

        CompilerContext ctx(filepath, program->add_dbg_filename(filepath), fc.contents());
        //divine_options( ctx );
        bool bres = read_function_declarations( ctx );
        // cout << "bres:" << bres << endl;
        if (!bres)
        {
            res = -1;
        }
        else
        {
            res = compile(ctx) ||
                  emit_functions();
        }
    }
	catch( const char *s )
	{
        cout << "Exception Detected:" << s << endl;
        res = -1;
	}
    catch( exception& ex )
    {
        cout << "Exception Detected:" << endl;
        cout << ex.what() << endl;
        res = -1;
    }
//    catch(...)
//    {
//        cout << "Generic Exception" << endl;
//        res = -1;
//    }

    if (res < 0)
        cout << "Compilation failed." << endl;
    
    //if (contains_tabs && Compiler.warnings_)
    //    cout << "Warning! Source contains TAB characters" << endl;
    
    return res;
}

#include "filefmt.h"

int Compiler::write(const char *fname)
{
    return program->write( fname );
}

int Compiler::write_dbg(const char *fname, bool gen_txt)
{
    return program->write_dbg( fname, gen_txt );
}

void Compiler::writeIncludedFilenames( const char* fname ) const
{
    ofstream ofs( fname, ios::out|ios::trunc );
//    ofs << current_file_path << endl;
    for( unsigned i = 0; i < referencedPathnames.size(); ++i )
    {
        ofs << referencedPathnames[i] << endl;
    }
}

void Compiler::dump( ostream& os )
{
    program->dump(os);
}
/* 
	local x;			[ "x", RSV_LOCAL, # ]
	local x:=5;			[ "x", RSV_LOCAL, 5, TOK_ASSIGN, # ]
	local x,y:=5;		[ "x", RSV_LOCAL, #, "y", TOK_LOCAL, 
	local x:=5,y;
	
	x := 5;
	
declare function foo(a,b,c,d);

function foo(a,b,c,d)
begin
	return
		or
	return "hey"
end

  statements:

   if expr [then] statement [else statement];

   do
     statement;
   while expr;

   begin
     statements;
   end


   while expr 
       statement;


   Alternative:
   if expr
     statments;
   [else
     statements; ]
   endif;
*/
#endif
