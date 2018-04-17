#ifndef CLIB_FILECONT_H
#define CLIB_FILECONT_H

#include <string>
#include "unicode.h"

namespace Pol
{
namespace Clib
{

/**
 * Simple class to read a whole file as a utf8 string
 */
class FileContents
{
public:
  /**
   * Creates the instance and reads the whole file content into it
   *
   * @param filename Full path of the file to be read
   * @throws std::runtime_error When something goes wrong when reading the file
   */
  inline FileContents( const std::string& filename )
    : filename_( filename ), contents_( std::make_shared<UnicodeString>("") )
  {
  }

  const std::shared_ptr<UnicodeString>& contents() const;
  void set_contents( const UnicodeString& str );

protected:
  std::string filename_;
  std::shared_ptr<UnicodeString> contents_;

  virtual void read();
  virtual void reportNotFoundError() const = 0;
};

/**
 * Just like FileContents but with personalized error messages
 */
class SourceFileContents : public FileContents
{
public:
  inline SourceFileContents( const std::string& filename )
    : FileContents( filename )
  {
    read();
  };

protected:
  virtual void reportNotFoundError() const override;
};

/**
 * Just like FileContents but with personalized error messages
 */
class IncludeFileContents : public FileContents
{
public:
  inline IncludeFileContents( const std::string& filename )
    : FileContents( filename )
  {
    read();
  };

protected:
  virtual void reportNotFoundError() const override;
};

/**
 * Just like FileContents but with personalized error messages
 */
class ModuleFileContents : public FileContents
{
public:
  inline ModuleFileContents( const std::string& filename, const std::string& modulename )
    : FileContents( filename ), modulename_( modulename )
  {
    read();
  };

protected:
  std::string modulename_;

  virtual void reportNotFoundError() const override;
};

}
}
#endif
