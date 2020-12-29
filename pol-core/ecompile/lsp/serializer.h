#ifndef LSP_SERIALIZER_H
#define LSP_SERIALIZER_H

#include "bscript/compiler/Report.h"
#include "protocol.h"
#include "server.h"
#include "types.h"
#include <nlohmann/json.hpp>
#include <optional>

#define OPTIONAL_JSON_TO( v1 ) optional_emplace_to( nlohmann_json_j, #v1, nlohmann_json_t.v1 );
#define OPTIONAL_JSON_FROM( v1 ) optional_get_to( nlohmann_json_j, #v1, nlohmann_json_t.v1 );

#define OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( Type, ... )                             \
  void to_json( nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t )     \
  {                                                                                \
    NLOHMANN_JSON_EXPAND( NLOHMANN_JSON_PASTE( OPTIONAL_JSON_TO, __VA_ARGS__ ) )   \
  }                                                                                \
  void from_json( const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t )   \
  {                                                                                \
    NLOHMANN_JSON_EXPAND( NLOHMANN_JSON_PASTE( OPTIONAL_JSON_FROM, __VA_ARGS__ ) ) \
  }

#define EMPTY_DEFINE_TYPE_NONINTRUSIVE( Type )                                    \
  void to_json( nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t ) {} \
  void from_json( const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t ) {}

template <typename T>
void optional_get_to( const nlohmann::json& j, const std::string& key, std::optional<T>& where )
{
  // @TODO This does not differentiate between an optionally-present key and a
  // required-but-null-valued key, ie. the difference between std::optional<T> and
  // std::variant<std::nullptr_t, T> respectively.
  if ( j.contains( key ) && !j.at( key ).is_null() )
  {
    where.emplace( j.at( key ).get<T>() );
  }
}

template <typename T>
void optional_get_to( const nlohmann::json& j, const std::string& key, T& where )
{
  j.at( key ).get_to( where );
}

template <typename T>
void optional_emplace_to( nlohmann::json& j, const std::string& key, const std::optional<T>& where )
{
  if ( where.has_value() )
  {
    j.emplace( key, where.value() );
  }
}

template <typename T>
void optional_emplace_to( nlohmann::json& j, const std::string& key, const T& where )
{
  j.emplace( key, where );
}

namespace Pol::ECompile::LSP::Protocol
{
// types.h

void to_json( nlohmann::json& j, const DocumentUri& t )
{
  j = t.raw_uri;
}

void from_json( const nlohmann::json& j, DocumentUri& t )
{
  t.raw_uri = j.get<std::string>();
}

Protocol::Diagnostic to_lsp( const Bscript::Compiler::Diagnostic& t )
{
  Diagnostic d;
  d.message = t.message;
  d.range = Range{ Position{ t.line - 1, t.character }, Position{ t.line - 1, t.character } };
  d.severity = t.severity == Bscript::Compiler::Diagnostic::DiagnosticSeverity::Error
                   ? DiagnosticSeverity::Error
                   : DiagnosticSeverity::Warning;
  d.source.emplace( "escript" );
  return d;
}

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( Position, line, character )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( Range, start, end )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( Location, uri, range )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( LocationLink, originSelectionRange, targetUri, targetRange,
                                   targetSelectionRange )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( Color, red, green, blue, alpha )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( ColorInformation, range, color )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( TextEdit, range, newText )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( ColorPresentation, label, textEdit, additionalTextEdits )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( VersionedTextDocumentIdentifier, version, uri )

NLOHMANN_JSON_SERIALIZE_ENUM( FoldingRangeKind, {
                                                    { FoldingRangeKind::INVALID, nullptr },
                                                    { FoldingRangeKind::Comment, "comments" },
                                                    { FoldingRangeKind::Imports, "imports" },
                                                    { FoldingRangeKind::Region, "region" },
                                                } )


NLOHMANN_JSON_SERIALIZE_ENUM( InitializeParams::TraceInitializeParams,
                              {
                                  { InitializeParams::TraceInitializeParams::INVALID, nullptr },
                                  { InitializeParams::TraceInitializeParams::Off, "off" },
                                  { InitializeParams::TraceInitializeParams::Messages, "messages" },
                                  { InitializeParams::TraceInitializeParams::Verbose, "verbose" },
                              } )


OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( FoldingRange, startLine, startCharacter, endLine, endCharacter,
                                   kind )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( DiagnosticRelatedInformation, location, message )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( CodeDescription, href )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( WorkspaceFolder, uri, name )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( Diagnostic, range, severity, code, codeDescription, source,
                                   message, tags, relatedInformation )  // @TODO data ignored

// protocol.h

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( DocumentFilter, language, scheme, pattern )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( InitializeParams::ClientInfoInitializeParams, name, version )
EMPTY_DEFINE_TYPE_NONINTRUSIVE( ClientCapabilities )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( InitializeParams, capabilities, clientInfo, locale, processId,
                                   rootPath, rootUri, trace, workspaceFolders )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( ServerCapabilities, textDocumentSync )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( InitializeResult::ServerInfoInitializeResult, name, version )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( InitializeResult, capabilities, serverInfo )

EMPTY_DEFINE_TYPE_NONINTRUSIVE( InitializedParams )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( TextDocumentItem, uri, languageId, version, text )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( DidOpenTextDocumentParams, textDocument )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( TextDocumentContentChangeEvent, range, rangeLength, text )
OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( DidChangeTextDocumentParams, textDocument, contentChanges )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( TextDocumentIdentifier, uri )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( DidCloseTextDocumentParams, textDocument )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( DidSaveTextDocumentParams, textDocument, text )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( WillSaveTextDocumentParams, textDocument, reason )

OPTIONAL_DEFINE_TYPE_NONINTRUSIVE( PublishDiagnosticsParams, uri, version, diagnostics )

void to_json( nlohmann::json& j, const RequestId& p )
{
  if ( p.type == RequestId::kString )
  {
    j = p.value;
  }
  else
  {
    j = atoi( p.value.c_str() );
  }
}

void from_json( const nlohmann::json& j, RequestId& p )
{
  if ( j.is_string() )
  {
    j.get_to( p.value );
    p.type = RequestId::kString;
  }
  else if ( j.is_number_integer() )
  {
    int n;
    j.get_to( n );
    p.value = std::to_string( n );
    p.type = RequestId::kInt;
  }
}
}  // namespace Pol::ECompile::LSP::Protocol
#endif
