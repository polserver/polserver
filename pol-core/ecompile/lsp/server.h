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

  /**
   * The document open notification is sent from the client to the server to signal
   * newly opened text documents. The document's truth is now managed by the client
   * and the server must not try to read the document's truth using the document's
   * uri. Open in this sense means it is managed by the client. It doesn't necessarily
   * mean that its content is presented in an editor. An open notification must not
   * be sent more than once without a corresponding close notification send before.
   * This means open and close notification must be balanced and the max open count
   * is one.
   */
  void onDidOpenTextDocument( const Protocol::DidOpenTextDocumentParams& params );

  /**
   * The document change notification is sent from the client to the server to signal
   * changes to a text document.
   */
  void onDidChangeTextDocument( const Protocol::DidChangeTextDocumentParams& params );

  /**
   * The document close notification is sent from the client to the server when
   * the document got closed in the client. The document's truth now exists where
   * the document's uri points to (e.g. if the document's uri is a file uri the
   * truth now exists on disk). As with the open notification the close notification
   * is about managing the document's content. Receiving a close notification
   * doesn't mean that the document was open in an editor before. A close
   * notification requires a previous open notification to be sent.
   */
  void onDidCloseTextDocument( const Protocol::DidCloseTextDocumentParams& params );

  /**
   * The document save notification is sent from the client to the server when
   * the document got saved in the client.
   */
  void onDidSaveTextDocument( const Protocol::DidSaveTextDocumentParams& params );

  /**
   * A document will save notification is sent from the client to the server before
   * the document is actually saved.
   */
  void onWillSaveTextDocument( const Protocol::WillSaveTextDocumentParams& params );

  /**
   * A document will save request is sent from the client to the server before
   * the document is actually saved. The request can return an array of TextEdits
   * which will be applied to the text document before it is saved. Please note that
   * clients might drop results if computing the text edits took too long or if a
   * server constantly fails on this request. This is done to keep the save fast and
   * reliable.
   */
  Protocol::WillSaveTextDocumentResult onWillSaveTextDocumentWaitUntil(
      const Protocol::WillSaveTextDocumentParams& params );
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
