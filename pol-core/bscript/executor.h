/** @file
 *
 * @par History
 * - 2009/09/05 Turley: Added struct .? and .- as shortcut for .exists() and .erase()
 */


#ifndef __EXECUTOR_H
#define __EXECUTOR_H


#include "pol_global_config.h"

#ifndef __EXECTYPE_H
#include "exectype.h"
#endif

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#include <exception>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "../clib/refptr.h"
#include "../clib/spinlock.h"
#include "bobject.h"
#include "eprog.h"
#include "executortype.h"
#include <format/format.h>


namespace Pol
{
namespace Core
{
class UOExecutor;

void list_script( UOExecutor* uoexec );
}  // namespace Core
namespace Bscript
{
class Executor;
class ExecutorModule;
class ModuleFunction;
class String;
class Token;
#ifdef ESCRIPT_PROFILE
#include <map>

struct profile_instr
{
  unsigned long sum;
  unsigned long max;
  unsigned long min;
  unsigned long count;
};
typedef std::map<std::string, profile_instr> escript_profile_map;
extern escript_profile_map EscriptProfileMap;
#endif

typedef std::vector<BObjectRef> ValueStackCont;

class ExecutorDebugListener
{
public:
  virtual void on_halt(){};
  virtual void on_destroy(){};
};

// FIXME: how to make this a nested struct in Executor?
struct ReturnContext
{
  unsigned PC;
  unsigned ValueStackDepth;
};

struct BackupStruct
{
  std::unique_ptr<BObjectRefVec> Locals;
  ValueStackCont ValueStack;
  unsigned PC;
};

enum class ExecutorType
{
  EXECUTOR,
  POL
};

class Executor
{
public:
  static Clib::SpinLock _executor_lock;
  virtual size_t sizeEstimate() const;

  friend void Core::list_script( Core::UOExecutor* uoexec );
  int done;
  void seterror( bool err );
  bool error() const;
  bool error_;
  bool halt_;
  bool run_ok_;

  virtual ExecutorType type() { return ExecutorType::EXECUTOR; }

  enum DEBUG_LEVEL
  {
    NONE,
    SOURCELINES,
    INSTRUCTIONS
  };
  DEBUG_LEVEL debug_level;
  unsigned PC;  // program counter

  bool AttachFunctionalityModules();


  bool setProgram( EScriptProgram* prog );

  BObjectRefVec Globals2;

  std::vector<BObjectRefVec*> upperLocals2;

  std::vector<ReturnContext> ControlStack;

  BObjectRefVec* Locals2;

  static UninitObject* m_SharedUninitObject;


  ValueStackCont ValueStack;

  static ExecInstrFunc GetInstrFunc( const Token& token );

  /*
      These must both be deleted.  instr references _symbols, so it should be deleted first.
      FIXME: there should be a separate object, called EProgram or something,
      that owns both the instructions and the symbols.  It should be ref_counted,
      so a code repository can store programs that multiple Executors use.
      That means debugger stuff has to come out of Instruction.
      */
  unsigned nLines;

  std::vector<BObjectRef> fparams;

  friend class ExecutorModule;
  void setFunctionResult( BObjectImp* imp );

protected:
  int getParams( unsigned howMany );
  void cleanParams();

public:
  int makeString( unsigned param );
  bool hasParams( unsigned howmany ) const { return ( fparams.size() >= howmany ); }
  size_t numParams() const { return fparams.size(); }
  BObjectImp* getParamImp( unsigned param );
  BObjectImp* getParamImp( unsigned param, BObjectImp::BObjectType type );
  BObjectImp* getParamImp2( unsigned param, BObjectImp::BObjectType type );
  BObject* getParamObj( unsigned param );

  const String* getStringParam( unsigned param );
  const BLong* getLongParam( unsigned param );

