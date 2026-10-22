/** @file
 *
 * @par History
 */

#include <algorithm>
#include <string>

#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/MemberAssignment.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compiler/model/UserFunctionType.h"
#include "bscript/compiler/model/Variable.h"
#include "pol/testing/testcompileranalysis.h"
#include "pol/testing/testenv.h"

// Compiler::analyze() is what vscode-escript calls for diagnostics, hover, completion and the
// rest; nothing in ecompile or pol reaches it. These cases pin the behaviour that only analysis
// mode has: it keeps going after errors that stop ecompile, and it builds the scope tree.

namespace Pol::Testing
{
using namespace Bscript::Compiler;
using namespace CompilerAnalysis;

namespace
{
void test_clean_source()
{
  auto a = analyze_src( "clean.src",
                        "program p()\n"
                        "  var x := 1;\n"
                        "  print( x );\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->workspace != nullptr; }, true, "clean source builds a workspace" );
  UnitTest( [&]() { return a->reporter.diagnostics.size(); }, 0u,
            "clean source has no diagnostics" );
  UnitTest( [&]() { return a->workspace && a->workspace->scope_tree != nullptr; }, true,
            "analysis builds the scope tree" );
  UnitTest( [&]() { return a->workspace && !a->workspace->tokens.empty(); }, true,
            "analysis fills the semantic tokens" );
}

void test_module()
{
  auto a = std::make_unique<Analysis>();
  a->run( a->add_source( "testmod.em",
                         "Foo( a, b := 0 );\n"
                         "Bar();\n"
                         "const BAZ := 1;\n" ),
          true /* is_module */, true /* continue_on_error */, false /* include_all_functions */ );
  UnitTest( [&]() { return a->workspace != nullptr; }, true, "module builds a workspace" );
  UnitTest( [&]() { return a->reporter.diagnostics.size(); }, 0u, "module has no diagnostics" );
  UnitTest( [&]() { return a->workspace ? a->workspace->module_function_declarations.size() : 0u; },
            2u, "module declares its two functions" );
}

void test_module_unreadable()
{
  auto a = std::make_unique<Analysis>();
  a->run( memdir + "nosuchmodule.em", true /* is_module */, true /* continue_on_error */,
          false /* include_all_functions */ );
  UnitTest( [&]() { return a->workspace == nullptr; }, true, "missing module file builds nothing" );
  UnitTest( [&]() { return a->has_message( "Unable to load" ); }, true,
            "missing module file is reported" );
}

// A parse error leaves the referenced user functions unbuilt unless analysis is asked to go on.
void test_continue_on_error()
{
  const std::string body =
      "program p()\n"
      "  var x := ;\n"
      "  helper();\n"
      "endprogram\n"
      "function helper()\n"
      "  return 1;\n"
      "endfunction\n";
  auto stop = analyze_src( "stop.src", body, false /* continue_on_error */,
                           false /* include_all_functions */ );
  auto go_on = analyze_src( "goon.src", body, true /* continue_on_error */,
                            false /* include_all_functions */ );
  UnitTest( [&]() { return stop->report.error_count() > 0; }, true, "parse error is reported" );
  UnitTest( [&]() { return stop->workspace ? stop->workspace->user_functions.size() : 99u; }, 0u,
            "without continue_on_error no user function is built" );
  UnitTest( [&]() { return go_on->workspace ? go_on->workspace->user_functions.size() : 99u; }, 1u,
            "with continue_on_error the user function is built" );
}

// ecompile stops at these (escript tests errors012/errors013); analysis reports them and goes on
// to report the rest of the file.
void test_missing_module_continues()
{
  auto a = analyze_src( "missmod.src",
                        "use nosuchmodule;\n"
                        "program p()\n"
                        "  print( NoSuchFunction() );\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->workspace != nullptr; }, true, "missing module: workspace built" );
  UnitTest( [&]() { return a->has_message( "Unable to use module 'nosuchmodule'." ); }, true,
            "missing module is reported" );
  UnitTest( [&]() { return a->has_message( "NoSuchFunction" ); }, true,
            "missing module: the rest of the file is still analyzed" );
}

void test_missing_include_continues()
{
  auto a = analyze_src( "missinc.src",
                        "include \":nosuchpkg:nosuchinclude\";\n"
                        "program p()\n"
                        "  print( NO_SUCH_CONSTANT );\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->workspace != nullptr; }, true, "missing include: workspace built" );
  UnitTest( [&]() { return a->has_message( "Unable to read include file" ); }, true,
            "missing include is reported" );
  UnitTest( [&]() { return a->has_message( "NO_SUCH_CONSTANT" ); }, true,
            "missing include: the rest of the file is still analyzed" );
}

void test_duplicate_method_keeps_first()
{
  auto a = analyze_src( "dupmethod.src",
                        "class C()\n"
                        "  function C( this )\n"
                        "  endfunction\n"
                        "  function m( this )\n"
                        "    return 1;\n"
                        "  endfunction\n"
                        "  function m( this )\n"
                        "    return 2;\n"
                        "  endfunction\n"
                        "endclass\n"
                        "program p()\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->report.error_count() > 0; }, true, "duplicate method is an error" );
  // Only the link keeps the first declaration. The resolver keeps the last parse tree under a
  // name, so the UserFunction behind the link is built from the second one.
  UnitTest(
      [&]() -> size_t
      {
        if ( !a->workspace || !a->workspace->scope_tree )
          return 0;
        auto cd = a->workspace->scope_tree->find_class( "C" );
        if ( !cd || !cd->methods.contains( "m" ) )
          return 0;
        return cd->methods.at( "m" )->source_location.range.start.line_number;
      },
      size_t{ 4 }, "duplicate method's link points at the first declaration" );
}

void test_case_group_without_label()
{
  auto a = analyze_src( "caselabel.src",
                        "program p()\n"
                        "  var elem;\n"
                        "  case ( elem ) 5: x\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->report.error_count() > 0; }, true,
            "half-typed case is analyzed without a crash" );
}

void test_report_clear()
{
  auto a = analyze_src( "clear.src", "program p(\n" );
  UnitTest( [&]() { return a->report.error_count() > 0; }, true, "broken source has errors" );
  a->report.clear();
  UnitTest( [&]() { return a->report.error_count(); }, 0u, "Report::clear() resets the count" );
  UnitTest( [&]() { return a->reporter.diagnostics.size(); }, size_t{ 0 },
            "Report::clear() empties the DiagnosticReporter" );
}

// One script with every kind of declaration the scope tree answers for. The line numbers matter:
// the cases below ask about positions in it.
const char* const scope_fixture =
    "use uo;\n"                                                                     // 1
    "const TOP_CONST := 1;\n"                                                       // 2
    "enum Colors\n"                                                                 // 3
    "  RED, GREEN\n"                                                                // 4
    "endenum\n"                                                                     // 5
    "enum class Fruit\n"                                                            // 6
    "  APPLE, BANANA\n"                                                             // 7
    "endenum\n"                                                                     // 8
    "var global_var := 1;\n"                                                        // 9
    "program prog( prog_param )\n"                                                  // 10
    "  var outer := 1;\n"                                                           // 11
    "  if ( outer )\n"                                                              // 12
    "    var inner := 2;\n"                                                         // 13
    "    print( inner );\n"                                                         // 14
    "  endif\n"                                                                     // 15
    "  foreach item in ( array{ 1 } )\n"                                            // 16
    "    print( item + _item_iter );\n"                                             // 17
    "  endforeach\n"                                                                // 18
    "  for i := 1 to 3\n"                                                           // 19
    "    print( i );\n"                                                             // 20
    "  endfor\n"                                                                    // 21
    "  var fexpr := @( fe_param ) { return fe_param; };\n"                          // 22
    "  var [ seq_a, seq_b ] := array{ 1, 2 };\n"                                    // 23
    "  var { idx_a } := struct{ idx_a := 1 };\n"                                    // 24
    "  print( helper( prog_param ) + seq_a + seq_b + idx_a + fexpr.call( 1 ) );\n"  // 25
    "  print( Fruit::APPLE + RED + TOP_CONST + global_var );\n"                     // 26
    "  print( Dog().fetch() + Animal::create() );\n"                                // 27
    "endprogram\n"                                                                  // 28
    "function helper( p_nodefault, p_default := 5 )\n"                              // 29
    "  var local_in_helper := p_nodefault;\n"                                       // 30
    "  return local_in_helper + p_default;\n"                                       // 31
    "endfunction\n"                                                                 // 32
    "class Animal()\n"                                                              // 33
    "  var class_var := 1;\n"                                                       // 34
    "  function Animal( this, name )\n"                                             // 35
    "    this.name := name;\n"                                                      // 36
    "    var captured := @() { this.captured_member := 1; };\n"                     // 37
    "    var shadowed := @( this ) { this.shadowed_member := 1; };\n"               // 38
    "  endfunction\n"                                                               // 39
    "  function speak( this )\n"                                                    // 40
    "    return this.name;\n"                                                       // 41
    "  endfunction\n"                                                               // 42
    "  function create()\n"                                                         // 43
    "    return Animal( \"x\" );\n"                                                 // 44
    "  endfunction\n"                                                               // 45
    "endclass\n"                                                                    // 46
    "class Dog( Animal )\n"                                                         // 47
    "  function Dog( this )\n"                                                      // 48
    "    super( \"dog\" );\n"                                                       // 49
    "    this.tricks := 0;\n"                                                       // 50
    "  endfunction\n"                                                               // 51
    "  function fetch( this )\n"                                                    // 52
    "    return this.speak() + Animal::class_var;\n"                                // 53
    "  endfunction\n"                                                               // 54
    "endclass\n";                                                                   // 55

Position at( size_t line, size_t column )
{
  return Position{ line, column, 0 };
}

ScopeTreeQuery query( std::string calling_scope, std::string current_user_function,
                      ScopeName prefix_scope, std::string prefix )
{
  return ScopeTreeQuery{ std::move( calling_scope ), std::move( current_user_function ),
                         std::move( prefix_scope ), std::move( prefix ) };
}

// Sorted and comma-joined, so a list compares as one string and a failure prints all of it.
template <typename Container, typename NameOf>
std::string names( const Container& items, NameOf name_of )
{
  std::vector<std::string> result;
  for ( const auto& item : items )
    result.push_back( name_of( *item ) );
  std::ranges::sort( result );
  std::string joined;
  for ( const auto& name : result )
    joined += ( joined.empty() ? "" : "," ) + name;
  return joined;
}

std::string name_of_variable( const Variable& v )
{
  return v.name;
}
std::string name_of_function( const Function& f )
{
  return f.scoped_name();
}
std::string name_of_constant( const ConstDeclaration& c )
{
  return c.name.string();
}
std::string name_of_member( const MemberAssignment& m )
{
  return m.name;
}

void scope_variable_test( const ScopeTree& tree )
{
  auto found_line = [&]( const std::string& name, Position pos ) -> size_t
  {
    auto variable = tree.find_variable( name, pos );
    return variable ? variable->source_location.range.start.line_number : 0;
  };
  UnitTest( [&]() { return found_line( "outer", at( 14, 5 ) ); }, size_t{ 11 },
            "find_variable: program local from a nested block" );
  UnitTest( [&]() { return found_line( "inner", at( 14, 5 ) ); }, size_t{ 13 },
            "find_variable: block local inside its block" );
  UnitTest( [&]() { return found_line( "inner", at( 16, 3 ) ); }, size_t{ 0 },
            "find_variable: block local not after its block" );
  UnitTest( [&]() { return found_line( "item", at( 17, 5 ) ) > 0; }, true,
            "find_variable: foreach variable inside the loop" );
  UnitTest( [&]() { return found_line( "_item_iter", at( 17, 5 ) ) > 0; }, true,
            "find_variable: foreach iterator inside the loop" );
  UnitTest( [&]() { return found_line( "item", at( 20, 5 ) ); }, size_t{ 0 },
            "find_variable: foreach variable not after the loop" );
  UnitTest( [&]() { return found_line( "i", at( 20, 5 ) ) > 0; }, true,
            "find_variable: for variable inside the loop" );
  UnitTest( [&]() { return found_line( "prog_param", at( 11, 3 ) ); }, size_t{ 10 },
            "find_variable: program parameter" );
  UnitTest( [&]() { return found_line( "fe_param", at( 22, 34 ) ) > 0; }, true,
            "find_variable: function expression parameter inside its body" );
  UnitTest( [&]() { return found_line( "fe_param", at( 23, 3 ) ); }, size_t{ 0 },
            "find_variable: function expression parameter not outside it" );
  UnitTest( [&]() { return found_line( "seq_b", at( 25, 3 ) ); }, size_t{ 23 },
            "find_variable: sequence binding" );
  UnitTest( [&]() { return found_line( "idx_a", at( 25, 3 ) ); }, size_t{ 24 },
            "find_variable: index binding" );
  UnitTest( [&]() { return found_line( "p_default", at( 31, 3 ) ); }, size_t{ 29 },
            "find_variable: function parameter with a default" );
  UnitTest( [&]() { return found_line( "p_nodefault", at( 31, 3 ) ); }, size_t{ 29 },
            "find_variable: function parameter without a default" );
  UnitTest( [&]() { return found_line( "outer", at( 31, 3 ) ); }, size_t{ 0 },
            "find_variable: another function's local is not visible" );
  UnitTest( [&]() { return found_line( "global_var", at( 31, 3 ) ); }, size_t{ 9 },
            "find_variable: global from a function" );

  // The globals are added whatever the prefix. The editor filters completions by the typed word.
  UnitTest(
      [&]()
      {
        return names( tree.list_variables( query( "", "", ScopeName::None, "o" ), at( 14, 5 ) ),
                      name_of_variable );
      },
      std::string( "global_var,outer" ), "list_variables: prefix in a nested block" );
  UnitTest(
      [&]()
      {
        return names( tree.list_variables( query( "", "", ScopeName::None, "Ou" ), at( 14, 5 ) ),
                      name_of_variable );
      },
      std::string( "global_var,outer" ), "list_variables: prefix is case-insensitive" );
  UnitTest(
      [&]()
      {
        return names( tree.list_variables( query( "", "", ScopeName::None, "" ), at( 31, 3 ) ),
                      name_of_variable );
      },
      std::string( "global_var,local_in_helper,p_default,p_nodefault" ),
      "list_variables: a function's locals and the globals" );
  UnitTest(
      [&]()
      {
        return names(
            tree.list_variables( query( "", "", ScopeName( "Animal" ), "" ), at( 14, 5 ) ),
            name_of_variable );
      },
      std::string( "Animal::class_var" ),
      "list_variables: class scope gives class variables only" );
  UnitTest(
      [&]()
      {
        return names(
            tree.list_variables( query( "Animal", "", ScopeName::None, "" ), at( 41, 5 ) ),
            name_of_variable );
      },
      std::string( "Animal::class_var,global_var,this" ),
      "list_variables: inside a method, its class variables too" );
}

void scope_function_test( const ScopeTree& tree )
{
  auto user_function = [&]( ScopeTreeQuery q ) -> std::string
  {
    auto fn = tree.find_user_function( q );
    return fn ? fn->scoped_name() : "";
  };
  UnitTest( [&]() { return user_function( query( "", "", ScopeName::None, "HELPER" ) ); },
            std::string( "helper" ), "find_user_function: global, case-insensitive" );
  UnitTest( [&]() { return user_function( query( "", "", ScopeName::Global, "helper" ) ); },
            std::string( "helper" ), "find_user_function: explicitly global" );
  UnitTest( [&]() { return user_function( query( "Animal", "", ScopeName::None, "speak" ) ); },
            std::string( "Animal::speak" ), "find_user_function: method from inside its class" );
  UnitTest( [&]() { return user_function( query( "", "", ScopeName( "Animal" ), "create" ) ); },
            std::string( "Animal::create" ), "find_user_function: scoped static function" );
  UnitTest( [&]() { return user_function( query( "", "", ScopeName::None, "speak" ) ); },
            std::string( "" ), "find_user_function: method not found from outside its class" );
  UnitTest( [&]() { return user_function( query( "Dog", "Dog", ScopeName::Super, "fetch" ) ); },
            std::string( "" ), "find_user_function: super:: does not find the class's own method" );
  UnitTest( [&]() { return user_function( query( "Cat", "x", ScopeName::Super, "speak" ) ); },
            std::string( "" ), "find_user_function: super:: from an unknown class" );
  UnitTest( [&]() { return user_function( query( "Dog", "Dog", ScopeName::Super, "speak" ) ); },
            std::string( "Animal::speak" ), "find_user_function: super:: finds the base method" );

  UnitTest(
      [&]()
      {
        return names(
            tree.list_user_functions( query( "", "", ScopeName::None, "he" ), at( 25, 3 ) ),
            name_of_function );
      },
      std::string( "helper" ), "list_user_functions: global prefix" );
  UnitTest(
      [&]()
      {
        return names(
            tree.list_user_functions( query( "", "", ScopeName( "Animal" ), "" ), at( 25, 3 ) ),
            name_of_function );
      },
      std::string( "Animal::Animal,Animal::create,Animal::speak" ),
      "list_user_functions: one class's functions" );
  UnitTest(
      [&]()
      {
        return names(
            tree.list_user_functions( query( "Dog", "fetch", ScopeName::Super, "" ), at( 53, 5 ) ),
            name_of_function );
      },
      std::string( "Animal::Animal,Animal::create,Animal::speak" ),
      "list_user_functions: super:: lists the base class's functions" );
  // super() is offered only inside a constructor whose base class has a constructor.
  auto offers_super = [&]( Position pos )
  {
    for ( auto* fn : tree.list_user_functions( query( "Dog", "", ScopeName::None, "" ), pos ) )
      if ( fn->type == UserFunctionType::Super )
        return true;
    return false;
  };
  UnitTest( [&]() { return offers_super( at( 49, 5 ) ); }, true,
            "list_user_functions: super inside a derived class's constructor" );
  UnitTest( [&]() { return offers_super( at( 53, 5 ) ); }, false,
            "list_user_functions: no super inside a method" );

  auto module_function = [&]( ScopeTreeQuery q ) -> std::string
  {
    auto fn = tree.find_module_function( q );
    return fn ? fn->scoped_name() : "";
  };
  UnitTest( [&]() { return module_function( query( "", "", ScopeName::None, "print" ) ); },
            std::string( "basicio::Print" ), "find_module_function: unscoped" );
  UnitTest( [&]()
            { return module_function( query( "", "", ScopeName( "uo" ), "SendSysMessage" ) ); },
            std::string( "uo::SendSysMessage" ), "find_module_function: module-scoped" );
  UnitTest( [&]()
            { return module_function( query( "", "", ScopeName( "basic" ), "SendSysMessage" ) ); },
            std::string( "" ), "find_module_function: wrong module" );
  UnitTest(
      [&]()
      {
        return names( tree.list_module_functions( query( "", "", ScopeName::None, "SendSysMess" ) ),
                      name_of_function );
      },
      std::string( "uo::SendSysMessage" ), "list_module_functions: prefix" );
  UnitTest(
      [&]()
      {
        return names(
            tree.list_module_functions( query( "", "", ScopeName( "basic" ), "SendSysMess" ) ),
            name_of_function );
      },
      std::string( "" ), "list_module_functions: wrong module" );

  UnitTest(
      [&]()
      {
        auto modules = tree.list_modules( query( "", "", ScopeName::None, "" ) );
        std::ranges::sort( modules );
        std::string joined;
        for ( const auto& m : modules )
          joined += ( joined.empty() ? "" : "," ) + m;
        return joined;
      },
      std::string( "basic,basicio,uo" ), "list_modules: every module the script uses" );
  UnitTest( [&]() { return tree.list_modules( query( "", "", ScopeName::None, "u" ) ).size(); },
            size_t{ 1 }, "list_modules: prefix" );
  UnitTest( [&]() { return tree.list_modules( query( "", "", ScopeName( "uo" ), "" ) ).size(); },
            size_t{ 0 }, "list_modules: nothing under a scope" );
}

void scope_constant_test( const ScopeTree& tree )
{
  UnitTest( [&]() { return tree.find_constant( "TOP_CONST" ) != nullptr; }, true,
            "find_constant: global const" );
  UnitTest( [&]() { return tree.find_constant( "RED" ) != nullptr; }, true,
            "find_constant: enum member" );
  UnitTest( [&]() { return tree.find_constant( "Fruit::APPLE" ) != nullptr; }, true,
            "find_constant: enum class member by scoped name" );
  UnitTest( [&]() { return tree.find_constant( "APPLE" ) != nullptr; }, false,
            "find_constant: enum class member is not global" );
  UnitTest(
      [&]()
      {
        return names( tree.list_constants( query( "", "", ScopeName::None, "TOP" ) ),
                      name_of_constant );
      },
      std::string( "TOP_CONST" ), "list_constants: global prefix" );
  UnitTest(
      [&]()
      {
        return names( tree.list_constants( query( "", "", ScopeName( "Fruit" ), "A" ) ),
                      name_of_constant );
      },
      std::string( "Fruit::APPLE" ), "list_constants: enum class scope" );
  // With an empty prefix the global list matches every constant, the enum class's members included.
  UnitTest(
      [&]()
      {
        return names( tree.list_constants( query( "Fruit", "", ScopeName::None, "BAN" ) ),
                      name_of_constant );
      },
      std::string( "Fruit::BANANA" ), "list_constants: inside an enum class, its own members" );
  UnitTest(
      [&]()
      {
        return names( tree.list_constants( query( "Fruit", "", ScopeName::None, "TOP" ) ),
                      name_of_constant );
      },
      std::string( "TOP_CONST" ), "list_constants: inside an enum class, the globals too" );
}

void scope_class_test( const ScopeTree& tree )
{
  UnitTest( [&]() { return tree.find_class( "animal" ) != nullptr; }, true,
            "find_class: case-insensitive" );
  UnitTest( [&]() { return tree.find_class( "Cat" ) != nullptr; }, false,
            "find_class: unknown class" );

  auto member = [&]( const std::string& cls, const std::string& name )
  { return tree.find_class_member( query( cls, "", ScopeName::None, name ) ) != nullptr; };
  UnitTest( [&]() { return member( "Animal", "name" ); }, true,
            "find_class_member: assigned in the constructor" );
  UnitTest( [&]() { return member( "Animal", "captured_member" ); }, true,
            "find_class_member: assigned in a function expression that captures this" );
  UnitTest( [&]() { return member( "Animal", "shadowed_member" ); }, false,
            "find_class_member: not from a function expression with its own this" );
  UnitTest( [&]() { return member( "Dog", "name" ); }, true,
            "find_class_member: inherited from the base class" );
  UnitTest( [&]() { return member( "", "name" ); }, false,
            "find_class_member: needs a calling scope" );
  UnitTest( [&]() { return member( "Cat", "name" ); }, false, "find_class_member: unknown class" );
  UnitTest(
      [&]()
      {
        return names( tree.list_class_members( query( "Dog", "", ScopeName::None, "" ) ),
                      name_of_member );
      },
      std::string( "captured_member,name,tricks" ), "list_class_members: own and inherited" );

  auto method = [&]( const std::string& cls, const std::string& name ) -> std::string
  {
    auto fn = tree.find_class_method( query( cls, "", ScopeName::None, name ) );
    return fn ? fn->scoped_name() : "";
  };
  UnitTest( [&]() { return method( "Dog", "fetch" ); }, std::string( "Dog::fetch" ),
            "find_class_method: own method" );
  UnitTest( [&]() { return method( "Dog", "speak" ); }, std::string( "Animal::speak" ),
            "find_class_method: base class method" );
  UnitTest( [&]() { return method( "Animal", "fetch" ); }, std::string( "" ),
            "find_class_method: not a derived class's method" );
  UnitTest( [&]() { return method( "", "speak" ); }, std::string( "" ),
            "find_class_method: needs a calling scope" );
  UnitTest( [&]() { return method( "Cat", "speak" ); }, std::string( "" ),
            "find_class_method: unknown class" );
  for ( const char* cls : { "", "Cat" } )
  {
    UnitTest( [&]()
              { return tree.list_class_members( query( cls, "", ScopeName::None, "" ) ).size(); },
              size_t{ 0 }, std::string( "list_class_members: none for class '" ) + cls + "'" );
    UnitTest( [&]()
              { return tree.list_class_methods( query( cls, "", ScopeName::None, "" ) ).size(); },
              size_t{ 0 }, std::string( "list_class_methods: none for class '" ) + cls + "'" );
  }
  UnitTest(
      [&]()
      {
        return names( tree.list_class_methods( query( "Dog", "", ScopeName::None, "" ) ),
                      name_of_function );
      },
      std::string( "Animal::speak,Dog::fetch" ),
      "list_class_methods: own and inherited, no static function" );

  auto scopes = [&]( ScopeTreeQuery q )
  {
    auto result = tree.list_scopes( q );
    std::ranges::sort( result );
    std::string joined;
    for ( const auto& s : result )
      joined += ( joined.empty() ? "" : "," ) + s;
    return joined;
  };
  UnitTest( [&]() { return scopes( query( "", "", ScopeName::None, "A" ) ); },
            std::string( "Animal" ), "list_scopes: class prefix" );
  UnitTest( [&]() { return scopes( query( "Dog", "fetch", ScopeName::None, "" ) ); },
            std::string( "Animal,Dog,super" ),
            "list_scopes: super inside a derived class's method" );
  UnitTest( [&]() { return scopes( query( "Animal", "speak", ScopeName::None, "" ) ); },
            std::string( "Animal,Dog" ), "list_scopes: no super without a base class" );
  UnitTest( [&]() { return scopes( query( "", "", ScopeName( "Animal" ), "" ) ); },
            std::string( "" ), "list_scopes: nothing under a scope" );
}

// Bottom reaches Empty through both Left and Right. Kid's only base class has no functions.
const char* const diamond_fixture =
    "class Empty()\n"
    "endclass\n"
    "class Left( Empty )\n"
    "  function Left( this )\n"
    "  endfunction\n"
    "  function from_left( this )\n"
    "  endfunction\n"
    "endclass\n"
    "class Right( Empty )\n"
    "  function Right( this )\n"
    "  endfunction\n"
    "endclass\n"
    "class Bottom( Left, Right )\n"
    "  function Bottom( this )\n"
    "    super();\n"
    "  endfunction\n"
    "endclass\n"
    "class Kid( Empty )\n"
    "  function Kid( this )\n"
    "  endfunction\n"
    "  function play( this )\n"
    "  endfunction\n"
    "endclass\n"
    "print( Bottom() + Kid() );\n";

void test_scope_tree_diamond()
{
  auto a = analyze_src( "diamond.src", diamond_fixture );
  if ( !a->workspace || !a->workspace->scope_tree )
  {
    UnitTest::inc_failures();
    UnitTest::report_failure( "diamond fixture", "no scope tree", "a scope tree" );
    return;
  }
  const auto& tree = *a->workspace->scope_tree;
  auto scopes = [&]( ScopeTreeQuery q )
  {
    auto result = tree.list_scopes( q );
    std::ranges::sort( result );
    std::string joined;
    for ( const auto& name : result )
      joined += ( joined.empty() ? "" : "," ) + name;
    return joined;
  };
  UnitTest(
      [&]()
      {
        return names( tree.list_class_methods( query( "Bottom", "", ScopeName::None, "" ) ),
                      name_of_function );
      },
      std::string( "Left::from_left" ), "list_class_methods: a shared base is visited once" );
  UnitTest( [&]() { return scopes( query( "Bottom", "Bottom", ScopeName::None, "" ) ); },
            std::string( "Bottom,Empty,Kid,Left,Right,super" ),
            "list_scopes: super when a base class has functions" );
  UnitTest( [&]() { return scopes( query( "Kid", "play", ScopeName::None, "" ) ); },
            std::string( "Bottom,Empty,Kid,Left,Right" ),
            "list_scopes: no super when the base class has no functions" );
}

// ecompile appends the source line to this warning. The extension marks the location itself, so
// analysis keeps the message to one line.
void test_short_circuit_warning()
{
  auto& cfg = Bscript::compilercfg;
  const bool saved_eval = cfg.ShortCircuitEvaluation;
  const bool saved_warning = cfg.ShortCircuitEvaluationWarning;
  cfg.ShortCircuitEvaluation = true;
  cfg.ShortCircuitEvaluationWarning = true;
  auto a = analyze_src( "shortcircuit.src",
                        "program p()\n"
                        "  var a := 1;\n"
                        "  if ( a && ( a := 2 ) )\n"
                        "    print( a );\n"
                        "  endif\n"
                        "endprogram\n" );
  cfg.ShortCircuitEvaluation = saved_eval;
  cfg.ShortCircuitEvaluationWarning = saved_warning;

  UnitTest(
      [&]() -> std::string
      {
        for ( const auto& diagnostic : a->reporter.diagnostics )
          if ( diagnostic.message.starts_with( "Short circuit change" ) )
            return diagnostic.message.find( '\n' ) == std::string::npos ? "one line" : "two lines";
        return "no warning";
      },
      std::string( "one line" ), "short-circuit warning: one line under analysis" );
}

void test_scope_tree()
{
  auto a = analyze_src( "scopes.src", scope_fixture );
  UnitTest( [&]() { return a->reporter.diagnostics.size(); }, size_t{ 0 },
            "scope fixture has no diagnostics" );
  if ( !a->workspace || !a->workspace->scope_tree )
  {
    UnitTest::inc_failures();
    UnitTest::report_failure( "scope fixture", "no scope tree", "a scope tree" );
    return;
  }
  const auto& tree = *a->workspace->scope_tree;
  scope_variable_test( tree );
  scope_function_test( tree );
  scope_constant_test( tree );
  scope_class_test( tree );
}
}  // namespace

void compiler_analysis_test()
{
  ScopedCompilerConfig cfg;

  test_clean_source();
  test_module();
  test_module_unreadable();
  test_continue_on_error();
  test_missing_module_continues();
  test_missing_include_continues();
  test_duplicate_method_keeps_first();
  test_case_group_without_label();
  test_report_clear();
  test_scope_tree();
  test_scope_tree_diamond();
  test_short_circuit_warning();
}
}  // namespace Pol::Testing
