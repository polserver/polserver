/** @file
 *
 * stdafx.h: include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently
 */


#if !defined( _POL_STDAFX_H )
#define _POL_STDAFX_H

#include <array>
#include <unordered_map>

// include default pch
#include "../clib/StdAfx.h"

#include "baseobject.h"     // uobject include
#include "dynproperties.h"  // uobject include
#include "gameclck.h"       // dynprop include
#include "proplist.h"       // uobject include

#include "uobject.h"

#include "../plib/uconst.h"           // network/packets include
#include "layers.h"                   // network/packets include
#include "network/packetinterface.h"  // network/packets include
#include "network/pktboth.h"          // network/packets include
#include "network/pktbothid.h"        // network/packets include
#include "network/pktdef.h"           // network/packets include
#include "network/pktoutid.h"         // network/packets include
#include "realms.h"                   // network/packets include

#include "network/packets.h"

#include "scrdef.h"
#include "utype.h"

#include "../plib/poltype.h"  // region include
#include "zone.h"             // region include

#include "region.h"

#include "../plib/clidata.h"
#include "../plib/udatfile.h"
#include "crypt/cryptkey.h"
#include "mobile/attribute.h"
#include "skillid.h"

#include "../bscript/execmodl.h"

#endif