  bool getStringParam( unsigned param, const String*& pstr );
  bool getUnicodeStringParam( unsigned param, const String*& pstr );  // accepts also BLong array
  bool getParam( unsigned param, int& value );
  bool getParam( unsigned param, int& value, int maxval );
  bool getParam( unsigned param, int& value, int minval, int maxval );
  bool getRealParam( unsigned param, double& value );
  bool getObjArrayParam( unsigned param, ObjArray*& pobjarr );

  bool getParam( unsigned param, unsigned& value );

  bool getParam( unsigned param, unsigned short& value );
  bool getParam( unsigned param, unsigned short& value, unsigned short maxval );
  bool getParam( unsigned param, unsigned short& value, unsigned short minval,
                 unsigned short maxval );

  bool getParam( unsigned param, short& value );
  bool getParam( unsigned param, short& value, short maxval );
  bool getParam( unsigned param, short& value, short minval, short maxval );
  bool getParam( unsigned param, signed char& value );

  void* getApplicPtrParam( unsigned param, const BApplicObjType* pointer_type );
  BApplicObjBase* getApplicObjParam( unsigned param, const BApplicObjType* object_type );


  const char* paramAsString( unsigned param );
  double paramAsDouble( unsigned param );
  int paramAsLong( unsigned param );

protected:
  int makeDouble( unsigned param );


  BObject* getParam( unsigned param );

  BObject getValue( void );
  BObjectRef getObjRef( void );

public:
  int getToken( Token& token, unsigned position );
  BObjectRef& LocalVar( unsigned int varnum );
  BObjectRef& GlobalVar( unsigned int varnum );
  int makeGlobal( const Token& token );
  void popParam( const Token& token );
  void popParamByRef( const Token& token );
  void getArg( const Token& token );
  void pushArg( BObjectImp* arg );
  void pushArg( const BObjectRef& ref );

  static BObjectRef addmember( BObject& left, const BObject& right );
  static BObjectRef removemember( BObject& left, const BObject& right );
  static BObjectRef checkmember( BObject& left, const BObject& right );
  void addmember2( BObject& left, const BObject& right );
  static bool builtinMethodForced( const char*& methodname );

  // execmodules: modules associated with the current program.  References modules owned by
  // availmodules.
  std::vector<ExecutorModule*> execmodules;
  std::vector<ExecutorModule*> availmodules;  // owns

public:
  Executor();
  virtual ~Executor();

  void addModule( ExecutorModule* module );  // NOTE, executor deletes its modules when done
  ExecutorModule* findModule( const std::string& name );

  ModuleFunction* current_module_function;
  // NOTE: the debugger code expects these to be virtual..
  void execFunc( const Token& token );
  void execInstr();

  void ins_nop( const Instruction& ins );
  void ins_jmpiftrue( const Instruction& ins );
  void ins_jmpiffalse( const Instruction& ins );
  void ins_skipiftrue_else_consume( const Instruction& ins );
  void ins_globalvar( const Instruction& ins );
  void ins_localvar( const Instruction& ins );
  void ins_makeLocal( const Instruction& ins );
  void ins_declareArray( const Instruction& ins );
  void ins_long( const Instruction& ins );
  void ins_double( const Instruction& ins );
  void ins_string( const Instruction& ins );
  void ins_error( const Instruction& ins );
  void ins_struct( const Instruction& ins );
  void ins_array( const Instruction& ins );
  void ins_dictionary( const Instruction& ins );
  void ins_uninit( const Instruction& ins );
  void ins_ident( const Instruction& ins );
  void ins_unminus( const Instruction& ins );
  void ins_unplusplus( const Instruction& ins );
  void ins_unminusminus( const Instruction& ins );
  void ins_unplusplus_post( const Instruction& ins );
  void ins_unminusminus_post( const Instruction& ins );

  void ins_logical_and( const Instruction& ins );
  void ins_logical_or( const Instruction& ins );
  void ins_logical_not( const Instruction& ins );

  void ins_bitwise_not( const Instruction& ins );

