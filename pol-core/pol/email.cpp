#include <boost/algorithm/string/predicate.hpp>  // for istarts_with

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "globals/settings.h"


namespace Pol::Core
{

void clear_email_cfg()
{
  settingsManager.email_cfg.url = "";
  settingsManager.email_cfg.username = "";
  settingsManager.email_cfg.password = "";
  settingsManager.email_cfg.use_tls = false;
}

void load_email_cfg()
{
  if ( !Clib::FileExists( "config/email.cfg" ) )
  {
    clear_email_cfg();
    return;
  }

  Clib::ConfigFile cf( "config/email.cfg" );
  Clib::ConfigElem elem;

  cf.readraw( elem );

  settingsManager.email_cfg.url = elem.remove_string( "URL" );

  if ( !boost::algorithm::istarts_with( settingsManager.email_cfg.url, "smtp://" ) &&
       !boost::algorithm::istarts_with( settingsManager.email_cfg.url, "smtps://" ) )
  {
    clear_email_cfg();
    elem.throw_error( "URL must start with 'smtp://' or 'smtps://'." );
  }
  settingsManager.email_cfg.username = elem.remove_string( "Username", "" );
  settingsManager.email_cfg.password = elem.remove_string( "Password", "" );
  settingsManager.email_cfg.use_tls = elem.remove_bool( "UseTLS", false );
  settingsManager.email_cfg.ca_file = elem.remove_string( "CAFile", "" );
  if ( !settingsManager.email_cfg.ca_file.empty() &&
       !Clib::FileExists( settingsManager.email_cfg.ca_file ) )
  {
    clear_email_cfg();
    elem.throw_error(
        fmt::format( "CAFile '{}' does not exist.", settingsManager.email_cfg.ca_file ) );
  }
}
}  // namespace Pol::Core
