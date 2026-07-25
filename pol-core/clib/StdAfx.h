/** @file
 *
 * stdafx.h: include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently
 */


#ifndef _CLIB_STDAFX_H
#define _CLIB_STDAFX_H

// System Includes
#include <assert.h>
#include <atomic>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <ranges>
#include <set>
#include <stack>
#include <stddef.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

// 3rd Party Includes
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/flyweight.hpp>
#include <fmt/format.h>

// Project Includes (be really really carefull what to include!)

// explicit included since needed anyway for later includes
#include "clib/Debugging/LogSink.h"
#include "clib/compilerspecifics.h"
#include "clib/message_queue.h"

#include "clib/fixalloc.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "clib/refptr.h"
#include "clib/spinlock.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"
#include "clib/weakptr.h"

#endif  // _CLIB_STDAFX_H
