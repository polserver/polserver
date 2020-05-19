/** @file
 *
 * @par History
 */


#ifndef H_COMPILER_H
#define H_COMPILER_H

#include "compctx.h"
#include "tokens.h"
#include "userfunc.h"

#ifndef __PARSER_H
#include "parser.h"
#endif

#include <iosfwd>
#include <map>
#include <stddef.h>
#include <string>
#include <vector>

#include "../clib/maputil.h"
#include "../clib/refptr.h"

namespace Pol
{
namespace Bscript
{
class ModuleFunction;
class Token;
class UserFunction;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class EScriptProgram;
class EScriptProgramCheckpoint;
class FunctionalityModule;
/*
    ack, this is a misnomer.
    "CanBeLabelled" means "break or continue can happen here."
    enum eb_type { CanBeLabelled = true, CanNotBeLabelled = false };
    */

enum eb_label_ok
{
  CanBeLabelled = true,
  CanNotBeLabelled = false
};
enum eb_break_ok
{
  BreakOk = true,
  BreakNotOk = false
};
enum eb_continue_ok
{
  ContinueOk = true,
  ContinueNotOk = false
};

struct BlockDesc
{
  unsigned varcount;  // how many variables in this block?
  unsigned valcount;  // how many values on the stack should be removed?

  eb_label_ok label_ok;
  eb_break_ok break_ok;
  eb_continue_ok continue_ok;

  std::string label;  // label of construct

  // addresses of tokens whos offset needs to be patched with
  // the final break/continue jump addresses.
  typedef std::vector<unsigned> TokenAddrs;
  TokenAddrs break_tokens;
  TokenAddrs continue_tokens;

  unsigned blockidx;
};

struct Variable
{
  std::string name;
  mutable bool used = false;
  mutable bool unused = false;
  CompilerContext ctx;
};
typedef std::vector<Variable> Variables;

class Scope
{
public:
  BlockDesc& pushblock();
  void popblock( bool varsOnly );
  void addvar( const std::string& varname, const CompilerContext& ctx, bool warn_on_notused = true,
               bool unused = false );
  void addvalue();
  bool inblock() const { return !blockdescs_.empty(); }
  unsigned numVarsInBlock() const { return blockdescs_.back().varcount; }
  const BlockDesc& blockdesc() const { return blockdescs_.back(); }
  bool varexists( const std::string& varname, unsigned& idx ) const;
  bool varexists( const std::string& varname ) const;
  unsigned int numVariables() const { return static_cast<unsigned int>( variables_.size() ); }

private:
  Variables variables_;

  // we may need a blocktag, too, in which case this
  // should be a stack/vector of a struct.
  std::vector<BlockDesc> blockdescs_;
  friend class Compiler;
};

class Compiler final : public SmartParser
{
public:
  static bool check_filecase_;
  static int verbosity_level_;
  static void setCheckFileCase( bool check ) { check_filecase_ = check; }
  static void setVerbosityLevel( int vlev ) { verbosity_level_ = vlev; }

private:
  std::string current_file_path;


  int curSourceFile;
  /** Part of the content of the line being read, only for info/debug output, limited to 80 chars */
  char curLine[80];
  int inExpr;
  int inFunction;
  bool haveProgram;
  bool compiling_include;
  unsigned programPos;
  unsigned nProgramArgs;

  CompilerContext program_ctx;
  char* program_source;

  typedef std::set<std::string, Clib::ci_cmp_pred> INCLUDES;
  INCLUDES included;

  std::vector<std::string> referencedPathnames;

  ref_ptr<EScriptProgram> program;
  typedef std::map<std::string, UserFunction, Clib::ci_cmp_pred> UserFunctions;
  UserFunctions userFunctions;

  int findLabel( Token& tok, unsigned& posn );
  int enterLabel( Token& tok );

  typedef std::map<std::string, Token> Constants;
  Constants constants;

  /*
  scopes:
  because functions don't nest, a stack of block scopes isn't necessary.
  when we enter a function, we'll enter a scope.
  exiting the function automatically removes its "locals"
  we'll still end up inserting a "leave block", but that
  could be optimized out, I suppose.
  */
  Variables globals_;

  bool globalexists( const std::string& varname, unsigned& idx,
                     CompilerContext* atctx = nullptr ) const;

  Scope localscope;

  bool varexists( const std::string& varname ) const;

  bool inGlobalScope() const { return localscope.inblock() == false; }
  /*
  special note on latest_label, enterblock, and leaveblock
  latest_label is a hidden parameter to enterblock, set up
  by getStatement processing a label ("tag:").
  note, getStatement checks to make sure the label is
  followed by a WHILE or DO statement.
  */
  std::string latest_label;
  void enterblock( eb_label_ok eblabel, eb_break_ok ebbreak, eb_continue_ok ebcontinue );
  void enterblock( eb_label_ok et );
  int readblock( CompilerContext& ctx, int level, BTokenId endtokenid,
                 BTokenId* last_statement_id = nullptr, Token* block_end = nullptr );
  void leaveblock( unsigned breakPC, unsigned continuePC );
  void emit_leaveblock();
  void patchblock_continues( unsigned continuePC );
  void patchblock_breaks( unsigned breakPC );

