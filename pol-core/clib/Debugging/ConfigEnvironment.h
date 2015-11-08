#ifndef CLIB_CONFIG_ENVIRONMENT_H
#define CLIB_CONFIG_ENVIRONMENT_H

#include <string>

namespace Pol{ namespace Clib{

class ConfigEnvironment
{
public:
    ConfigEnvironment();
    virtual ~ConfigEnvironment();

    /**
     * @brief Returns the name of the currently running program
     */
    static std::string programName();

    /**
     * @brief Returns the directory of the currently running program
     */
    static std::string programDir();

    /**
     * @brief Configures the bug reporting system
     */
    static void configureProgramEnvironment(std::string programName);

private:
    static std::string       m_programName;
    static std::string       m_programDir;
};

}} // namespaces

#define CONFIG_ENV            Pol::Clib::ConfigEnvironment

#endif // CLIB_CONFIG_ENVIRONMENT_H
