%lex
%%
\/+.+    {/* ignore */}
\s+       {/* ignore */}
"("       { return '('; }
")"       { return ')'; }
":="      { return ":="; }
"+"       { return "+";  }
"const"   { return "CONST"; }
"{"       { return "{"; }
"}"       { return "}"; }
\"[^"]*\" { return "STRING"; }
"array"   { return 'ARRAY'; }
"struct"  { return 'STRUCT'; }
(0x[0-9a-fA-F]+)|([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))    { return 'NUMBER'; }
[a-zA-Z_][a-zA-Z_0-9]*  { return 'ID'; }
","       { return ','; }
";"       { return ';'; }
<<EOF>>   { return 'EOF'; }
/lex


%left '+'
%left SEP

%%



file
  : func_def_list EOF
    {{ return { constants: this.constants, functions: this.functions}; }}
  ;


const_def
  : CONST id ':=' constexpr ';'
   {{ 
      $$ = { "constdef": { id: $id, value: $constexpr } }; 
      if (!this.constants) this.constants = { [$id]: $constexpr };
      else if (typeof this.constants[$id] !== "undefined") throw new Error(`Constant redefinition: ${$id}`);
      this.constants[$id] = $constexpr;
    }}
  ;

func_def
  : id '(' arg_list ')' ';'
  {{
    $$ = { "funcdef": { name: $id, args: $arg_list } }; 
    if (!this.functions) this.functions = { [$id]: $arg_list };
    else if (this.functions[$id]) throw new Error(`Function redefinition: ${$id}`);
    this.functions[$id] = $arg_list;
  }}
  ;

module_entry
  : func_def
    { $$ = $func_def; }
  | const_def
    { $$ = $const_def; }
  ;

func_def_list
  : func_def_list func_def 
   { $$ = $func_def_list  ; $$.push($func_def); }
  | func_def_list const_def 
   { $$ = $func_def_list ; $$.push($const_def); }
  | module_entry
   { $$ =  [ $module_entry ]; }
  ;


arg_list
  : arg_list ',' arg
    { $$ = $arg_list; $$.push($arg); }
  | arg 
    { $$ = [$arg]; }
  | 
    { $$ = []; }
  ;

array_optional_initializer: 
'{' constexpr_list '}'
    { $$ = $constexpr_list; }
 | '{' '}'
    { $$ = []; }
 | 
    { $$ = []; }
  ;

struct_constexpr_list
  : struct_constexpr_list ',' ID ':=' constexpr
    { $$ = $struct_constexpr_list; $$[$ID] = $constexpr; }
  | ID ':=' constexpr 
    { $$ = { [$ID]: $constexpr }; }
;

struct_optional_initializer: 
'{' struct_constexpr_list '}'
    { $$ = $struct_constexpr_list; }
 | '{' '}'
    { $$ = []; }
  ;

constexpr_list
  : constexpr_list ',' constexpr
  { debugger; $$ = $constexpr_list; $$.push($constexpr); }
  | constexpr
  { debugger; $$ = [$constexpr]; }
  ; 

constexpr 
  : constexpr '+' constexpr
  {{ 
    if (typeof $1 === "number" && typeof $3 === "number") $$ = $1 + $3;
    else throw new Error("Cannot add non-constants " + $1 + " + " + $3);
  }}
  | NUMBER
  {{ 
    $$ = Number.parseInt(yytext); 
  }}
  | STRING
  { $$ = yytext.substr(1,yytext.length-2); }
  | ARRAY array_optional_initializer
  { $$ =  $array_optional_initializer }
  | STRUCT struct_optional_initializer
  { $$ =  $struct_optional_initializer }
  | id
  {{
    if (typeof this.constants[yytext] === "undefined") throw new Error("Unknown constant " + yytext);
    $$ = this.constants[yytext]; 
  }}
  ; 

optional_initializer
  : ':=' constexpr
    { $$ = $constexpr; }
  | ''
    { $$ = undefined; }
  ;



arg
  : id optional_initializer
    { $$ = { name: $id, default: $optional_initializer }; }
  ;

id
  : ID 
    { $$ = yytext; }
  ;
  