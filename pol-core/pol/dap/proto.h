#ifndef DAP_PROTOCOL_H
#define DAP_PROTOCOL_H

#include <dap/protocol.h>

namespace dap
{
// Define a custom `initialize` request that adds optional `password`.
class PolInitializeRequest : public InitializeRequest
{
public:
  optional<string> password;
};

DAP_DECLARE_STRUCT_TYPEINFO( PolInitializeRequest );

// Define a custom `launch` request that adds `pid`.
class PolAttachRequest : public AttachRequest
{
public:
  number pid;
};

DAP_DECLARE_STRUCT_TYPEINFO( PolAttachRequest );

class PolLaunchRequest : public LaunchRequest
{
public:
  string program;
  optional<string> arg;
};

DAP_DECLARE_STRUCT_TYPEINFO( PolLaunchRequest );

// Define objects, response, request for custom `processes` command.
struct PolProcess
{
  number id;
  string program;
  number state; // 0 = sleeping, 1 = running, 2 = debugging
};

DAP_DECLARE_STRUCT_TYPEINFO( PolProcess );

class PolProcessesResponse : public Response
{
public:
  array<PolProcess> processes;
};

DAP_DECLARE_STRUCT_TYPEINFO( PolProcessesResponse );

class PolProcessesRequest : public Request
{
public:
  using Response = PolProcessesResponse;
  optional<string> filter;
};

DAP_DECLARE_STRUCT_TYPEINFO( PolProcessesRequest );

}  // namespace dap

#endif
