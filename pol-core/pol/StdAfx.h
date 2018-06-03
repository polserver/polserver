/** @file
 *
 * stdafx.h: include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently
 *
 * @par History
 *
 * @note HowTo
 * - Add a normal Header file which includes everything you want in the pch.
 * - Add a normal cpp which only includes this header.
 * - In the project settings C/C++ Precompiled header: Use /Yu
 * - And since the pch header file needs to be included everywhere also use /FI (forced include
 * file) in the advanced section.
 * - The pch needs also to be created and thats why the additional cpp file exists, in the
 * properties of this file instead of "Use /Yu" set it to "Create /Yc"
 */


#if !defined( _POL_STDAFX_H )
#define _POL_STDAFX_H

#include <array>
#include <unordered_map>

#include <boost/variant.hpp>

// include default pch
#include "../clib/StdAfx.h"

#include "baseobject.h"     // uobject include
#include "dynproperties.h"  // uobject include
#include "gameclck.h"       // dynprop include
#include "proplist.h"       // uobject include

#include "uobject.h"

#include "layers.h"                   // network/packets include
#include "network/packetinterface.h"  // network/packets include
#include "pktboth.h"                  // network/packets include
#include "pktbothid.h"                // network/packets include
#include "pktdef.h"                   // network/packets include
#include "pktoutid.h"                 // network/packets include
#include "realms.h"                   // network/packets include
#include "uconst.h"                   // network/packets include

#include "network/packets.h"

#include "clidata.h"
#include "scrdef.h"
#include "utype.h"

#include "poltype.h"  // region include
#include "zone.h"     // region include

#include "region.h"

#include "crypt/cryptkey.h"
#include "mobile/attribute.h"
#include "skillid.h"
#include "udatfile.h"

#include "../bscript/execmodl.h"

#endif
