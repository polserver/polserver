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
#include "Debugging/LogSink.h"
#include "compilerspecifics.h"
#include "message_queue.h"

#include "fixalloc.h"
#include "logfacility.h"
#include "passert.h"
#include "rawtypes.h"
#include "refptr.h"
#include "spinlock.h"
#include "stlutil.h"
#include "strutil.h"
#include "weakptr.h"

#endif  // _CLIB_STDAFX_H
