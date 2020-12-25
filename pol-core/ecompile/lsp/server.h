#ifndef LSP_SERVER_H
#define LSP_SERVER_H

#include "clib/message_queue.h"
#include "nlohmann/json.hpp"
#include "protocol.h"
#include <map>
#include <string>

namespace Pol::ECompile::LSP
{
struct InMessage
{
  Protocol::RequestId id;
  std::string method;
  std::unique_ptr<char[]> message;
  std::unique_ptr<nlohmann::json> document;
  std::chrono::steady_clock::time_point deadline;
  std::string backlog_path;
};

enum class ErrorCodes
{
  // Defined by JSON RPC
  ParseError = -32700,
  InvalidRequest = -32600,
  MethodNotFound = -32601,
  InvalidParams = -32602,
  InternalError = -32603
};

class MessageHandler
{
public:
  /**
   * The initialize request is sent from the client to the server.
   * It is sent once as the request after starting up the server.
   * The requests parameter is of type [InitializeParams](#InitializeParams)
   * the response if of type [InitializeResult](#InitializeResult) of a Thenable that
   * resolves to such.
   */
  Protocol::InitializeResult onInitialize( const Protocol::InitializeParams& params );

  /**
   * The intialized notification is sent from the client to the
   * server after the client is fully initialized and the server
   * is allowed to send requests from the server to the client.
   */
  void onInitialized( const Protocol::InitializedParams& params );
};

class LspServer
{
public:
  void start();
  Clib::message_queue<InMessage> on_request;
  Clib::message_queue<std::string> for_response;

private:
  MessageHandler _handler;
};

extern LspServer lsp_server;

}  // namespace Pol::ECompile::LSP


#endif  // LSP_SERVER_H
