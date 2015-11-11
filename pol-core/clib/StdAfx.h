/*
History
=======

Notes
=======
stdafx.h : include file for standard system include files,
           or project specific include files that are used frequently, but
           are changed infrequently
HowTo:
Add a normal Header file which includes everything you want in the pch.
Add a normal cpp which only includes this header.

In the project settings C/C++ Precompiled header: Use /Yu
And since the pch header file needs to be included everywhere also use /FI (forced include file) in the advanced section.

The pch needs also to be created and thats why the additional cpp file exists, in the properties of this file instead of "Use /Yu" set it to "Create /Yc"
*/

#ifndef _CLIB_STDAFX_H
#define _CLIB_STDAFX_H

#include <Header_Windows.h>

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
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/any.hpp>
#include <boost/flyweight.hpp>
#include <boost/noncopyable.hpp>
#include "../../lib/format/format.h"

// Project Includes (be really really carefull what to include!)

// explicit included since needed anyway for later includes
#include "compileassert.h"
#include "compilerspecifics.h" 
#include "Debugging/LogSink.h"
#include "message_queue.h"

#include "passert.h"
#include "logfacility.h"
#include "stlutil.h"
#include "strutil.h"
#include "rawtypes.h"
#include "fixalloc.h"
#include "refptr.h"

#endif // _CLIB_STDAFX_H
