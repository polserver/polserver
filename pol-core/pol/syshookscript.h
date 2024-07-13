/** @file
 *
 * @par History
 */


#ifndef SYSHOOKSCRIPT_H
#define SYSHOOKSCRIPT_H

#include <string>
#include <vector>

#include "../bscript/bobject.h"
#include "scrdef.h"
#include "uoexec.h"

namespace Pol
{
namespace Bscript
{
struct BackupStruct;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Plib
{
class Package;
}
namespace Core
{
class ExportScript
{
public:
  ExportScript( const Plib::Package* pkg, std::string scriptname );
  ExportScript( const ScriptDef& isd );
  bool Initialize();
  const std::string& scriptname() const;
  bool FindExportedFunction( const std::string& name, unsigned args, unsigned& PC ) const;
  bool FindExportedFunction( const char* name, unsigned args, unsigned& PC ) const;

  bool call( unsigned PC, Bscript::BObjectImp* p0 );                           // throw()
  bool call( unsigned PC, Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 );  // throw()
  bool call( unsigned PC, Bscript::BObjectImp* p0, Bscript::BObjectImp* p1,
             Bscript::BObjectImp* p2 );  // throw()
  bool call( unsigned PC, Bscript::BObjectImp* p0, Bscript::BObjectImp* p1, Bscript::BObjectImp* p2,
             Bscript::BObjectImp* p3 );  // throw()

  std::string call_string( unsigned PC, Bscript::BObjectImp* p0,
                           Bscript::BObjectImp* p1 );  // throw()
  std::string call_string( unsigned PC, Bscript::BObjectImp* p0, Bscript::BObjectImp* p1,
                           Bscript::BObjectImp* p2 );  // throw()

  int call_long( unsigned PC, Bscript::BObjectImp* p0 );                           // throw()
  int call_long( unsigned PC, Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 );  // throw()

  Bscript::BObjectImp* call( unsigned PC, Bscript::BObjectImp* p0,
                             std::vector<Bscript::BObjectRef>& pmore );
  Bscript::BObject call( unsigned PC, Bscript::BObjectImp* p0, Bscript::BObjectImpRefVec& pmore );

  Bscript::BObject call_object( unsigned PC, Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 );
  Bscript::BObject call_object( unsigned PC, Bscript::BObjectImp* p0, Bscript::BObjectImp* p1,
                                Bscript::BObjectImp* p2 );

  void SaveStack( Bscript::BackupStruct& backup );
  void LoadStack( Bscript::BackupStruct& backup );

  size_t estimateSize() const;
  friend class SystemHook;

private:
  bool check_bool();
  int check_int();
  std::string check_string();
  Bscript::BObjectImp* check_imp();

  ScriptDef sd;
  UOExecutor uoexec;
};
}  // namespace Core
}  // namespace Pol

#endif