  void rollback( EScriptProgram& prog, const EScriptProgramCheckpoint& checkpoint );
  bool substitute_constant( Token* tkn ) const;  // returns true if a constant was found.
  void substitute_constants( Expression& expr ) const;
  void convert_variables( Expression& expr ) const;
  int validate( const Expression& expr, CompilerContext& ctx ) const;
  int readexpr( Expression& expr, CompilerContext& ctx, unsigned flags );
  int read_subexpression( Expression& expr, CompilerContext& ctx, unsigned flags );
  void inject( Expression& expr );
  int insertBreak( const std::string& label );

public:
  Compiler();
  ~Compiler();

  void dump( std::ostream& os );
  void setIncludeCompileMode() { compiling_include = true; }
  void addModule( FunctionalityModule* module );
  int useModule( const char* modulename );
  int includeModule( const std::string& modulename );
  virtual int isFunc( Token& tok, ModuleFunction** v ) override;
  virtual int isUserFunc( Token& tok, UserFunction** userfunc ) override;

  void patchoffset( unsigned instruc, unsigned newoffset );
  void addToken( Token& tok );

  virtual int isLegal( Token& tok ) override;

  virtual int getUserArgs( Expression& ex, CompilerContext& ctx, bool inject_jsr ) override;
  virtual int getArrayElements( Expression& expr, CompilerContext& ctx ) override;
  virtual int getNewArrayElements( Expression& expr, CompilerContext& ctx ) override;
  virtual int getStructMembers( Expression& expr, CompilerContext& ctx ) override;
  virtual int getDictionaryMembers( Expression& expr, CompilerContext& ctx ) override;
  virtual int getMethodArguments( Expression& expr, CompilerContext& ctx, int& nargs ) override;
  virtual int getFunctionPArgument( Expression& expr, CompilerContext& ctx,
                                    Token* tok ) override;

  int eatToken( CompilerContext& ctx, BTokenId tokenid );
  int getExpr( CompilerContext& ctx, unsigned expr_flags, size_t* exprlen = nullptr,
               Expression* ex = nullptr );
  int getExpr2( CompilerContext& ctx, unsigned expr_flags, Expression* ex = nullptr );
  int getExprInParens( CompilerContext& ctx, Expression* ex = nullptr );
  int getSimpleExpr( CompilerContext& ctx );

  int handleProgram( CompilerContext& ctx, int level );
  int handleProgram2( CompilerContext& ctx, int level );
  int readFunctionDeclaration( CompilerContext& ctx, UserFunction& userfunc );
  int handleDoClause( CompilerContext& ctx, int level );
  int handleRepeatUntil( CompilerContext& ctx, int level );
  int handleForEach( CompilerContext& ctx, int level );
  int handleReturn( CompilerContext& ctx );
  int handleExit( CompilerContext& ctx );
  int handleBreak( CompilerContext& ctx );
  int handleContinue( CompilerContext& ctx );
  int handleBlock( CompilerContext& ctx, int level );
  int handleBracketedIf( CompilerContext& ctx, int level );
  int handleBracketedWhile( CompilerContext& ctx, int level );
  int handleVarDeclare( CompilerContext& ctx, unsigned save_id );
  int handleConstDeclare( CompilerContext& ctx );
  int handleEnumDeclare( CompilerContext& ctx );
  int handleUse( CompilerContext& ctx );
  int handleInclude( CompilerContext& ctx );
  int handleFor( CompilerContext& ctx );
  int handleBracketedFor_c( CompilerContext& ctx );
  int handleBracketedFor_basic( CompilerContext& ctx );
  int handleSwitch( CompilerContext& ctx, int level );

  void emitFileLine( CompilerContext& ctx );
  void emitFileLineIfFileChanged( CompilerContext& ctx );
  void readCurLine( CompilerContext& ctx );
  void savesourceline();

  int getStatement( CompilerContext& ctx /*char **s */, int level );
  int _getStatement( CompilerContext& ctx /*char **s */, int level );

  int getFileContents( const char* filename, char** contents );
  int compileFile( const char* fname );
  int compileContext( CompilerContext& ctx );
  int compile( CompilerContext& ctx /* char *s */ );

  int write( const char* fname );
  int write_dbg( const char* fname, bool generate_txtfile );
  void writeIncludedFilenames( const char* fname ) const;

  // phase 0: determining bracket syntax

  // phase 1: read function declarations, constants (but clear constants)
  int forward_read_function( CompilerContext& ctx );
  bool read_function_declarations( const CompilerContext& ctx );
  bool read_function_declarations_in_included_file( const char* modulename );
  bool inner_read_function_declarations( const CompilerContext& ctx );

  // phase 2: compile the source
  int handleBracketedFunction2( CompilerContext& ctx, int level, int tokentype );

  // phase 3: emit functions that are actually used.
  int handleBracketedFunction3( UserFunction& userfunc, CompilerContext& ctx );
  int emit_function( UserFunction& uf );
  int emit_functions();
  void patch_callers( UserFunction& uf );

private:
  std::vector<char*> delete_these_arrays;
};
}
}
#endif  // H_COMPILER_H
