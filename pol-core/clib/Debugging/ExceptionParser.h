#ifndef EXCEPTION_PARSER_H
#define EXCEPTION_PARSER_H

#include "../stl_inc.h" //TODO: fix this header relocation if the build system is beautified

namespace Pol{ namespace Clib{

class ExceptionParser
{
public:
	ExceptionParser();
	virtual ~ExceptionParser();

	/**
	 * Returns a string containing the current stack trace
	 */
	static string GetTrace();

	/**
	 * Initiates globally the exception catching (signal handlers for Linux)
	 */
	static void InitGlobalExceptionCatching();
};

}} // namespaces

#endif
