#include "proto.h"

namespace dap
{
DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT( PolInitializeRequest, InitializeRequest, "initialize",
                                   DAP_FIELD( password, "password" ) );

DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT( PolAttachRequest, AttachRequest, "attach",
                                   DAP_FIELD( pid, "pid" ) );

DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT( PolLaunchRequest, LaunchRequest, "launch",
                                   DAP_FIELD( script, "script" ), DAP_FIELD( arg, "arg" ) );

DAP_IMPLEMENT_STRUCT_TYPEINFO( PolProcess, "process", DAP_FIELD( id, "id" ),
                               DAP_FIELD( script, "script" ), DAP_FIELD( state, "state" ) );

DAP_IMPLEMENT_STRUCT_TYPEINFO( PolProcessesResponse, "processes",
                               DAP_FIELD( processes, "processes" ) );

DAP_IMPLEMENT_STRUCT_TYPEINFO( PolProcessesRequest, "processes", DAP_FIELD( filter, "filter" ) );

}  // namespace dap
