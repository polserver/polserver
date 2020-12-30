
#include "server.h"
#include "bscript/compiler/Compiler.h"
#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "clib/esignal.h"
#include "clib/logfacility.h"
#include "clib/threadhelp.h"
#include "nlohmann/json.hpp"
#include "protocol.h"
#include "serializer.h"
#include "types.h"
#include <chrono>
#include <limits>
#include <memory>
#include <string>
#include <vector>

// ANTLR undefines this...
#ifndef EOF
#define EOF ( -1 )
#endif

namespace Pol::ECompile::LSP
{
using namespace LSP::Protocol;
using namespace Bscript;

LspServer lsp_server;

void launchStdin()
{
  std::string str;
  const std::string_view kContentLength( "Content-Length: " );
  bool received_exit = false;
  while ( true )
  {
    int len = 0;
    str.clear();
    while ( true )
    {
      int c = getchar();
      if ( c == EOF )
        goto quit;
      if ( c == '\n' )
      {
        if ( str.empty() )
          break;
        if ( !str.compare( 0, kContentLength.size(), kContentLength ) )
          len = atoi( str.c_str() + kContentLength.size() );
        str.clear();
      }
      else if ( c != '\r' )
      {
        str += c;
      }
    }

    str.resize( len );
    for ( int i = 0; i < len; ++i )
    {
      int c = getchar();
      if ( c == EOF )
        goto quit;
      str[i] = c;
    }

    auto message = std::make_unique<char[]>( len );
    std::copy( str.begin(), str.end(), message.get() );
    try
    {
      auto v = std::make_unique<nlohmann::json>( nlohmann::json::parse( str ) );

      ERROR_PRINT << "Successful parsing: " << v->dump() << "\n";
      RequestId id;
      std::string method;
      if ( v->contains( "id" ) )
      {
        from_json( v->at( "id" ), id );
      }
      if ( v->contains( "method" ) )
      {
        v->at( "method" ).get_to( method );
        lsp_server.on_request.push_move( InMessage{
            id, method, std::move( message ), std::move( v ), std::chrono::steady_clock::now() } );
      }
      else
      {
        // @TODO handle error messages
      }
    }
    catch ( const std::exception& e )
    {
      ERROR_PRINT << "Error parsing: " << e.what() << "\n";
    }
    if ( received_exit )
      break;
  }

quit:
  if ( !received_exit )
  {
    const std::string_view str( "{\"jsonrpc\":\"2.0\",\"method\":\"exit\"}" );
    auto message = std::make_unique<char[]>( str.size() );
    std::copy( str.begin(), str.end(), message.get() );
  }
  ERROR_PRINT << "STDIN thread finished!\n";
}

void launchStdout()
{
  std::string msg;
  while ( !Clib::exit_signalled )
  {
    lsp_server.for_response.pop_wait( &msg );
#ifdef WINDOWS
    INFO_PRINT << "Content-Length: " << msg.size() << "\n\n" << msg;
#else
    INFO_PRINT << "Content-Length: " << msg.size() << "\r\n\r\n" << msg;
#endif
    ERROR_PRINT << "Sending response: " << msg << "\n";
  }
}

#define HANDLE_REQUEST( METHOD, HANDLER, PARAMS )                                      \
  if ( msg.method == METHOD )                                                          \
  {                                                                                    \
    lsp_server.for_response.push_move( nlohmann::json{                                 \
        { "jsonrpc", "2.0" },                                                          \
        { "id", msg.id },                                                              \
        { "result", _handler.HANDLER( msg.document->at( "params" ).get<PARAMS>() ) } } \
                                           .dump() );                                  \
    break;                                                                             \
  }

#define HANDLE_NOTIFICATION( METHOD, HANDLER, PARAMS )              \
  if ( msg.method == METHOD )                                       \
  {                                                                 \
    _handler.HANDLER( msg.document->at( "params" ).get<PARAMS>() ); \
    break;                                                          \
  }


void LspServer::start()
{
  threadhelp::start_thread( launchStdin, "LSP-STDIN" );
  threadhelp::start_thread( launchStdout, "LSP-STDOUT" );

  InMessage msg;
  while ( !Clib::exit_signalled )
  {
    lsp_server.on_request.pop_wait( &msg );
    ERROR_PRINT << "id=" << msg.id.value << ", method=" << msg.method
                << ", document= " << msg.document->dump() << "\n";
    try
    {
      do
      {
        HANDLE_REQUEST( "initialize", onInitialize, InitializeParams )
        HANDLE_NOTIFICATION( "initialized", onInitialized, InitializedParams )
        HANDLE_NOTIFICATION( "textDocument/didOpen", onDidOpenTextDocument,
                             DidOpenTextDocumentParams )
        HANDLE_NOTIFICATION( "textDocument/didChange", onDidChangeTextDocument,
                             DidChangeTextDocumentParams )
        HANDLE_NOTIFICATION( "textDocument/didClose", onDidCloseTextDocument,
                             DidCloseTextDocumentParams )
        HANDLE_NOTIFICATION( "textDocument/didSave", onDidSaveTextDocument,
                             DidSaveTextDocumentParams )
        HANDLE_NOTIFICATION( "textDocument/willSave", onWillSaveTextDocument,
                             WillSaveTextDocumentParams )
        HANDLE_REQUEST( "textDocument/willSaveWaitUntil", onWillSaveTextDocumentWaitUntil,
                        WillSaveTextDocumentParams )

        throw std::runtime_error( "Unknown method " + msg.method );
      } while ( false );
    }
    catch ( const std::exception& ex )
    {
      if ( msg.id.valid() )
      {
        auto response = nlohmann::json{
            { "jsonrpc", "2.0" },
            { "id", msg.id },
            { "method", msg.method },
            { "error",
              nlohmann::json{ { "code", ErrorCodes::InternalError }, { "message", ex.what() } } } };
        lsp_server.for_response.push_move( response.dump() );
        ERROR_PRINT << "error handling request: " << ex.what() << "\n";
      }
      else
      {
        ERROR_PRINT << "error handling notification: " << ex.what() << "\n";
      }
    }
  }
}

InitializeResult MessageHandler::onInitialize( const InitializeParams& params )
{
  InitializeResult result;
  result.serverInfo.emplace(
      InitializeResult::ServerInfoInitializeResult{ "ecompile", "version" } );
  result.capabilities.textDocumentSync = TextDocumentSyncKind::Full;
  return result;
}

void MessageHandler::onInitialized( const InitializedParams& /* params */ )
{
  ERROR_PRINT << "initialized\n";
}


struct Summary
{
  unsigned UpToDateScripts = 0;
  unsigned CompiledScripts = 0;
  unsigned ScriptsWithCompileErrors = 0;
  size_t ThreadCount = 0;
  Compiler::Profile profile;
} summary;

struct Comparison
{
  std::atomic<long long> CompileTimeV1Micros{};
  std::atomic<long long> CompileTimeV2Micros{};
  std::atomic<long> MatchingResult{};
  std::atomic<long> NonMatchingResult{};
  std::atomic<long> MatchingOutput{};
  std::atomic<long> NonMatchingOutput{};
} comparison;

Compiler::SourceFileCache em_parse_tree_cache( summary.profile );
Compiler::SourceFileCache inc_parse_tree_cache( summary.profile );

void MessageHandler::onDidOpenTextDocument( const DidOpenTextDocumentParams& params )
{
  auto path = params.textDocument.uri.getPath();
  if ( params.textDocument.languageId == "escript" )
  {
    std::string contents = params.textDocument.text;
    ERROR_PRINT << "open " << path << "\n";
    em_parse_tree_cache.configure( UINT_MAX );
    inc_parse_tree_cache.configure( UINT_MAX );
    auto compiler = std::make_unique<Bscript::Compiler::Compiler>(
        em_parse_tree_cache, inc_parse_tree_cache, summary.profile );

    Compiler::DiagnosticReport report;

    auto sf = Compiler::SourceFile::load( path, contents, summary.profile, report );

    compiler->compile_file_steps( sf, nullptr, report );

    if ( report.error_count() == 0 )
    {
      auto& identifiers = compiler->source_file_identifiers();
      for ( auto& identifier : identifiers )
      {
        ERROR_PRINT << "source " << identifier->pathname << "#" << identifier->index << "\n";
      }
    }


    ERROR_PRINT << "errors = " << report.error_count() << ", warnings = " << report.warning_count()
                << "\n";

    PublishDiagnosticsParams result;
    result.uri = params.textDocument.uri;
    result.version.emplace( params.textDocument.version );
    result.diagnostics.resize( report.diagnostics.size() );
    std::transform( report.diagnostics.begin(), report.diagnostics.end(),
                    result.diagnostics.begin(), to_lsp );

    lsp_server.for_response.push_move( nlohmann::json{
        { "jsonrpc", "2.0" },
        { "method", "textDocument/publishDiagnostics" },
        { "params", result } }.dump() );
  }
}

void MessageHandler::onDidChangeTextDocument( const DidChangeTextDocumentParams& params )
{
  ERROR_PRINT << "change " << params.textDocument.uri.getPath() << "\n";
}

void MessageHandler::onDidCloseTextDocument( const DidCloseTextDocumentParams& params )
{
  ERROR_PRINT << "close " << params.textDocument.uri.getPath() << "\n";
}

void MessageHandler::onDidSaveTextDocument( const DidSaveTextDocumentParams& params )
{
  ERROR_PRINT << "save " << params.textDocument.uri.getPath() << "\n";
}

void MessageHandler::onWillSaveTextDocument( const WillSaveTextDocumentParams& params )
{
  ERROR_PRINT << "willSave " << params.textDocument.uri.getPath() << "\n";
}

WillSaveTextDocumentResult MessageHandler::onWillSaveTextDocumentWaitUntil(
    const WillSaveTextDocumentParams& params )
{
  ERROR_PRINT << "willSaveWaitUntil " << params.textDocument.uri.getPath() << "\n";
  WillSaveTextDocumentResult result;
  return result;
}

}  // namespace Pol::ECompile::LSP
