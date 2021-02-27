#ifndef CLIB_FILECONT_H
#define CLIB_FILECONT_H

#include <string>
namespace Pol
{
namespace Clib
{
/**
 * Simple class to read a whole file as a char array
 */
class FileContents
{
public:
  FileContents( const char* filname, bool suppress_error_print = false );

  const char* contents() const;
  void set_contents( const std::string& str );

private:
  std::string contents_;
};
}
}
#endif