  void ins_set_member( const Instruction& ins );
  void ins_set_member_consume( const Instruction& ins );
  void ins_get_member( const Instruction& ins );
  void ins_get_member_id( const Instruction& ins );                       // test id
  void ins_set_member_id( const Instruction& ins );                       // test id
  void ins_set_member_id_consume( const Instruction& ins );               // test id
  void ins_set_member_id_consume_plusequal( const Instruction& ins );     // test id
  void ins_set_member_id_consume_minusequal( const Instruction& ins );    // test id
  void ins_set_member_id_consume_timesequal( const Instruction& ins );    // test id
  void ins_set_member_id_consume_divideequal( const Instruction& ins );   // test id
  void ins_set_member_id_consume_modulusequal( const Instruction& ins );  // test id
  void ins_set_member_id_unplusplus( const Instruction& ins );            // test id
  void ins_set_member_id_unminusminus( const Instruction& ins );          // test id
  void ins_set_member_id_unplusplus_post( const Instruction& ins );       // test id
  void ins_set_member_id_unminusminus_post( const Instruction& ins );     // test id

  void ins_assign_localvar( const Instruction& ins );
  void ins_assign_globalvar( const Instruction& ins );
  void ins_assign_consume( const Instruction& ins );
  void ins_consume( const Instruction& ins );
  void ins_assign( const Instruction& ins );
  void ins_array_assign( const Instruction& ins );
  void ins_array_assign_consume( const Instruction& ins );
  void ins_multisubscript_assign( const Instruction& ins );
  void ins_multisubscript_assign_consume( const Instruction& ins );
  void ins_multisubscript( const Instruction& ins );

  void ins_add( const Instruction& ins );
  void ins_subtract( const Instruction& ins );
  void ins_mult( const Instruction& ins );
  void ins_div( const Instruction& ins );
  void ins_modulus( const Instruction& ins );

  void ins_interpolate_string( const Instruction& ins );
  void ins_format_expression( const Instruction& ins );

  void ins_insert_into( const Instruction& ins );

  void ins_plusequal( const Instruction& ins );
  void ins_minusequal( const Instruction& ins );
  void ins_timesequal( const Instruction& ins );
  void ins_divideequal( const Instruction& ins );
  void ins_modulusequal( const Instruction& ins );

  void ins_bitshift_right( const Instruction& ins );
  void ins_bitshift_left( const Instruction& ins );
  void ins_bitwise_and( const Instruction& ins );
  void ins_bitwise_xor( const Instruction& ins );
  void ins_bitwise_or( const Instruction& ins );

  void ins_equal( const Instruction& ins );
  void ins_notequal( const Instruction& ins );
  void ins_lessthan( const Instruction& ins );
  void ins_lessequal( const Instruction& ins );
  void ins_greaterthan( const Instruction& ins );
  void ins_greaterequal( const Instruction& ins );

  void ins_goto( const Instruction& ins );
  void ins_arraysubscript( const Instruction& ins );
  void ins_func( const Instruction& ins );
  void ins_call_method( const Instruction& ins );
  void ins_call_method_id( const Instruction& ins );
  void ins_statementbegin( const Instruction& ins );
  void ins_progend( const Instruction& ins );
  void ins_makelocal( const Instruction& ins );
  void ins_jsr_userfunc( const Instruction& ins );
  void ins_pop_param( const Instruction& ins );
  void ins_pop_param_byref( const Instruction& ins );
  void ins_get_arg( const Instruction& ins );
  void ins_leave_block( const Instruction& ins );
  void ins_gosub( const Instruction& ins );
  void ins_return( const Instruction& ins );
  void ins_exit( const Instruction& ins );

  void ins_member( const Instruction& ins );
  void ins_addmember( const Instruction& ins );
  void ins_removemember( const Instruction& ins );
  void ins_checkmember( const Instruction& ins );
  void ins_dictionary_addmember( const Instruction& ins );
  void ins_addmember2( const Instruction& ins );
  void ins_addmember_assign( const Instruction& ins );
  void ins_in( const Instruction& ins );

  void ins_initforeach( const Instruction& ins );
  void ins_stepforeach( const Instruction& ins );
  void ins_initforeach2( const Instruction& ins );
  void ins_stepforeach2( const Instruction& ins );

