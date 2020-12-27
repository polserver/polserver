#ifndef LSP_TYPES_H
#define LSP_TYPES_H

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Pol::ECompile::LSP::Protocol
{
/** @todo */
using any = void*;
using object = void*;
using unknown = void*;
using empty_object = void*;

/**
 * A tagging type for string properties that are actually document URIs.
 */
using DocumentUri = std::string;
/**
 * A tagging type for string properties that are actually URIs
 *
 * @since 3.16.0 - proposed state
 */
using URI = std::string;
/**
 * Defines an integer in the range of -2^31 to 2^31 - 1.
 */
using integer = int32_t;

/**
 * Defines an unsigned integer in the range of 0 to 2^31 - 1.
 */
using uinteger = uint32_t;

/**
 * Defines a decimal number. Since decimal numbers are very rare in the language
 * server specification we denote the exact range with every decimal using the
 * mathematics interval notations (e.g. [0, 1] denotes all decimals d with 0 <=
 * d <= 1.
 */
using decimal = float;
/**
 * Position in a text document expressed as zero-based line and character
 * offset. The offsets are based on a UTF-16 string representation. So a string
 * of the form `a𐐀b` the character offset of the character `a` is 0, the
 * character offset of `𐐀` is 1 and the character offset of b is 3 since `𐐀`
 * is represented using two code units in UTF-16.
 *
 * Positions are line end character agnostic. So you can not specify a position
 * that denotes `\r|\n` or `\n|` where `|` represents the character offset.
 */
struct Position
{
  /**
   * Line position in a document (zero-based).
   */
  uinteger line;
  /**
   * Character offset on a line in a document (zero-based). Assuming that the
   * line is represented as a string, the `character` value represents the gap
   * between the `character` and `character + 1`.
   *
   * If the character value is greater than the line length it defaults back to
   * the line length.
   */
  uinteger character;
};

/**
 * A range in a text document expressed as (zero-based) start and end positions.
 *
 * If you want to specify a range that contains a line including the line ending
 * character(s) then use an end position denoting the start of the next line.
 * For example:
 * ```ts
 * {
 *     start: { line: 5, character: 23 }
 *     end : { line 6, character : 0 }
 * }
 * ```
 */
struct Range
{
  /**
   * The range's start position
   */
  Position start;
  /**
   * The range's end position.
   */
  Position end;
};

/**
 * Represents a location inside a resource, such as a line inside a text file.
 */
struct Location
{
  DocumentUri uri;
  Range range;
};
/**
 * Represents the connection of two locations. Provides additional metadata over
 * normal [locations](#Location), including an origin range.
 */
struct LocationLink
{
  /**
   * Span of the origin of this link.
   *
   * Used as the underlined span for mouse definition hover. Defaults to the
   * word range at the definition position.
   */
  std::optional<Range> originSelectionRange;
  /**
   * The target resource identifier of this link.
   */
  DocumentUri targetUri;
  /**
   * The full target range of this link. If the target for example is a symbol
   * then target range is the range enclosing this symbol not including
   * leading/trailing whitespace but everything else like comments. This
   * information is typically used to highlight the range in the editor.
   */
  Range targetRange;
  /**
   * The range that should be selected and revealed when this link is being
   * followed, e.g the name of a function. Must be contained by the the
   * `targetRange`. See also `DocumentSymbol#range`
   */
  Range targetSelectionRange;
};

/**
 * Represents a color in RGBA space.
 */
struct Color
{
  /**
   * The red component of this color in the range [0-1].
   */
  decimal red;
  /**
   * The green component of this color in the range [0-1].
   */
  decimal green;
  /**
   * The blue component of this color in the range [0-1].
   */
  decimal blue;
  /**
   * The alpha component of this color in the range [0-1].
   */
  decimal alpha;
};

/**
 * Represents a color range from a document.
 */
struct ColorInformation
{
  /**
   * The range in the document where this color appears.
   */
  Range range;
  /**
   * The actual color value for this color range.
   */
  Color color;
};

/**
 * A text edit applicable to a text document.
 */
struct TextEdit
{
  /**
   * The range of the text document to be manipulated. To insert text into a
   * document create a range where start === end.
   */
  Range range;
  /**
   * The string to be inserted. For delete operations use an empty string.
   */
  std::string newText;
};

struct ColorPresentation
{
  /**
   * The label of this color presentation. It will be shown on the color picker
   * header. By default this is also the text that is inserted when selecting
   * this color presentation.
   */
  std::string label;
  /**
   * An [edit](#TextEdit) which is applied to a document when selecting this
   * presentation for the color.  When `falsy` the
   * [label](#ColorPresentation.label) is used.
   */
  std::optional<TextEdit> textEdit;
  /**
   * An optional array of additional [text edits](#TextEdit) that are applied
   * when selecting this color presentation. Edits must not overlap with the
   * main [edit](#ColorPresentation.textEdit) nor with themselves.
   */
  std::optional<std::vector<TextEdit>> additionalTextEdits;
};

/**
 * Enum of known range kinds
 */
enum class FoldingRangeKind
{
  INVALID,
  /**
   * Folding range for a comment
   */
  Comment,  // = "comment",
  /**
   * Folding range for a imports or includes
   */
  Imports,  // = "imports",
  /**
   * Folding range for a region (e.g. `#region`)
   */
  Region,  // = "region"
};

/**
 * Represents a folding range. To be valid, start and end line must be bigger
 * than zero and smaller than the number of lines in the document. Clients are
 * free to ignore invalid ranges.
 */
struct FoldingRange
{
  /**
   * The zero-based start line of the range to fold. The folded area starts
   * after the line's last character. To be valid, the end must be zero or
   * larger and smaller than the number of lines in the document.
   */
  uinteger startLine;
  /**
   * The zero-based character offset from where the folded range starts. If not
   * defined, defaults to the length of the start line.
   */
  std::optional<uinteger> startCharacter;
  /**
   * The zero-based end line of the range to fold. The folded area ends with the
   * line's last character. To be valid, the end must be zero or larger and
   * smaller than the number of lines in the document.
   */
  uinteger endLine;
  /**
   * The zero-based character offset before the folded range ends. If not
   * defined, defaults to the length of the end line.
   */
  std::optional<uinteger> endCharacter;
  /**
   * Describes the kind of the folding range such as `comment' or 'region'. The
   * kind is used to categorize folding ranges and used by commands like 'Fold
   * all comments'. See [FoldingRangeKind](#FoldingRangeKind) for an enumeration
   * of standardized kinds.
   */
  std::optional<std::string> kind;
};


/**
 * Represents a related message and source code location for a diagnostic. This
 * should be used to point to code locations that cause or related to a
 * diagnostics, e.g when duplicating a symbol in a scope.
 */
struct DiagnosticRelatedInformation
{
  /**
   * The location of this related diagnostic information.
   */
  Location location;
  /**
   * The message of this related diagnostic information.
   */
  std::string message;
};

/**
 * The diagnostic's severity.
 */
enum class DiagnosticSeverity
{
  INVALID,
  /**
   * Reports an error.
   */
  Error = 1,
  /**
   * Reports a warning.
   */
  Warning = 2,
  /**
   * Reports an information.
   */
  Information = 3,
  /**
   * Reports a hint.
   */
  Hint = 4
};

/**
 * The diagnostic tags.
 *
 * @since 3.15.0
 */
enum class DiagnosticTag
{
  INVALID,
  /**
   * Unused or unnecessary code.
   *
   * Clients are allowed to render diagnostics with this tag faded out instead
   * of having an error squiggle.
   */
  Unnecessary = 1,
  /**
   * Deprecated or obsolete code.
   *
   * Clients are allowed to rendered diagnostics with this tag strike through.
   */
  Deprecated = 2
};

/**
 * Structure to capture a description for an error code.
 *
 * @since 3.16.0 - proposed state
 */
struct CodeDescription
{
  /**
   * An URI to open with more information about the diagnostic error.
   */
  URI href;
};

/**
 * Represents a diagnostic, such as a compiler error or warning. Diagnostic
 * objects are only valid in the scope of a resource.
 */
struct Diagnostic
{
  /**
   * The range at which the message applies
   */
  Range range;
  /**
   * The diagnostic's severity. Can be omitted. If omitted it is up to the
   * client to interpret diagnostics as error, warning, info or hint.
   */
  std::optional<DiagnosticSeverity> severity;
  /**
   * The diagnostic's code, which usually appear in the user interface.
   * @TODO only using strings
   */
  std::optional<std::string> code;
  /**
   * An optional property to describe the error code.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<CodeDescription> codeDescription;
  /**
   * A human-readable string describing the source of this diagnostic, e.g.
   * 'typescript' or 'super lint'. It usually appears in the user interface.
   */
  std::optional<std::string> source;
  /**
   * The diagnostic's message. It usually appears in the user interface
   */
  std::string message;
  /**
   * Additional metadata about the diagnostic.
   *
   * @since 3.15.0
   */
  std::optional<std::vector<DiagnosticTag>> tags;
  /**
   * An array of related diagnostic information, e.g. when symbol-names within a
   * scope collide all definitions can be marked via this property.
   */
  std::optional<std::vector<DiagnosticRelatedInformation>> relatedInformation;
  /**
   * A data entry field that is preserved between a
   * `textDocument/publishDiagnostics` notification and
   * `textDocument/codeAction` request.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<unknown> data;
};

/**
 * Represents a reference to a command. Provides a title which will be used to
 * represent a command in the UI and, optionally, an array of arguments which
 * will be passed to the command handler function when invoked.
 */
struct Command
{
  /**
   * Title of the command, like `save`.
   */
  std::string title;
  /**
   * The identifier of the actual command handler.
   */
  std::string command;
  /**
   * Arguments that the command handler should be invoked with.
   */
  std::optional<std::vector<any>> arguments;
};

/**
 * Additional information that describes document changes.
 *
 * @since 3.16.0 - proposed state
 */
struct ChangeAnnotation
{
  /**
   * A human-readable string describing the actual change. The string is
   * rendered prominent in the user interface.
   */
  std::string label;
  /**
   * A flag which indicates that user confirmation is needed before applying the
   * change.
   */
  std::optional<bool> needsConfirmation;
  /**
   * A human-readable string which is rendered less prominent in the user
   * interface.
   */
  std::optional<std::string> description;
};

/**
 * An identifier to refer to a change annotation stored with a workspace edit.
 */
using ChangeAnnotationIdentifier = std::string;
/**
 * A special text edit with an additional change annotation.
 *
 * @since 3.16.0 - proposed state.
 */
struct AnnotatedTextEdit : TextEdit
{
  /**
   * The actual identifier of the change annotation
   */
  ChangeAnnotationIdentifier annotationId;
};

/**
 * A literal to identify a text document in the client.
 */
struct TextDocumentIdentifier
{
  /**
   * The text document's uri.
   */
  DocumentUri uri;
};

/**
 * A text document identifier to optionally denote a specific version of a text
 * document.
 */
struct OptionalVersionedTextDocumentIdentifier : TextDocumentIdentifier
{
  /**
   * The version number of this document. If a versioned text document
   * identifier is sent from the server to the client and the file is not open
   * in the editor (the server has not received an open notification before) the
   * server can send `null` to indicate that the version is unknown and the
   * content on disk is the truth (as specified with document content
   * ownership).
   */
  std::variant<integer, std::nullptr_t> version;
};

/**
 * Describes textual changes on a text document. A TextDocumentEdit describes
 * all changes on a document version Si and after they are applied move the
 * document to version Si+1. So the creator of a TextDocumentEdit doesn't need
 * to sort the array of edits or do any kind of ordering. However the edits must
 * be non overlapping.
 */
struct TextDocumentEdit
{
  /**
   * The text document to change.
   */
  OptionalVersionedTextDocumentIdentifier textDocument;
  /**
   * The edits to be applied.
   *
   * @since 3.16.0 - support for AnnotatedTextEdit. This is guarded using a
   * client capability.
   */
  std::vector<std::variant<TextEdit, AnnotatedTextEdit>> edits;
};

/**
 * A generic resource operation.
 */
struct ResourceOperation
{
  /**
   * The resource operation kind.
   */
  std::string kind;
  /**
   * An optional annotation identifier describing the operation.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<ChangeAnnotationIdentifier> annotationId;
};

/**
 * Options to create a file.
 */
struct CreateFileOptions
{
  /**
   * Overwrite existing file. Overwrite wins over `ignoreIfExists`
   */
  std::optional<bool> overwrite;
  /**
   * Ignore if exists.
   */
  std::optional<bool> ignoreIfExists;
};

/**
 * Create file operation.
 */
struct CreateFile : ResourceOperation
{
  /**
   * A create
   */
  const std::string kind = "create";
  /**
   * The resource to create.
   */
  DocumentUri uri;
  /**
   * Additional options
   */
  std::optional<CreateFileOptions> options;
};

/**
 * Rename file options
 */
struct RenameFileOptions
{
  /**
   * Overwrite target if existing. Overwrite wins over `ignoreIfExists`
   */
  std::optional<bool> overwrite;
  /**
   * Ignores if target exists.
   */
  std::optional<bool> ignoreIfExists;
};
/**
 * Rename file operation
 */
struct RenameFile : ResourceOperation
{
  /**
   * A rename
   */
  const std::string kind = "rename";
  /**
   * The old (existing) location.
   */
  DocumentUri oldUri;
  /**
   * The new location.
   */
  DocumentUri newUri;
  /**
   * Rename options.
   */
  std::optional<RenameFileOptions> options;
};

/**
 * Delete file options
 */
struct DeleteFileOptions
{
  /**
   * Delete the content recursively if a folder is denoted.
   */
  std::optional<bool> recursive;
  /**
   * Ignore the operation if the file doesn't exist.
   */
  std::optional<bool> ignoreIfNotExists;
};
/**
 * Delete file operation
 */
struct DeleteFile : ResourceOperation
{
  /**
   * A delete
   */
  const std::string kind = "delete";
  /**
   * The file to delete.
   */
  DocumentUri uri;
  /**
   * Delete options.
   */
  std::optional<DeleteFileOptions> options;
};

/**
 * A workspace edit represents changes to many resources managed in the
 * workspace. The edit should either provide `changes` or `documentChanges`. If
 * documentChanges are present they are preferred over `changes` if the client
 * can handle versioned document edits.
 */
struct WorkspaceEdit
{
  /**
   * Holds changes to existing resources.
   */
  std::optional<std::map<std::string /*uri*/, std::vector<TextEdit>>> changes;

  /**
   * Depending on the client capability
   * `workspace.workspaceEdit.resourceOperations` document changes are either an
   * array of `TextDocumentEdit`s to express changes to n different text
   * documents where each text document edit addresses a specific version of a
   * text document. Or it can contain above `TextDocumentEdit`s mixed with
   * create, rename and delete file / folder operations.
   *
   * Whether a client supports versioned document edits is expressed via
   * `workspace.workspaceEdit.documentChanges` client capability.
   *
   * If a client neither supports `documentChanges` nor
   * `workspace.workspaceEdit.resourceOperations` then only plain `TextEdit`s
   * using the `changes` property are supported.
   */
  std::optional<std::vector<std::variant<TextDocumentEdit, CreateFile, RenameFile, DeleteFile>>>
      documentChanges;
  /**
   * A map of change annotations that can be referenced in `AnnotatedTextEdit`s
   * or create, rename and delete file / folder operations.
   *
   * Whether clients honor this property depends on the client capability
   * `workspace.changeAnnotationSupport`.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<std::map<std::string /*id*/, ChangeAnnotation>> changeAnnotations;
};

/**
 * A text document identifier to denote a specific version of a text document.
 */
struct VersionedTextDocumentIdentifier : TextDocumentIdentifier
{
  /**
   * The version number of this document.
   */
  integer version;
};


/**
 * An item to transfer a text document from the client to the server.
 */
struct TextDocumentItem
{
  /**
   * The text document's uri.
   */
  DocumentUri uri;
  /**
   * The text document's language identifier
   */
  std::string languageId;
  /**
   * The version number of this document (it will increase after each change,
   * including undo/redo).
   */
  integer version;
  /**
   * The content of the opened text document.
   */
  std::string text;
};

/**
 * Describes the content type that a client supports in various result literals
 * like `Hover`, `ParameterInfo` or `CompletionItem`.
 *
 * Please note that `MarkupKinds` must not start with a `$`. This kinds are
 * reserved for internal usage.
 */
enum class MarkupKind
{
  INVALID,
  /**
   * Plain text is supported as a content format
   */
  PlainText,  //: 'plaintext';
  /**
   * Markdown is supported as a content format
   */
  Markdown,  //: 'markdown';
};

/**
 * A `MarkupContent` literal represents a string value which content is
 * interpreted base on its kind flag. Currently the protocol supports
 * `plaintext` and `markdown` as markup kinds.
 *
 * If the kind is `markdown` then the value can contain fenced code blocks like
 * in GitHub issues. See
 * https://help.github.com/articles/creating-and-highlighting-code-blocks/#syntax-highlighting
 *
 * Here is an example how such a string can be constructed using JavaScript /
 * TypeScript:
 * ```ts
 * let markdown: MarkdownContent = {
 *  kind: MarkupKind.Markdown,
 *  value: [
 *    '# Header',
 *    'Some text',
 *    '```typescript',
 *    'someCode();',
 *    '```'
 *  ].join('\n')
 * };
 * ```
 *
 * *Please Note* that clients might sanitize the return markdown. A client could
 * decide to remove HTML from the markdown to avoid script execution.
 */
struct MarkupContent
{
  /**
   * The type of the Markup
   */
  MarkupKind kind;
  /**
   * The content itself
   */
  std::string value;
};

/**
 * The kind of a completion entry.
 */
enum class CompletionItemKind
{
  INVALID,
  Text = 1,
  Method = 2,
  Function = 3,
  Constructor = 4,
  Field = 5,
  Variable = 6,
  Class = 7,
  Interface = 8,
  Module = 9,
  Property = 10,
  Unit = 11,
  Value = 12,
  Enum = 13,
  Keyword = 14,
  Snippet = 15,
  Color = 16,
  File = 17,
  Reference = 18,
  Folder = 19,
  EnumMember = 20,
  Constant = 21,
  Struct = 22,
  Event = 23,
  Operator = 24,
  TypeParameter = 25
};

/**
 * Defines whether the insert text in a completion item should be interpreted as
 * plain text or a snippet.
 */
enum class InsertTextFormat
{
  INVALID,
  /**
   * The primary text to be inserted is treated as a plain string.
   */
  PlainText = 1,
  /**
   * The primary text to be inserted is treated as a snippet.
   *
   * A snippet can define tab stops and placeholders with `$1`, `$2` and
   * `${3:foo}`. `$0` defines the final tab stop, it defaults to the end of the
   * snippet. Placeholders with equal identifiers are linked, that is typing in
   * one will update others too.
   *
   */
  Snippet = 2
};

/**
 * Completion item tags are extra annotations that tweak the rendering of a
 * completion item.
 *
 * @since 3.15.0
 */
enum class CompletionItemTag
{
  INVALID,
  /**
   * Render a completion as obsolete, usually using a strike-out.
   */
  Deprecated = 1
};

/**
 * A special text edit to provide an insert and a replace operation.
 *
 * @since 3.16.0 - proposed state
 */
struct InsertReplaceEdit
{
  /**
   * The string to be inserted.
   */
  std::string newText;
  /**
   * The range if the insert is requested
   */
  Range insert;
  /**
   * The range if the replace is requested.
   */
  Range replace;
};

/**
 * How whitespace and indentation is handled during completion item insertion.
 *
 * @since 3.16.0 - proposed state
 */
enum class InsertTextMode
{
  INVALID,
  /**
   * The insertion or replace strings is taken as it is. If the value is multi
   * line the lines below the cursor will be inserted using the indentation
   * defined in the string value. The client will not apply any kind of
   * adjustments to the string.
   */
  AsIs = 1,
  /**
   * The editor adjusts leading whitespace of new lines so that they match the
   * indentation up to the cursor of the line for which the item is accepted.
   *
   * Consider a line like this: <2tabs><cursor><3tabs>foo. Accepting a multi
   * line completion item is indented using 2 tabs and all following lines
   * inserted will be indented using 2 tabs as well.
   */
  AdjustIndentation = 2
};

/**
 * A completion item represents a text snippet that is proposed to complete text
 * that is being typed.
 */
struct CompletionItem
{
  /**
   * The label of this completion item. By default also the text that is
   * inserted when selecting this completion.
   */
  std::string label;
  /**
   * The kind of this completion item. Based of the kind an icon is chosen by
   * the editor.
   */
  std::optional<CompletionItemKind> kind;
  /**
   * Tags for this completion item.
   *
   * @since 3.15.0
   */
  std::optional<std::vector<CompletionItemTag>> tags;
  /**
   * A human-readable string with additional information about this item, like
   * type or symbol information.
   */
  std::optional<std::string> detail;
  /**
   * A human-readable string that represents a doc-comment.
   */
  std::optional<std::variant<std::string, MarkupContent>> documentation;
  /**
   * Indicates if this item is deprecated.
   * @deprecated Use `tags` instead.
   */
  std::optional<bool> deprecated;
  /**
   * Select this item when showing.
   *
   * *Note* that only one completion item can be selected and that the tool /
   * client decides which item that is. The rule is that the *first* item of
   * those that match best is selected.
   */
  std::optional<bool> preselect;
  /**
   * A string that should be used when comparing this item with other items.
   * When `falsy` the [label](#CompletionItem.label) is used.
   */
  std::optional<std::string> sortText;
  /**
   * A string that should be used when filtering a set of completion items. When
   * `falsy` the [label](#CompletionItem.label) is used.
   */
  std::optional<std::string> filterText;
  /**
   * A string that should be inserted into a document when selecting this
   * completion. When `falsy` the [label](#CompletionItem.label) is used.
   *
   * The `insertText` is subject to interpretation by the client side. Some
   * tools might not take the string literally. For example VS Code when code
   * complete is requested in this example `con<cursor position>` and a
   * completion item with an `insertText` of `console` is provided it will only
   * insert `sole`. Therefore it is recommended to use `textEdit` instead since
   * it avoids additional client side interpretation.
   */
  std::optional<std::string> insertText;
  /**
   * The format of the insert text. The format applies to both the `insertText`
   * property and the `newText` property of a provided `textEdit`. If omitted
   * defaults to `InsertTextFormat.PlainText`.
   */
  std::optional<InsertTextFormat> insertTextFormat;
  /**
   * How whitespace and indentation is handled during completion item insertion.
   * If ignored the clients default value depends on the
   * `textDocument.completion.insertTextMode` client capability.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<InsertTextMode> insertTextMode;
  /**
   * An [edit](#TextEdit) which is applied to a document when selecting this
   * completion. When an edit is provided the value of
   * [insertText](#CompletionItem.insertText) is ignored.
   *
   * Most editors support two different operation when accepting a completion
   * item. One is to insert a completion text and the other is to replace an
   * existing text with a completion text. Since this can usually not
   * predetermined by a server it can report both ranges. Clients need to signal
   * support for `InsertReplaceEdits` via the
   * `textDocument.completion.insertReplaceSupport` client capability property.
   *
   * *Note 1:* The text edit's range as well as both ranges from a insert
   * replace edit must be a [single line] and they must contain the position at
   * which completion has been requested. *Note 2:* If an `InsertReplaceEdit` is
   * returned the edit's insert range must be a prefix of the edit's replace
   * range, that means it must be contained and starting at the same position.
   *
   * @since 3.16.0 additional type `InsertReplaceEdit` - proposed state
   */
  std::optional<std::variant<TextEdit, InsertReplaceEdit>> textEdit;
  /**
   * An optional array of additional [text edits](#TextEdit) that are applied
   * when selecting this completion. Edits must not overlap (including the same
   * insert position) with the main [edit](#CompletionItem.textEdit) nor with
   * themselves.
   *
   * Additional text edits should be used to change text unrelated to the
   * current cursor position (for example adding an import statement at the top
   * of the file if the completion item will insert an unqualified type).
   */
  std::optional<std::vector<TextEdit>> additionalTextEdits;
  /**
   * An optional set of characters that when pressed while this completion is
   * active will accept it first and then type that character. *Note* that all
   * commit characters should have `length=1` and that superfluous characters
   * will be ignored.
   */
  std::optional<std::vector<std::string>> commitCharacters;
  /**
   * An optional [command](#Command) that is executed *after* inserting this
   * completion. *Note* that additional modifications to the current document
   * should be described with the
   * [additionalTextEdits](#CompletionItem.additionalTextEdits)-property.
   */
  std::optional<Command> command;
  /**
   * A data entry field that is preserved on a completion item between a
   * [CompletionRequest](#CompletionRequest) and a [CompletionResolveRequest]
   * (#CompletionResolveRequest)
   */
  std::optional<any> data;
};

/**
 * Represents a collection of [completion items](#CompletionItem) to be
 * presented in the editor.
 */
struct CompletionList
{
  /**
   * This list it not complete. Further typing results in recomputing this list.
   */
  bool isIncomplete;
  /**
   * The completion items.
   */
  std::vector<CompletionItem> items;
};

struct MarkedStringExt
{
  std::string language;
  std::string value;
};

/**
 * MarkedString can be used to render human readable text. It is either a
 * markdown string or a code-block that provides a language and a code snippet.
 * The language identifier is semantically equal to the optional language
 * identifier in fenced code blocks in GitHub issues. See
 * https://help.github.com/articles/creating-and-highlighting-code-blocks/#syntax-highlighting
 *
 * The pair of a language and a value is an equivalent to markdown:
 * ```${language}
 * ${value}
 * ```
 *
 * Note that markdown strings will be sanitized - that means html will be
 * escaped.
 * @deprecated use MarkupContent instead.
 */
using MarkedString = std::variant<std::string, MarkedStringExt>;

/**
 * The result of a hover request.
 */
struct Hover
{
  /**
   * The hover's content
   */
  std::vector<std::variant<MarkupContent, MarkedString, MarkedString>> contents;
  /**
   * An optional range
   */
  std::optional<Range> range;
};

/**
 * Represents a parameter of a callable-signature. A parameter can have a label
 * and a doc-comment.
 */
struct ParameterInformation
{
  /**
   * The label of this parameter information.
   *
   * Either a string or an inclusive start and exclusive end offsets within its
   * containing signature label. (see SignatureInformation.label). The offsets
   * are based on a UTF-16 string representation as `Position` and `Range` does.
   *
   * *Note*: a label of type string should be a substring of its containing
   * signature label. Its intended use case is to highlight the parameter label
   * part in the `SignatureInformation.label`.
   */
  std::variant<std::string, std::tuple<uinteger, uinteger>> label;
  /**
   * The human-readable doc-comment of this signature. Will be shown in the UI
   * but can be omitted.
   */
  std::optional<std::variant<std::string, MarkupContent>> documentation;
};

/**
 * Represents the signature of something callable. A signature can have a label,
 * like a function-name, a doc-comment, and a set of parameters.
 */
struct SignatureInformation
{
  /**
   * The label of this signature. Will be shown in the UI.
   */
  std::string label;
  /**
   * The human-readable doc-comment of this signature. Will be shown in the UI
   * but can be omitted.
   */
  std::optional<std::variant<std::string, MarkupContent>> documentation;
  /**
   * The parameters of this signature.
   */
  std::optional<std::vector<ParameterInformation>> parameters;
  /**
   * The index of the active parameter.
   *
   * If provided, this is used in place of `SignatureHelp.activeParameter`.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<uinteger> activeParameter;
};

/**
 * Signature help represents the signature of something callable. There can be
 * multiple signature but only one active and only one active parameter.
 */
struct SignatureHelp
{
  /**
   * One or more signatures.
   */
  std::vector<SignatureInformation> signatures;
  /**
   * The active signature. Set to `null` if no signatures exist.
   */
  std::variant<uinteger, std::nullptr_t> activeSignature;
  /**
   * The active parameter of the active signature. Set to `null` if the active
   * signature has no parameters.
   */
  std::variant<uinteger, std::nullptr_t> activeParameter;
};
/**
 * The definition of a symbol represented as one or many [locations](#Location).
 * For most programming languages there is only one location at which a symbol
 * is defined.
 *
 * Servers should prefer returning `DefinitionLink` over `Definition` if
 * supported by the client.
 */
using Definition = std::variant<Location, std::vector<Location>>;
/**
 * Information about where a symbol is defined.
 *
 * Provides additional metadata over normal [location](#Location) definitions,
 * including the range of the defining symbol
 */
using DefinitionLink = LocationLink;
/**
 * The declaration of a symbol representation as one or many
 * [locations](#Location).
 */
using Declaration = std::variant<Location, std::vector<Location>>;
/**
 * Information about where a symbol is declared.
 *
 * Provides additional metadata over normal [location](#Location) declarations,
 * including the range of the declaring symbol.
 *
 * Servers should prefer returning `DeclarationLink` over `Declaration` if
 * supported by the client.
 */
using DeclarationLink = LocationLink;
/**
 * Value-object that contains additional information when requesting references.
 */
struct ReferenceContext
{
  /**
   * Include the declaration of the current symbol.
   */
  bool includeDeclaration;
};
/**
 * A document highlight kind.
 */
enum class DocumentHighlightKind
{
  INVALID,
  /**
   * A textual occurrence.
   */
  Text = 1,
  /**
   * Read-access of a symbol, like reading a variable.
   */
  Read = 2,
  /**
   * Write-access of a symbol, like writing to a variable.
   */
  Write = 3
};
/**
 * A document highlight is a range inside a text document which deserves special
 * attention. Usually a document highlight is visualized by changing the
 * background color of its range.
 */
struct DocumentHighlight
{
  /**
   * The range this highlight applies to.
   */
  Range range;
  /**
   * The highlight kind, default is [text](#DocumentHighlightKind.Text).
   */
  std::optional<DocumentHighlightKind> kind;
};

/**
 * A symbol kind.
 */
enum class SymbolKind
{
  INVALID,
  File = 1,
  Module = 2,
  Namespace = 3,
  Package = 4,
  Class = 5,
  Method = 6,
  Property = 7,
  Field = 8,
  Constructor = 9,
  Enum = 10,
  Interface = 11,
  Function = 12,
  Variable = 13,
  Constant = 14,
  String = 15,
  Number = 16,
  Boolean = 17,
  Array = 18,
  Object = 19,
  Key = 20,
  Null = 21,
  EnumMember = 22,
  Struct = 23,
  Event = 24,
  Operator = 25,
  TypeParameter = 26
};

/**
 * Symbol tags are extra annotations that tweak the rendering of a symbol.
 * @since 3.16
 */
enum class SymbolTag
{
  INVALID,
  /**
   * Render a symbol as obsolete, usually using a strike-out.
   */
  Deprecated = 1
};
/**
 * Represents information about programming constructs like variables, classes,
 * interfaces etc.
 */
struct SymbolInformation
{
  /**
   * The name of this symbol.
   */
  std::string name;
  /**
   * The kind of this symbol.
   */
  SymbolKind kind;
  /**
   * Tags for this completion item.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<std::vector<SymbolTag>> tags;
  /**
   * Indicates if this symbol is deprecated.
   *
   * @deprecated Use tags instead
   */
  std::optional<bool> deprecated;
  /**
   * The location of this symbol. The location's range is used by a tool to
   * reveal the location in the editor. If the symbol is selected in the tool
   * the range's start information is used to position the cursor. So the range
   * usually spans more than the actual symbol's name and does normally include
   * thinks like visibility modifiers.
   *
   * The range doesn't have to denote a node range in the sense of a abstract
   * syntax tree. It can therefore not be used to re-construct a hierarchy of
   * the symbols.
   */
  Location location;
  /**
   * The name of the symbol containing this symbol. This information is for user
   * interface purposes (e.g. to render a qualifier in the user interface if
   * necessary). It can't be used to re-infer a hierarchy for the document
   * symbols.
   */
  std::optional<std::string> containerName;
};

/**
 * Represents programming constructs like variables, classes, interfaces etc.
 * that appear in a document. Document symbols can be hierarchical and they have
 * two ranges: one that encloses its definition and one that points to its most
 * interesting range, e.g. the range of an identifier.
 */
struct DocumentSymbol
{
  /**
   * The name of this symbol. Will be displayed in the user interface and
   * therefore must not be an empty string or a string only consisting of white
   * spaces.
   */
  std::string name;
  /**
   * More detail for this symbol, e.g the signature of a function.
   */
  std::optional<std::string> detail;
  /**
   * The kind of this symbol.
   */
  SymbolKind kind;
  /**
   * Tags for this completion item.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<std::vector<SymbolTag>> tags;
  /**
   * Indicates if this symbol is deprecated.
   *
   * @deprecated Use tags instead
   */
  std::optional<bool> deprecated;
  /**
   * The range enclosing this symbol not including leading/trailing whitespace
   * but everything else like comments. This information is typically used to
   * determine if the the clients cursor is inside the symbol to reveal in the
   * symbol in the UI.
   */
  Range range;
  /**
   * The range that should be selected and revealed when this symbol is being
   * picked, e.g the name of a function. Must be contained by the the `range`.
   */
  Range selectionRange;
  /**
   * Children of this symbol, e.g. properties of a class.
   */
  std::optional<std::vector<DocumentSymbol>> children;
};

/**
 * The kind of a code action.
 *
 * Kinds are a hierarchical list of identifiers separated by `.`, e.g.
 * `"refactor.extract.function"`.
 *
 * The set of kinds is open and client needs to announce the kinds it supports
 * to the server during initialization.
 */
using CodeActionKind = std::string;
/**
 * A set of predefined code action kinds
 */
// export declare namespace CodeActionKind {
//     /**
//      * Empty kind.
//      */
//     const Empty: CodeActionKind;
//     /**
//      * Base kind for quickfix actions: 'quickfix'
//      */
//     const QuickFix: CodeActionKind;
//     /**
//      * Base kind for refactoring actions: 'refactor'
//      */
//     const Refactor: CodeActionKind;
//     /**
//      * Base kind for refactoring extraction actions: 'refactor.extract'
//      *
//      * Example extract actions:
//      *
//      * - Extract method
//      * - Extract function
//      * - Extract variable
//      * - Extract interface from class
//      * - ...
//      */
//     const RefactorExtract: CodeActionKind;
//     /**
//      * Base kind for refactoring inline actions: 'refactor.inline'
//      *
//      * Example inline actions:
//      *
//      * - Inline function
//      * - Inline variable
//      * - Inline constant
//      * - ...
//      */
//     const RefactorInline: CodeActionKind;
//     /**
//      * Base kind for refactoring rewrite actions: 'refactor.rewrite'
//      *
//      * Example rewrite actions:
//      *
//      * - Convert JavaScript function to class
//      * - Add or remove parameter
//      * - Encapsulate field
//      * - Make method static
//      * - Move method to base class
//      * - ...
//      */
//     const RefactorRewrite: CodeActionKind;
//     /**
//      * Base kind for source actions: `source`
//      *
//      * Source code actions apply to the entire file.
//      */
//     const Source: CodeActionKind;
//     /**
//      * Base kind for an organize imports source action: `source.organizeImports`
//      */
//     const SourceOrganizeImports: CodeActionKind;
//     /**
//      * Base kind for auto-fix source actions: `source.fixAll`.
//      *
//      * Fix all actions automatically fix errors that have a clear fix that do not require user
//      input.
//      * They should not suppress errors or perform unsafe fixes such as generating new types or
//      classes.
//      *
//      * @since 3.15.0
//      */
//     const SourceFixAll: CodeActionKind;
// }
/**
 * Contains additional diagnostic information about the context in which a [code
 * action](#CodeActionProvider.provideCodeActions) is run.
 */
struct CodeActionContext
{
  /**
   * An array of diagnostics known on the client side overlapping the range
   * provided to the `textDocument/codeAction` request. They are provided so
   * that the server knows which errors are currently presented to the user for
   * the given range. There is no guarantee that these accurately reflect the
   * error state of the resource. The primary parameter to compute code actions
   * is the provided range.
   */
  std::vector<Diagnostic> diagnostics;
  /**
   * Requested kind of actions to return.
   *
   * Actions not of this kind are filtered out by the client before being shown.
   * So servers can omit computing them.
   */
  std::optional<std::vector<CodeActionKind>> only;
};

/**
 * A code action represents a change that can be performed in code, e.g. to fix
 * a problem or to refactor code.
 *
 * A CodeAction must set either `edit` and/or a `command`. If both are supplied,
 * the `edit` is applied first, then the `command` is executed.
 */
struct CodeAction
{
  /**
   * A short, human-readable, title for this code action.
   */
  std::string title;
  /**
   * The kind of the code action.
   *
   * Used to filter code actions.
   */
  std::optional<CodeActionKind> kind;
  /**
   * The diagnostics that this code action resolves.
   */
  std::optional<std::vector<Diagnostic>> diagnostics;
  /**
   * Marks this as a preferred action. Preferred actions are used by the `auto
   * fix` command and can be targeted by keybindings.
   *
   * A quick fix should be marked preferred if it properly addresses the
   * underlying error. A refactoring should be marked preferred if it is the
   * most reasonable choice of actions to take.
   *
   * @since 3.15.0
   */
  std::optional<bool> isPreferred;

  struct DisabledCodeAction
  {
    /**
     * Human readable description of why the code action is currently disabled.
     *
     * This is displayed in the code actions UI.
     */
    std::string reason;
  };
  /**
   * Marks that the code action cannot currently be applied.
   *
   * Clients should follow the following guidelines regarding disabled code
   * actions:
   *
   *   - Disabled code actions are not shown in automatic
   *     [lightbulb](https://code.visualstudio.com/docs/editor/editingevolved#_code-action)
   *     code action menu.
   *
   *   - Disabled actions are shown as faded out in the code action menu when
   *     the user request a more specific type of code action, such as
   *     refactorings.
   *
   *   - If the user has a keybinding that auto applies a code action and only a
   *     disabled code actions are returned, the client should show the user an
   *     error message with `reason` in the editor.
   *
   * @since 3.16.0
   */
  std::optional<DisabledCodeAction> disabled;
  /**
   * The workspace edit this code action performs.
   */
  std::optional<WorkspaceEdit> edit;
  /**
   * A command this code action executes. If a code action provides a edit and a
   * command, first the edit is executed and then the command.
   */
  std::optional<Command> command;
  /**
   * A data entry field that is preserved on a code action between a
   * `textDocument/codeAction` and a `codeAction/resolve` request.
   *
   * @since 3.16.0 - proposed state
   */
  std::optional<unknown> data;
};

/**
 * A code lens represents a [command](#Command) that should be shown along with
 * source text, like the number of references, a way to run tests, etc.
 *
 * A code lens is _unresolved_ when no command is associated to it. For
 * performance reasons the creation of a code lens and resolving should be done
 * to two stages.
 */
struct CodeLens
{
  /**
   * The range in which this code lens is valid. Should only span a single line.
   */
  Range range;
  /**
   * The command this code lens represents.
   */
  std::optional<Command> command;
  /**
   * A data entry field that is preserved on a code lens item between a
   * [CodeLensRequest](#CodeLensRequest) and a [CodeLensResolveRequest]
   * (#CodeLensResolveRequest)
   */
  std::optional<any> data;
};

/**
 * Value-object describing what options formatting should use.
 */
struct FormattingOptions
{
  /**
   * Size of a tab in spaces.
   */
  uinteger tabSize;
  /**
   * Prefer spaces over tabs.
   */
  bool insertSpaces;
  /**
   * Trim trailing whitespaces on a line.
   *
   * @since 3.15.0
   */
  std::optional<bool> trimTrailingWhitespace;
  /**
   * Insert a newline character at the end of the file if one does not exist.
   *
   * @since 3.15.0
   */
  std::optional<bool> insertFinalNewline;
  /**
   * Trim all newlines after the final newline at the end of the file.
   *
   * @since 3.15.0
   */
  std::optional<bool> trimFinalNewlines;
  /**
   * @todo
   * Signature for further properties.
   */
  // [key: string]: boolean | integer | string | undefined;
};

/**
 * A document link is a range in a text document that links to an internal or
 * external resource, like another text document or a web site.
 */
struct DocumentLink
{
  /**
   * The range this link applies to.
   */
  Range range;
  /**
   * The uri this link points to.
   */
  std::optional<std::string> target;
  /**
   * The tooltip text when you hover over this link.
   *
   * If a tooltip is provided, is will be displayed in a string that includes
   * instructions on how to trigger the link, such as `{0} (ctrl + click)`. The
   * specific instructions vary depending on OS, user settings, and
   * localization.
   *
   * @since 3.15.0
   */
  std::optional<std::string> tooltip;
  /**
   * A data entry field that is preserved on a document link between a
   * DocumentLinkRequest and a DocumentLinkResolveRequest.
   */
  std::optional<any> data;
};

/**
 * A selection range represents a part of a selection hierarchy. A selection
 * range may have a parent selection range that contains it.
 */
struct SelectionRange
{
  /**
   * The [range](#Range) of this selection range.
   */
  Range range;
  /**
   * @todo The parent selection range containing this range. Therefore
   * `parent.range` must contain `this.range`.
   */
  // std::optional<SelectionRange> parent;
};

/**
 * Represents programming constructs like functions or constructors in the
 * context of call hierarchy.
 *
 * @since 3.16.0
 */
struct CallHierarchyItem
{
  /**
   * The name of this item.
   */
  std::string name;
  /**
   * The kind of this item.
   */
  SymbolKind kind;
  /**
   * Tags for this item.
   */
  std::optional<std::vector<SymbolTag>> tags;
  /**
   * More detail for this item, e.g. the signature of a function.
   */
  std::optional<std::string> detail;
  /**
   * The resource identifier of this item.
   */
  DocumentUri uri;
  /**
   * The range enclosing this symbol not including leading/trailing whitespace
   * but everything else, e.g. comments and code.
   */
  Range range;
  /**
   * The range that should be selected and revealed when this symbol is being
   * picked, e.g. the name of a function. Must be contained by the
   * [`range`](#CallHierarchyItem.range).
   */
  Range selectionRange;
  /**
   * A data entry field that is preserved between a call hierarchy prepare and
   * incoming calls or outgoing calls requests.
   */
  std::optional<unknown> data;
};
/**
 * Represents an incoming call, e.g. a caller of a method or constructor.
 *
 * @since 3.16.0
 */
struct CallHierarchyIncomingCall
{
  /**
   * The item that makes the call.
   */
  CallHierarchyItem from;
  /**
   * The ranges at which the calls appear. This is relative to the caller
   * denoted by [`this.from`](#CallHierarchyIncomingCall.from).
   */
  std::vector<Range> fromRanges;
};
/**
 * Represents an outgoing call, e.g. calling a getter from a method or a method
 * from a constructor etc.
 *
 * @since 3.16.0
 */
struct CallHierarchyOutgoingCall
{
  /**
   * The item that is called.
   */
  CallHierarchyItem to;
  /**
   * The range at which this item is called. This is the range relative to the
   * caller, e.g the item passed to `provideCallHierarchyOutgoingCalls` and not
   * [`this.to`](#CallHierarchyOutgoingCall.to).
   */
  std::vector<Range> fromRanges;
};
/**
 * A simple text document. Not to be implemented. The document keeps the content
 * as string.
 *
 * @deprecated Use the text document from the new
 * vscode-languageserver-textdocument package.
 */
struct TextDocument
{
  /**
   * The associated URI for this document. Most documents have the
   * __file__-scheme, indicating that they represent files on disk. However,
   * some documents may have other schemes indicating that they are not
   * available on disk.
   *
   * @readonly
   */
  DocumentUri uri;
  /**
   * The identifier of the language associated with this document.
   *
   * @readonly
   */
  std::string languageId;
  /**
   * The version number of this document (it will increase after each change,
   * including undo/redo).
   *
   * @readonly
   */
  integer version;
};

};  // namespace Pol::ECompile::LSP::Protocol

#endif
