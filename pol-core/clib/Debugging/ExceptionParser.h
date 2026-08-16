#ifndef CLIB_EXCEPTION_PARSER_H
#define CLIB_EXCEPTION_PARSER_H

#include <string>


namespace Pol::Clib
{
class ExceptionParser
{
public:
  ExceptionParser();
  virtual ~ExceptionParser();

  /**
   * @brief Returns a string containing the current stack trace
   * @return the stack trace as multi-line string
   */
  static std::string getTrace();

  /**
   * @brief Reports a program abort to the program devs
   * @param stackTrace the stack trace of the abort
   * @param reason a descriptive text about the reason for the abort
   */
  static void reportProgramAbort( const std::string& stackTrace, const std::string& reason );

  /**
   * @brief Logs stack traces of all threads to stdout and error output
   */
  static void logAllStackTraces();

  /**
   * @brief Initiates globally the exception catching (signal handlers for Linux)
   */
  static void initGlobalExceptionCatching();

  /**
   * @brief Configures the bug reporting system
   */
  static void configureProgramAbortReportingSystem( bool active, std::string server,
                                                    std::string url, std::string reporter );

  /**
   * @brief Returns true if the bug reporting is active
   */
  static bool programAbortReporting();

  /**
   * @brief Handles exceptions
   */
  static void handleExceptionSignal( int signal );

private:
  static bool m_programAbortReporting;
  static std::string m_programAbortReportingServer;
  static std::string m_programAbortReportingUrl;
  static std::string m_programAbortReportingReporter;
  static std::string m_programStart;
};

// Frames to keep when a stack trace goes into a log report. Nothing caps what
// boost hands back; this only bounds a runaway, and is high enough never to fire
// on an ordinary report.
constexpr size_t MAX_REPORTED_NATIVE_FRAMES = 100;

// Indents every line of `block` by two spaces, keeping the first `max_lines` and
// replacing the rest with a count. Traces are innermost-first, so a cap drops the
// outermost end. Empty in, empty out -- which lets a caller omit its heading.
std::string indent_stack_block( const std::string& block, size_t max_lines );
}  // namespace Pol::Clib
   // namespaces

#endif
