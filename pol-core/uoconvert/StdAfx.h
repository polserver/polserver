/** @file
 *
 * stdafx.h: include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently
 *
 * @par History
 *
 * @par HowTo
 * - Add a normal Header file which includes everything you want in the pch.
 * - Add a normal cpp which only includes this header.
 * - In the project settings C/C++ Precompiled header: Use /Yu
 * - And since the pch header file needs to be included everywhere also use /FI (forced include file) in the advanced section.
 * - The pch needs also to be created and thats why the additional cpp file exists, in the properties of this file instead of "Use /Yu" set it to "Create /Yc"
 */


#if !defined(_UOCONVERT_STDAFX_H)
#define _UOCONVERT_STDAFX_H

// include default pch
#include "../clib/StdAfx.h"

#endif
