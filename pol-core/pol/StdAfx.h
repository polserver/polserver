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
#include "clib/StdAfx.h"

#include "pol/baseobject.h"     // uobject include
#include "pol/dynproperties.h"  // uobject include
#include "pol/gameclck.h"       // dynprop include
#include "pol/proplist.h"       // uobject include

#include "pol/uobject.h"

#include "plib/uconst.h"           // network/packets include
#include "pol/layers.h"                   // network/packets include
#include "pol/network/packetinterface.h"  // network/packets include
#include "pol/network/pktboth.h"          // network/packets include
#include "pol/network/pktbothid.h"        // network/packets include
#include "pol/network/pktdef.h"           // network/packets include
#include "pol/network/pktoutid.h"         // network/packets include
#include "pol/realms/realms.h"            // network/packets include

#include "pol/network/packets.h"

#include "pol/scrdef.h"
#include "pol/utype.h"

#include "plib/poltype.h"  // region include
#include "pol/zone.h"             // region include

#include "pol/regions/region.h"

#include "plib/clidata.h"
#include "plib/udatfile.h"
#include "pol/mobile/attribute.h"
#include "pol/skillid.h"

#include "bscript/execmodl.h"

#endif
