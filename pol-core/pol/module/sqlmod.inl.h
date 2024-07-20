#include "../polsem.h"
#include "../globals/network.h"

namespace Pol::Module
{
template <typename SQLConnectionType, typename SQLConnectionConnector>
Bscript::BObjectImp* SQLExecutorModule::background_connect( weak_ptr<Core::UOExecutor> uoexec,
                                                            SQLConnectionConnector connector )
{
  auto msg = [uoexec, connector = std::move( connector )]()
  {
    std::unique_ptr<Core::BSQLConnectionBase> sql;
    {
      Core::PolLock lck;
      sql = std::unique_ptr<Core::BSQLConnectionBase>( new SQLConnectionType );
    }
    if ( sql->getLastErrNo() )
    {
      Core::PolLock lck;
      if ( !uoexec.exists() )
        INFO_PRINTLN( "Script has been destroyed" );
      else
      {
        uoexec.get_weakptr()->ValueStack.back().set(
            new Bscript::BObject( new Bscript::BError( "Insufficient memory" ) ) );
        uoexec.get_weakptr()->revive();
      }
    }
    else if ( !connector( static_cast<SQLConnectionType*>( sql.get() ) ) )
    {
      Core::PolLock lck;
      if ( !uoexec.exists() )
        INFO_PRINTLN( "Script has been destroyed" );
      else
      {
        uoexec.get_weakptr()->ValueStack.back().set(
            new Bscript::BObject( new Bscript::BError( sql->getLastError() ) ) );
        uoexec.get_weakptr()->revive();
      }
    }
    else
    {
      Core::PolLock lck;
      if ( !uoexec.exists() )
        INFO_PRINTLN( "Script has been destroyed" );
      else
      {
        uoexec.get_weakptr()->ValueStack.back().set( new Bscript::BObject( sql.release() ) );
        uoexec.get_weakptr()->revive();
      }
    }
  };

  if ( !uoexec->suspend() )
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tThe execution of this script can't be blocked!",
        uoexec->scriptname(), uoexec->PC );
    return new Bscript::BError( "Script can't be blocked" );
  }

  Core::networkManager.sql_service->push( std::move( msg ) );
  return new Bscript::BLong( 0 );
}
}  // namespace Pol::Module
