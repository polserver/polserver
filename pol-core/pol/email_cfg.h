#pragma once

#include <string>

namespace Pol
{
namespace Core
{

struct Email_Cfg
{
  std::string url;
  std::string username;
  std::string password;
  std::string ca_file;
  bool use_tls;
};
}
}
