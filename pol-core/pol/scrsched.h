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
//
// template <typename... Args>
// void start_script_callback( const char* filename,
//                            std::function<void( Bscript::BObjectImp* )> callback, Args... args );

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
//
//
// void start_script_callback2( const char* filename,
//                             std::function<void( Bscript::BObjectImp* )> callback,
//                             const std::initializer_list<Bscript::BObjectImp*>& args );

class RunScriptImpl
{
  template <typename... Args>
  friend void Core::start_script( const char* filename, Args... args );

  template <typename... Args>
  friend Module::UOExecutorModule* Core::start_script( const ScriptDef& script, Args... args );

  template <typename... Args>
  friend Module::UOExecutorModule* Core::start_script( ref_ptr<Bscript::EScriptProgram> program,
                                                       Args... args );
  template <typename... Args>
  friend bool Core::run_script_to_completion( const char* filename, Args... args );

  template <typename... Args>
  friend Bscript::BObjectImp* Core::run_script_to_completion( const ScriptDef& script,
                                                              Args... args );

  template <typename... Args>
  friend bool Core::call_script( const ScriptDef& script,
                                 Args... args );  // throw()

  static void start_script( const char* filename,
                            const std::initializer_list<Bscript::BObjectImp*>& args );

  static Module::UOExecutorModule* start_script(
      const ScriptDef& script, const std::initializer_list<Bscript::BObjectImp*>& args );

  static Module::UOExecutorModule* start_script(
      ref_ptr<Bscript::EScriptProgram> program,
      const std::initializer_list<Bscript::BObjectImp*>& args );

  static bool run_script_to_completion( const char* filename,
                                        const std::initializer_list<Bscript::BObjectImp*>& args );

  static Bscript::BObjectImp* run_script_to_completion(
      const ScriptDef& script, const std::initializer_list<Bscript::BObjectImp*>& args );

  static bool call_script( const ScriptDef& script,
                           const std::initializer_list<Bscript::BObjectImp*>& args );  // throw()
};

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
  RunScriptImpl::start_script( filename, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

// template <typename... Args>
// void start_script_callback( const char* filename,
//                            std::function<void( Bscript::BObjectImp* )> callback, Args... args )
//{
//  start_script_callback2( filename, callback, std::initializer_list<Bscript::BObjectImp*>{args...}
//  );
//}
template <typename... Args>
Module::UOExecutorModule* start_script( const ScriptDef& script, Args... args )
{
  return RunScriptImpl::start_script( script,
                                      std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
Module::UOExecutorModule* start_script( ref_ptr<Bscript::EScriptProgram> program, Args... args )
{
  return RunScriptImpl::start_script( program,
                                      std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
bool run_script_to_completion( const char* filename, Args... args )
{
  return RunScriptImpl::run_script_to_completion(
      filename, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script, Args... args )
{
  return RunScriptImpl::run_script_to_completion(
      script, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

template <typename... Args>
bool call_script( const ScriptDef& script, Args... args )
{
  return RunScriptImpl::call_script( script, std::initializer_list<Bscript::BObjectImp*>{args...} );
}

}  // namespace Core
}  // namespace Pol
#endif
