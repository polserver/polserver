#ifndef EXCEPTION_PARSER_H
#define EXCEPTION_PARSER_H

#include <string>

namespace Pol{ namespace Clib{

class ExceptionParser
{
public:
	ExceptionParser();
	virtual ~ExceptionParser();

	/**
	 * Returns a string containing the current stack trace
	 */
	static std::string GetTrace();

	/**
	 * Initiates globally the exception catching (signal handlers for Linux)
	 */
	static void InitGlobalExceptionCatching();
};

}} // namespaces

#endif