  void ins_casejmp( const Instruction& ins );
  void ins_initfor( const Instruction& ins );
  void ins_nextfor( const Instruction& ins );

  void ins_funcref( const Instruction& ins );

  static int ins_casejmp_findlong( const Token& token, BLong* blong );
  static int ins_casejmp_findstring( const Token& token, String* bstringimp );
  static int ins_casejmp_finddefault( const Token& token );

  bool running_to_completion() const;
  void set_running_to_completion( bool to_completion );

  bool runnable() const;
  void calcrunnable();

  bool halt() const;
  void sethalt( bool halt );

  bool debugging() const;
  void setdebugging( bool debugging );

  bool attach_debugger( std::weak_ptr<ExecutorDebugListener> listener = {} );
  void detach_debugger();
  std::string dbg_get_instruction( size_t atPC ) const;
  void dbg_ins_trace();
  void dbg_step_into();
  void dbg_step_over();
  void dbg_run();
  void dbg_break();
  void dbg_setbp( unsigned atPC );
  void dbg_clrbp( unsigned atPC );
  void dbg_clrallbp();

  bool exec();
  void reinitExec();
  void initForFnCall( unsigned in_PC );
  void show_context( unsigned atPC );
  void show_context( fmt::Writer& os, unsigned atPC );

  int getDebugLevel() { return debug_level; }
  void setDebugLevel( DEBUG_LEVEL level ) { debug_level = level; }
  void setViewMode( bool vm ) { viewmode_ = vm; }
  const std::string& scriptname() const;
  bool empty_scriptname();
  const EScriptProgram* prog() const;

private:
  ref_ptr<EScriptProgram> prog_;
  bool prog_ok_;
  bool viewmode_;

  bool runs_to_completion_;

  bool debugging_;
  enum DEBUG_STATE
  {
    DEBUG_STATE_NONE,
    DEBUG_STATE_ATTACHING,
    DEBUG_STATE_ATTACHED,
    DEBUG_STATE_INS_TRACE,
    DEBUG_STATE_INS_TRACE_BRK,
    DEBUG_STATE_RUN,
    DEBUG_STATE_BREAK_INTO,
    DEBUG_STATE_STEP_INTO,
    DEBUG_STATE_STEPPING_INTO,
    DEBUG_STATE_STEP_OVER,
    DEBUG_STATE_STEPPING_OVER
  };
  DEBUG_STATE debug_state_;
  std::set<unsigned> breakpoints_;
  std::set<unsigned> tmpbreakpoints_;
  unsigned bp_skip_;

  BObjectImp* func_result_;

private:  // not implemented
  Executor( const Executor& exec );
  Executor& operator=( const Executor& exec );
  std::weak_ptr<ExecutorDebugListener> _listener;
#ifdef ESCRIPT_PROFILE
  unsigned long GetTimeUs();
  void profile_escript( std::string name, unsigned long profile_start );
#endif
};

inline const std::string& Executor::scriptname() const
{
  return prog_->name;
}

inline bool Executor::empty_scriptname()
{
  return prog_->name.get().empty();
}

inline const EScriptProgram* Executor::prog() const
{
  return prog_.get();
}

inline bool Executor::runnable( void ) const
{
  return run_ok_;
}
inline void Executor::calcrunnable()
{
  run_ok_ = !error_ && !halt_;
}

inline void Executor::seterror( bool err )
{
  error_ = err;
  calcrunnable();
}
inline bool Executor::error() const
{
  return error_;
}

inline bool Executor::halt() const
{
  return halt_;
}

inline bool Executor::debugging() const
{
  return debugging_;
}
inline void Executor::setdebugging( bool debugging )
{
  debugging_ = debugging;
}


inline bool Executor::running_to_completion() const
{
  return runs_to_completion_;
}
inline void Executor::set_running_to_completion( bool to_completion )
{
  runs_to_completion_ = to_completion;
}
}  // namespace Bscript
}  // namespace Pol
#endif
