#ifndef CLIB_PROGRAM_MAIN_H
#define CLIB_PROGRAM_MAIN_H

#include <string>
#include <vector>


namespace Pol::Clib
{
class ProgramMain
{
public:
  ProgramMain();
  virtual ~ProgramMain();

  void start( int argc, char* argv[] );

protected:
  virtual int main() = 0;
  virtual void showHelp() = 0;

  const std::vector<std::string>& programArgs() const;
  bool programArgsFind( const std::string& filter, std::string* rest = nullptr ) const;
  std::string programArgsFindEquals( const std::string& filter, std::string defaultVal ) const;
  int programArgsFindEquals( const std::string& filter, int defaultVal, bool hexVal ) const;

private:
  std::vector<std::string> m_programArguments;
};
}  // namespace Pol::Clib
   // namespaces

#define PROG_MAIN Pol::Clib::ProgramMain

#endif  // CLIB_PROGRAM_MAIN_H
