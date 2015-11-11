#ifndef CLIB_PROGRAM_MAIN_H
#define CLIB_PROGRAM_MAIN_H

#include <string>
#include <vector>

namespace Pol{ namespace Clib{

class ProgramMain
{
public:
    ProgramMain();
    virtual ~ProgramMain();

    void start(int argc, char *argv[]);

protected:
    virtual int main() = 0;
    virtual void showHelp() = 0;

    const std::vector<std::string>& programArgs();
    std::string programArgsFind(std::string filter);

private:
    std::vector<std::string>    m_programArguments;
};

}} // namespaces

#define PROG_MAIN           Pol::Clib::ProgramMain

#endif // CLIB_PROGRAM_MAIN_H
