/** @file
 *
 * @par History
 */


#ifndef __SCRSCHED_H
#define __SCRSCHED_H

#include <initializer_list>
#include <list>
#include <set>

#include "../clib/refptr.h"
#include "polclock.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class EScriptProgram;
}  // namespace Bscript
namespace Module
{
class UOExecutorModule;
}
namespace Core
{
class ScriptDef;
class UOExecutor;

void step_scripts( polclock_t* clocksleft, bool* pactivity );


UOExecutor* create_script_executor();
UOExecutor* create_full_script_executor();

void schedule_executor( UOExecutor* ex );

Bscript::BObjectImp* run_executor_to_completion( UOExecutor& ex, const ScriptDef& script );

bool find_uoexec( unsigned int pid, UOExecutor** pp_uoexec );

template <typename... Args>
void start_script( const char* filename, Args... args );

template <typename... Args>
Module::UOExecutorModule* start_script( const ScriptDef& script, Args... args );

template <typename... Args>
Module::UOExecutorModule* start_script( ref_ptr<Bscript::EScriptProgram> program, Args... args );

template <typename... Args>
bool run_script_to_completion( const char* filename, Args... args );

template <typename... Args>
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script, Args... args );

template <typename... Args>
bool call_script( const ScriptDef& script,
                  Args... args );  // throw()

void start_script2( const char* filename, const std::initializer_list<Bscript::BObjectImp*>& args );

Module::UOExecutorModule* start_script2( const ScriptDef& script,
                                         const std::initializer_list<Bscript::BObjectImp*>& args );

Module::UOExecutorModule* start_script2( ref_ptr<Bscript::EScriptProgram> program,
                                         const std::initializer_list<Bscript::BObjectImp*>& args );

bool run_script_to_completion2( const char* filename,
                                const std::initializer_list<Bscript::BObjectImp*>& args );

Bscript::BObjectImp* run_script_to_completion2(
    const ScriptDef& script, const std::initializer_list<Bscript::BObjectImp*>& args );

bool call_script2( const ScriptDef& script,
                   const std::initializer_list<Bscript::BObjectImp*>& args );  // throw()
}  // namespace Core
}  // namespace Pol

namespace Pol
{
namespace Module
{
class UOExecutorModule;
}
namespace Core
{
template <typename... Args>
void start_script( const char* filename, Args... args )
{
  start_script2( filename, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
Module::UOExecutorModule* start_script( const ScriptDef& script, Args... args )
{
  return start_script2( script, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
Module::UOExecutorModule* start_script( ref_ptr<Bscript::EScriptProgram> program, Args... args )
{
  return start_script2( program, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
bool run_script_to_completion( const char* filename, Args... args )
{
  return run_script_to_completion2( filename,
                                    std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script, Args... args )
{
  return run_script_to_completion2( script, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
bool call_script( const ScriptDef& script, Args... args )
{
  return call_script2( script, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

}  // namespace Core
}  // namespace Pol
#endif
