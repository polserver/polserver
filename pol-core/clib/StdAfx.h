/** @file
 *
 * stdafx.h: include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently
 */


#ifndef _CLIB_STDAFX_H
#define _CLIB_STDAFX_H

#include "Header_Windows.h"

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
#include <set>
#include <stack>
#include <stddef.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

// 3rd Party Includes
#include <format/format.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/flyweight.hpp>

// Project Includes (be really really carefull what to include!)

// explicit included since needed anyway for later includes
#include "compilerspecifics.h"
#include "Debugging/LogSink.h"
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
