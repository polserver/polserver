#ifndef UOTOOL_CLIENTQUERIES_READERACCESS_H
#define UOTOOL_CLIENTQUERIES_READERACCESS_H

namespace Pol::Plib
{
class UoClientFiles;

// uotool owns the one process-wide reader instance (uoconvert threads its own
// instance explicitly instead). Defined in readeraccess.cpp rather than in plib
// so the library no longer exports an ambient reader global.
UoClientFiles& uofiles();
}  // namespace Pol::Plib

namespace Pol::UoTool
{
// uotool's load-on-first-use convenience. Many commands query the raw map/statics
// without an explicit load (and some never set map dimensions -- those were always
// broken and still are). These reproduce the historical lazy first-touch so uotool's
// behavior is unchanged; UoClientFiles' queries now assert-loaded instead of loading
// themselves, so the load policy lives here in the tool that wants it.
void ensure_map( Pol::Plib::UoClientFiles& uof );
void ensure_statics( Pol::Plib::UoClientFiles& uof );
}  // namespace Pol::UoTool

#endif  // UOTOOL_CLIENTQUERIES_READERACCESS_H
