#ifndef LSP_PROTOCOL_H
#define LSP_PROTOCOL_H

#include "types.h"
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Pol::ECompile::LSP::Protocol
{
struct RequestId
{
  // The client can send the request id as an int or a string. We should output
  // the same format we received.
  enum Type
  {
    kNone,
    kInt,
    kString
  };
  Type type = kNone;

  std::string value;

  bool valid() const { return type != kNone; }
};

// @TODO uhh fix this...?
using ProgressToken = std::variant<int, std::string>;

/**
 * A document filter denotes a document by different properties like
 * the [language](#TextDocument.languageId), the [scheme](#Uri.scheme) of
 * its resource, or a glob-pattern that is applied to the [path](#TextDocument.fileName).
 *
 * Glob patterns can have the following syntax:
 * - `*` to match one or more characters in a path segment
 * - `?` to match on one character in a path segment
 * - `**` to match any number of path segments, including none
 * - `{}` to group conditions (e.g. `**​/*.{ts,js}` matches all TypeScript and JavaScript files)
 * - `[]` to declare a range of characters to match in a path segment (e.g., `example.[0-9]` to
 * match on `example.0`, `example.1`, …)
 * - `[!...]` to negate a range of characters to match in a path segment (e.g., `example.[!0-9]` to
 * match on `example.a`, `example.b`, but not `example.0`)
 *
 * @sample A language filter that applies to typescript files on disk: `{ language: 'typescript',
 * scheme: 'file' }`
 * @sample A language filter that applies to all package.json paths: `{ language: 'json', pattern:
 * '**package.json' }`
 */
struct DocumentFilter
{
  /** A language id, like `typescript`. */
  std::optional<std::string> language;
  /** A Uri [scheme](#Uri.scheme), like `file` or `untitled`. */
  std::optional<std::string> scheme;
  /** A glob pattern, like `*.{ts,js}`. */
  std::optional<std::string> pattern;
};

/**
 * A document selector is the combination of one or many document filters.
 *
 * @sample `let sel:DocumentSelector = [{ language: 'typescript' }, { language: 'json', pattern:
 * '**∕tsconfig.json' }]`;
 *
 * The use of a string as a document filter is deprecated @since 3.16.0.
 */
using DocumentSelector = std::vector<std::variant<std::string, DocumentFilter>>;

/**
 * General parameters to to register for an notification or to register a provider.
 */
struct Registration
{
  /**
   * The id used to register the request. The id can be used to deregister
   * the request again.
   */
  std::string id;
  /**
   * The method to register for.
   */
  std::string method;
  /**
   * Options necessary for the registration.
   */
  std::optional<any> registerOptions;
};

struct RegistrationParams
{
  std::vector<Registration> registrations;
};

// /**
//  * The `client/registerCapability` request is sent from the server to the client to register a
//  new capability
//  * handler on the client side.
//  */
// export declare namespace RegistrationRequest {
//     const type: ProtocolRequestType<RegistrationParams, void, never, void, void>;
// }
/**
 * General parameters to unregister a request or notification.
 */
struct Unregistration
{
  /**
   * The id used to unregister the request or notification. Usually an id
   * provided during the register request.
   */
  std::string id;
  /**
   * The method to unregister for.
   */
  std::string method;
};

struct UnregistrationParams
{
  std::vector<Unregistration> unregisterations;
};

// /**
//  * The `client/unregisterCapability` request is sent from the server to the client to unregister
//  a previously registered capability
//  * handler on the client side.
//  */
// export declare namespace UnregistrationRequest {
//     const type: ProtocolRequestType<UnregistrationParams, void, never, void, void>;
// }

struct WorkDoneProgressParams
{
  /**
   * An optional token that a server can use to report work done progress.
   */
  std::optional<ProgressToken> workDoneToken;
};

struct PartialResultParams
{
  /**
   * An optional token that a server can use to report partial results (e.g. streaming) to
   * the client.
   */
  std::optional<ProgressToken> partialResultToken;
};

/**
 * A parameter literal used in requests to pass a text document and a position inside that
 * document.
 */
struct TextDocumentPositionParams
{
  /**
   * The text document.
   */
  TextDocumentIdentifier textDocument;
  /**
   * The position inside the text document.
   */
  Position position;
};

/**
 * The kind of resource operations supported by the client.
 */
enum class ResourceOperationKind
{
  /**
   * Supports creating new files and folders.
   */
  Create,  // 'create'
  /**
   * Supports renaming existing files and folders.
   */
  Rename,  // 'rename'
  /**
   * Supports deleting existing files and folders.
   */
  Delete  // 'delete'
};

enum class FailureHandlingKind
{
  /**
   * Applying the workspace change is simply aborted if one of the changes provided
   * fails. All operations executed before the failing operation stay executed.
   */
  Abort,
  /**
   * All operations are executed transactional. That means they either all
   * succeed or no changes at all are applied to the workspace.
   */
  Transactional,
  /**
   * If the workspace edit contains only textual file changes they are executed transactional.
   * If resource changes (create, rename or delete file) are part of the change the failure
   * handling strategy is abort.
   */
  TextOnlyTransactional,
  /**
   * The client tries to undo the operations already executed. But there is no
   * guarantee that this is succeeding.
   */
  Undo,
};

//
struct ClientCapabilities
{
  ///**
  // * Workspace specific client capabilities.
  // */
  // std::optional<WorkspaceClientCapabilities> workspace;
  ///**
  // * Text document specific client capabilities.
  // */
  // std::optional<TextDocumentClientCapabilities> textDocument;
  ///**
  // * Window specific client capabilities.
  // */
  // std::optional<WindowClientCapabilities> window;
  ///**
  // * General client capabilities.
  // *
  // * @since 3.16.0 - proposed state
  // */
  // std::optional<GeneralClientCapabilities> general;
  ///**
  // * Experimental client capabilities.
  // */
  // std::optional<object> experimental;
};

struct WorkspaceFolder
{
  /**
   * The associated URI for this workspace folder.
   */
  std::string uri;
  /**
   * The name of the workspace folder. Used to refer to this
   * workspace folder in the user interface.
   */
  std::string name;
};

// /**
//  * The initialize request is sent from the client to the server.
//  * It is sent once as the request after starting up the server.
//  * The requests parameter is of type [InitializeParams](#InitializeParams)
//  * the response if of type [InitializeResult](#InitializeResult) of a Thenable that
//  * resolves to such.
//  */
// export declare namespace InitializeRequest {
//     const type: ProtocolRequestType<_InitializeParams & WorkspaceFoldersInitializeParams &
//     WorkDoneProgressParams, InitializeResult<any>, never, InitializeError, void>;
// }
/**
 * The initialize parameters
 */
struct InitializeParams
{
  /**
   * The process Id of the parent process that started
   * the server.
   */
  std::optional<integer> processId;

  struct ClientInfoInitializeParams
  {
    /**
     * The name of the client as defined by the client.
     */
    std::string name;
    /**
     * The client's version as defined by the client.
     */
    std::optional<std::string> version;
  };
  /**
   * Information about the client
   *
   * @since 3.15.0
   */
  std::optional<ClientInfoInitializeParams> clientInfo;
  /**
   * The locale the client is currently showing the user interface
   * in. This must not necessarily be the locale of the operating
   * system.
   *
   * Uses IETF language tags as the value's syntax
   * (See https://en.wikipedia.org/wiki/IETF_language_tag)
   *
   * @since 3.16.0
   */
  std::optional<std::string> locale;
  /**
   * The rootPath of the workspace. Is null
   * if no folder is open.
   *
   * @deprecated in favour of rootUri.
   */
  std::optional<std::string> rootPath;
  /**
   * The rootUri of the workspace. Is null if no
   * folder is open. If both `rootPath` and `rootUri` are set
   * `rootUri` wins.
   *
   * @deprecated in favour of workspaceFolders.
   */
  std::optional<DocumentUri> rootUri;
  /**
   * The capabilities provided by the client (editor or tool)
   */
  ClientCapabilities capabilities;
  /**
   * User provided initialization options.
   */
  std::optional<any> initializationOptions;

  enum class TraceInitializeParams
  {
    INVALID = -1,
    Off,
    Messages,
    Verbose
  };
  /**
   * The initial trace setting. If omitted trace is disabled ('off').
   */
  std::optional<TraceInitializeParams> trace;

  /**
   * The actual configured workspace folders.
   */
  std::optional<std::vector<WorkspaceFolder>> workspaceFolders;
};

/**
 * Defines how the host (editor) should sync
 * document changes to the language server.
 */
enum class TextDocumentSyncKind
{
  INVALID = -1,
  /**
   * Documents should not be synced at all.
   */
  None = 0,
  /**
   * Documents are synced by always sending the full content
   * of the document.
   */
  Full = 1,
  /**
   * Documents are synced by sending the full content on open.
   * After that only incremental updates to the document are
   * send.
   */
  Incremental = 2
};

/**
 * Defines the capabilities provided by a language
 * server.
 */
struct ServerCapabilities
{
  /**
   * Defines how text documents are synced. Is either a detailed structure defining each
   * notification or for backwards compatibility the TextDocumentSyncKind number.
   */
  std::optional<TextDocumentSyncKind> textDocumentSync;
};

/**
 * The result returned from an initialize request.
 */
struct InitializeResult
{
  /**
   * The capabilities the language server provides.
   */
  ServerCapabilities capabilities;

  struct ServerInfoInitializeResult
  {
    /**
     * The name of the server as defined by the server.
     */
    std::string name;
    /**
     * The server's version as defined by the server.
     */
    std::optional<std::string> version;
  };
  /**
   * Information about the server.
   *
   * @since 3.15.0
   */
  std::optional<ServerInfoInitializeResult> serverInfo;
};

struct InitializedParams
{
};

};  // namespace Pol::ECompile::LSP::Protocol

#endif
