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
     *
     * @return the stack trace as multi-line string
     */
    static std::string getTrace();

    /**
     * Reports a program abort to the core devs
     *
     * @param stackTrace the stack trace of the abort
     * @param reason a descriptive text about the reason for the abort
     */
    static void reportProgramAbort(std::string stackTrace, std::string reason);

    /**
     * Initiates globally the exception catching (signal handlers for Linux)
     */
    static void initGlobalExceptionCatching();
};

}} // namespaces

#endif
