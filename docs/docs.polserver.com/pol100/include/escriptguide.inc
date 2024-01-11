<h1>Racalac's eScript Reference and Guide (POL 100.1.0)<br>
<font size="2">(Last Updated: March 14, 2021)</font></h1>

<!-- TODO:
	- talk about ecompile.cfg and recursive compiling, mention POL ignores
	  files starting with '.'
	- talk about escape sequences in string, do not forget "\xFF"
-->

<br/>

<p><b>Foreword:</b><br>This reference is targeted to be used and understood by
scripters who have at least a little experience in other procedural-type
languages. If you have never done any type of scripting or programming before,
I'd suggest that you look for a class on BASIC or Pascal in your school or
college. If you're not in college, think about buying a "dummies" book on
programming from your local book store. Any of these methods will make learning
and writing eScript easier and quicker for you. And on the other extreme, my
apologies if I explain some things in annoyingly simple terms.</p>

<br/>

<!-- ====================================================================== -->

<h3>Table of Contents</h3>

<p><b><a href="#chap1">Chapter 1:</a></b>
</p><ul>
  <li><a href="#chap1a">Variable
  Declaration and Assignment</a>
  </li><li><a href="#chap1b">Arithmetic</a>
  </li><li><a href="#chap1c">Comments</a></li></ul>
<p></p>

<p><a href="#chap2"><b>Chapter 2: Brief Review of Data Structures </b></a>
</p><ul>
  <li><a href="#chap2a">Arrays</a>
  </li><li><a href="#chap2b">Structures</a></li><li><a href="#chap2c">Dictionaries</a></li></ul>
<p></p>

<p><b><a href="#chap3">Chapter 3: Conditionals and Iteration</a></b>
</p><ul>
  <li><a href="#chap3a">If-statements</a>
  </li><li><a href="#chap3b">Binary Operators</a>
  </li><li><a href="#chap3b2">Conditional (Ternary) Operator</a>
  </li><li><a href="#chap3c">Case (switch) statements</a>
  </li><li><a href="#chap3d">Iteration</a></li></ul>
<p></p>

<p><b><a href="#chap4">Chapter 4:</a></b>
</p><ul>
  <li><a href="#chap4a">Functions and Parameters</a>
  </li><li><a href="#chap4b">Including Code from Other Files</a></li></ul>
<p></p>

<p><b><a href="#chap5">Chapter 5:</a></b>
</p><ul>
  <li><a href="#chap5a">Scripts</a>
  </li><li><a href="#chap5b">Compiling</a></li></ul>
<p></p>

<p><b><a href="#chap6">Chapter 6: Built-in Properties and the POL Object Reference</a></b></p>

<p><b><a href="#chap7">Chapter 7:</a></b>
</p><ul>
  <li><a href="#chap7a">CProps</a>
  </li><li><a href="#chap7b">CProps on Accounts</a></li></ul>
<p></p>

<p><b><a href="#chap8">Chapter 8: Config File Usage and Access</a></b></p>

<p><b><a href="#chap9">Chapter 9: Packages</a></b></p>

<p><b><a href="#chap10">Chapter 10: Debugging</a></b></p>

<p><b><a href="#chap11">Chapter 11: Advanced Data Types and Functions</a></b>

</p><ul>
  <li><a href="#chap11a">Global Properties</a>
  </li><li><a href="#chap11b">Arrays</a>
  </li><li><a href="#chap11c">Structs</a>
  </li><li><a href="#chap11d">Dictionaries</a>
  </li><li><a href="#chap11e">Function Calls by Reference</a>
  </li><li><a href="#chap11f">Interpolated Strings</a></li></ul>
<p></p>

<p><b><a href="#app2">Appendix A: Gump Tag Descriptions</a></b></p>

<p><b><a href="#appB">Appendix B: Operator Precedence</a></b></p>

<p><b><a href="#appX">Appendix X: Revision History</a></b></p>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap1">Chapter 1</a></h3>
<br/>
<p><b><a name="chap1a">Variable Declaration and Assignment; Arithmetic</a></b></p>

<p>There are no strict variable types in eScript, but there are a few types you
can cast (convert) to: integer (whole number), real (float; integer with decimal
part), and string (text). It is simple to declare a new variable:</p>

<div class="doc-guide-pre">
<pre>var my_variable;</pre></div></p>

<p>Depending where you declare this in your program depends on when you can have
access to that variable. This is called the variable's scope. If you place the
above line outside a function block, it is visible to all functions in the
program (known as a 'global' variable). If you place that line inside a function
block, it is accessable no matter where you currently are in that function. If
you delcare it inside a code block (i.e. between if-endif, for-endfor, etc), it
is only usable inside that block. Note in older scripts you may see variable
declared with 'local' and 'global'. The first is the same as 'var' inside a
function and the second is the same as 'var' outside a function. You should
always use 'var' instead of these older keywords to declare your variables.</p>

<p>Okay, now we have some variables, but now to put some data into them so we
can use them in the script. You can do this either at declaration-time or later
in the script. The syntax for this in eScript is:</p>

<div class="doc-guide-pre">
<pre>my_variable := 10;</pre></div></p>

<p>The ":=" (colon + equals) operator assigns the right-hand-value to the
left-hand variable. A lot of different expressions can be used on the
right-hand-side. Some of these include numbers (such as above), other variables,
mathematical expressions, function calls, and more. You can perform these at
declaration-time like: var number_of_pies := 15; </p>

<p>Variables that are not given any initializer will be set to the
"uninitialized value". This is equivalent to the concept of <code>NULL</code>,
<code>nil</code>, and <code>None</code> in other programming languages. The
eScript keyword for the uninitialized value is <code>uninit</code> and can be
used in any place expressions are used.</p>

<p>To declare a "constant," or a variable that you can only read its value from,
you must do it at the top of the file, before any of the functions, much like
declaring a global variable since constants are global themselves. Whenever the
compiler sees a constant in an expression, the value of that constant is
substituted instead. The syntax for this is:</p>

<div class="doc-guide-pre">
<pre>CONST MY_CONSTANT := 47;</pre></div></p>

<p>The use of constants is key in well-written, readable code. They remove the
ambiguity of having raw numbers thrown around in a script by giving a meaning to
them. It is a good idea to use the convention of declaring constants in ALL CAPS
since most programmers are used to it.</p>

<p>Example snip of code (doesn't do much but show different ways of declaring
variables):</p>

<div class="doc-guide-pre">
<pre>CONST SIZE_OF_TRAY := 15;<br>var pies_in_tray := SIZE_OF_TRAY;</pre></div></p>

<br/>

<p><b><a name="chap1b">Arithmetic</a></b></p>

<p>This stuff is easy, the expression on the left of the assignment operator is
evaluated and placed into the variable on the right side.</p>

<p>Arithmetic expressions are evaluated first inside parenthesis then from
left-to-right. Such as:</p>

<div class="doc-guide-pre">
<pre>var pies_used := pies_used + 1; // Increment 'pies_used' by 1
var pies_left := number_of_trays * (pies_in_tray &#8211; pies_used);
</pre></div>

<p>The math operators in
eScript are:</p>

<table border="1" cellpadding="2" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" id="AutoNumber1">
  <tr>
    <td align="center">Operator</td>
    <td align="center">Meaning</td>
  </tr>
  <tr>
    <td align="center">+</td>
    <td align="center">Addition</td>
  </tr>
  <tr>
    <td align="center">-</td>
    <td align="center">Subtraction</td>
  </tr>
  <tr>
    <td align="center">*</td>
    <td align="center">Multiplication</td>
  </tr>
  <tr>
    <td align="center">/</td>
    <td align="center">Division</td>
  </tr>
  <tr>
    <td align="center">%</td>
    <td align="center">Modulus</td>
  </tr>

  <tr>
    <td align="center">+=</td>
    <td align="center">Addition Assign</td>
  </tr>

  <tr>
    <td align="center">-=</td>
    <td align="center">Subtraction Assign</td>
  </tr>

  <tr>
    <td align="center">*=</td>
    <td align="center">Multiplication Assign</td>
  </tr>

  <tr>
    <td align="center">/=</td>
    <td align="center">Division Assign</td>
  </tr>

  <tr>
    <td align="center">%=</td>
    <td align="center">Modulus Assign</td>
  </tr>

  <tr>
    <td align="center">++</td>
    <td align="center">Unary Increment</td>
  </tr>

  <tr>
    <td align="center">--</td>
    <td align="center">Unary Decrement</td>
  </tr>

  <tr>
    <td align="center">&gt;&gt;</td>
    <td align="center">Bit-Shift right</td>
  </tr>

  <tr>
    <td align="center">&lt;&lt;</td>
    <td align="center">Bit-Shift left</td>
  </tr>

  <tr>
    <td align="center">&amp;</td>
    <td align="center">Bitwise <b>AND</b></td>
  </tr>

  <tr>
    <td align="center">|</td>
    <td align="center">Bitwise <b>OR</b></td>
  </tr>

  <tr>
    <td align="center">^</td>
    <td align="center">Bitwise <b>XOR</b></td>
  </tr>

  <tr>
    <td align="center">~</td>
    <td align="center">Ones Complement</td>
  </tr>
</table>

<h3><b>Assign, Unary, and Bitwise Operators</b></h3>
<h4><b>The Assign Operators:</b></h4>

<p>These operators perform the mathematical operation and then assign the result to the variable
on the left. Using these operators simplifies coding. Instead of writing:</P>
<div class="doc-guide-pre">
<pre>
x := x + 5
</pre></div>
you can write:
<div class="doc-guide-pre">
<pre>
x += 5.
</pre></div>

<h4><b>The Unary Operators:</b></h4>

<p>For those unfamiliar with the unary operators, a brief explanation of their
use is in order. Examples of unary operator syntax is, ++i, i++,--i, and i--.
The 'i' can be any named variable.
These function as another shorthand for incrementing and decrementing a variable.
They do act differently though depending on whether the operator, ++ or -- precedes or
follows the variable. If the operator precedes the variable, as in ++i, that
is equivalent to i += 1. If the operator follows the variable, as in i++, then
i is incremented after any assignment of the value of i might have taken place.</p>
Example:
<div class="doc-guide-pre">
<pre>
var i := 5;
var j :=0;
j := i++;
...
</pre></div>
After executing that code snippet, j will be equal to 5 and i will be equal to 6.

As these are unary operators they can be used directly as a function parameter
or in loops, eg. the while loop example below.
Instead of writing:
<div class="doc-guide-pre">
<pre>
while (i<5)
i+=1;
...
</pre></div>
You can save a line of code by writing:
<div class="doc-guide-pre">
<pre>
while (++i<5)
...
</pre></div>
<p>I should note that the assign and unary operators were added after the majority of
this guide was written. So most, if not all, of the example code in
this guide will use the longer forms of expressions, for readability, but mostly because
we are too lazy to edit the entire guide.</p>

<h4><b>The Bitwise (sometimes called Logical) Operators:</b></h4>

<h4>Bitwise <b>AND</b></h4>
The single &amp; (ampersand) symbol will perform a logical <b>AND</b> of the value on the left of the operator with the value on the right.
If we have a statement
<div class="doc-guide-pre">
<pre>
var value := 10 &amp; 14;
</pre></div>
The result will equal 10. What is actually happening is that eScript will AND the binary value of 10, 1010, with the binary value of 14, 1110.
<p>Every 1 bit in the value on the left that corresponds to a 1 bit in the same position in the value on the right produces a 1 bit in that position in the result. You can see this visually in the table below.</p>
<table border="1" cellpadding="2" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111">
	<tr>
    	<td>Value: 10</td><td>1</td><td>0</td><td>1</td><td>0</td>
    </tr>
	<tr>
    	<td>Value: 14</td><td>1</td><td>1</td><td>1</td><td>0</td>
    </tr>
	<tr>
    	<td><b>AND</b> (10)</td><td>1</td><td>0</td><td>1</td><td>0</td>
    </tr>
</table>
<br>
<h4>Bitwise <b>OR</b></h4>

<p>The single | (bar) symbol will perform a logical <b>OR</b> of the value on the left of the operator with the value on the right. If we have a statement</P>
<div class="doc-guide-pre">
<pre>
var value := 10 | 6;
</pre></div>
<p>The result will equal 14. Escript will <b>OR</b> the binary value of 10, 1010, with the binary value of 6, 0110. Where there is a 1 bit in either the value on the left or the value on the right, or both bits are 1, produces a 1 bit in the result. The table below helps you visualize this.</p>

<table border="1" cellpadding="2" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111">
	<tr>
    	<td>Value: 10</td><td>1</td><td>0</td><td>1</td><td>0</td>
    </tr>
	<tr>
    	<td>Value: 6</td>
    	<td>0</td>
    	<td>1</td><td>1</td><td>0</td>
    </tr>
	<tr>
    	<td><b>OR</b> (14)</td>
    	<td>1</td>
    	<td>1</td>
    	<td>1</td><td>0</td>
    </tr>
</table>
<br>
<h4>Bitwise <b>XOR</b></h4>
<p>
The ^ (caret) will perform a logical <strong>XOR</strong>, or exclusive <strong>OR</strong>, of the value on the left of the operator with the value on the right. If we have a statement:
<div class="doc-guide-pre">
<pre>
var value := 10 ^ 12;
</pre></div>
The result will equal 6. This time eScript will <strong>XOR</strong> the binary value of 10, 1010, with the binary value of 12, 1100. Where there is a 1 bit in either the value on the left or the value on the right produces a 1 bit in the result. Where there is a 0 or a 1 in both bits produces a 0. Again, see the table below.</p>


<table border="1" cellpadding="2" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111">
	<tr>
    	<td>Value: 10</td><td>1</td><td>0</td><td>1</td><td>0</td>
    </tr>
	<tr>
    	<td>Value: 12</td>
    	<td>1</td>
    	<td>1</td>
    	<td>0</td>
    	<td>0</td>
    </tr>
	<tr>
    	<td><b>XOR</b> (6)</td>
    	<td>0</td>
    	<td>1</td>
    	<td>1</td><td>0</td>
    </tr>
</table>
<br>
<h4><strong id="docs-internal-guid-f9b28dc1-7fff-cec3-02c5-0ed74c4fce53">Ones&rsquo; complement</strong></h4>
<p>
The ~ (tilde) symbol will perform a ones’ complement on the value to the right of the operator. A ones’ complement operation inverts all the bits in an integer. 1s are flipped to 0s and 0s are flipped to 1s. The following statement will return -2 or in hexadecimal 0xFFFFFFFE.
Var value := ~1;
In a 32 bit hexadecimal representation the value 1 is 0x00000001 or, in binary
0000 0000 0000 0000 0000 0000 0000 0001
If you invert the bits, ones’ complement, you end up with the result, in hexadecimal, of 0xFFFFFFFE or in binary
1111 1111 1111 1111 1111 1111 1111 1110
Quoting “The Book of Knowledge”, Wikipedia: <em>“The ones' complement of the number then behaves like the negative of the original number in some arithmetic operations. To within a constant (of -1), the ones' complement behaves like the negative of the original number with binary addition. However, unlike two's complement, these numbers have not seen widespread use because of issues such as the offset of -1, that negating zero results in a distinct negative zero bit pattern, less simplicity with arithmetic borrowing, etc.”</em>.


<h3>In Addition</h3>
<p>I should mention something about variable types in arithmetic, especially
  division. If you divide two integers together such that the result would have a
  decimal part, POL will only look at the whole number part. To make sure the
  result is a real (integer+decimal part), make sure at least one of the operands
  is a real. You can make sure of this by using a raw number with a decimal part
  (such as 100.0), or "cast" it to a real (convert the type). There are a few
casting functions in eScript, CInt(), CStr(), CDbl().</p>

<div class="doc-guide-pre">
<pre>
CInt(100.3)  	   	// Results in 100
CStr(100); 	  	  // Results in a string of "100"
                  // This now cannot be used in mathematical expressions.
CDbl(100); 	  	  // Results in 100.0
</pre></div>

<p>Additional conversion functions in basic.em</p>

<div class="doc-guide-pre">
<pre>
Hex(100);  	   	// Results in a string as "0x64"
Bin(100); 	  	// Results in a string as "1100100"
                // CInt() will not properly convert this string to its decimal value.
CAsc("A"); 	  	// Results in an integer of 65
CChr(65); 	  	// Results in a string as "A"
</pre></div>

<p><b><a name="chap1c">Bonus Topic: Comments</a></b></p>

<p>Commenting your code is of utmost importance. Comments allow you to add
arbitrary text to explain whatever you need to without disrupting the script's
functionality. Comments are also used to remove code from execution without
actually deleting it (known as "commenting out code"). In eScript there are a
couple ways to comment your code:</p>

<ol>
  <li>
  <p>Any text after "//" will be considered a comment until the end of the line.
  The next line will not be considered a comment. To make it a comment, add
  another "//" before your comment. This is the best way to make short comments
  because it allows you to comment out large block of code using "/* */"
  (below).</p>
  </li><li>
  <p>Any text after "/*" and before "*/" is considered a comment. Using this,
  you can comment out large blocks of code just by placing a /* at the beginning
  and / at the end of the code. You cannot "nest" this type of comment, as the
  next "*/" signals the end of the comment, even if another "*/" is found after
  the first. This is why we recommend you comment most things with "//" to avoid
  this.<br>
&nbsp;</p></li></ol>

<!-- ====================================================================== -->

<h3><a name="chap2">Chapter 2</a></h3>
<br/>
<p><b>Brief Review of Data Structures</b></p>

<p>Okay, so we can make integer, real, or string variables. To do any sort of
useful scripting, we also need some data structures. I will discuss arrays, structures
and dictionaries in this chapter.<br>
</p>

<br/>

<p><b><a name="chap2a">Arrays</a></b></p>

<p>Arrays are one-based, random access collections of objects. That means you
can access any place in the array from 1 (the first element) up to X (the last
element). Let's first look how you can declare an array. </p>

<p>To create an array variable, declare:</p>

<div class="doc-guide-pre">
<pre>
var a := array;                           // empty array
var b := array{5, 32, "hello", "world"};  // initialized array
var c := array{array{1,2}, array{3,4,5}}; // array containing arrays
</pre></div>

<p>An array can be assigned to any other variable, even if that variable was not
declared an array:</p>

<div class="doc-guide-pre">
<pre>
var a := array{2, 4, 6, 8};
var b;<br>b := a;</pre></div>

<p>Similarly, if a function returns an array (we will discuss this later), no
special declaration is needed:</p>

<div class="doc-guide-pre">
<pre>
var a;
a := FunctionThatReturnsAnArray();</pre></div>

<p>In some other programming languages, you cannot access (read or write) past
the end of the declared size of the array. In eScript, arrays grow automatically
without access errors:</p>

<div class="doc-guide-pre">
<pre>
var a := array;
a[1] := 4;
a[4] := 7;</pre></div>

<p>Arrays elements can be any type of object, including another array:</p>

<div class="doc-guide-pre">
<pre>
var a := array{};
var b := array{};
a[1] := 5;
b[1] := a;
b[2] := 6;<pre></div>

<p>The following are equivalent methods of looping through an array. The
'foreach' method is much more efficient, as well as being more convenient. Note
that we will discuss these kind of loops and more next chapter.</p>

<div class="doc-guide-pre">
<pre>
var a := array{2,4,6,8};

var i;
for(i:=1; i&lt;=a.Size(); i:=i+1)
  Print(a[i]);
endfor

foreach i in ( a )
  Print(i);
endforeach</pre></div>

<p>You can then use the array for whatever you need, such as passing more
than one variable back to a function (functions explained later), or when a core
function requires it. <br><br>

<p>Suppose you want to find out if the value in the variable "name" exists in
an array named "people". You can do something like the following to:<br>

<div class="doc-guide-pre">
<pre>
foreach person in people
  if(person == name)
    return _person_iter;
  endif</pre></div>

or you can do something even simpler:<br>

<div class="doc-guide-pre">
<pre>
var in_array := name in people;</pre></div>

in_array will be set to the ordinal position where the value in name exists
in the array or if it is not in the array in_array will be set to 0.<br><br>

Also, there are a few array-specific functions that are
explained later (such as array.Insert() and others).</p>

<br/>

<p><b><a name="chap2b">Structures</a></b></p>

<p>Structures are an object-oriented data type with various members or
properties that are part of them that contain the actual data.</p>

<p>To access members, use the '<b>.</b>' operator:</p>

<div class="doc-guide-pre">
<pre>
Print(a.height);
Print(a.width);</pre></div>

<p>To create an structure variable, declare:</p>

<div class="doc-guide-pre">
<pre>
var a := struct;                             // empty struct
var b := struct{&quot;height&quot;, &quot;width&quot;};          // struct with members that have no values.
var c := struct{&quot;height&quot;:=5, &quot;width&quot;:=100};  // struct with members and values.
</pre></div>

<p>To add new members, use the '<b>.+</b>' operator:</p>

<div class="doc-guide-pre">
<pre>
var a:= struct;
a.+height := 7;
a.+width;

// Assign an existing member
a.width := 5;</pre></div>
<br/>
<p><a name="chap2c"><b>Dictionaries</b></a></p>

<p>Dictionaries are arrays whose elements are named. They are often referred to
as 'associative arrays'</p>

<div class="doc-guide-pre">
<pre>
Print(a[&quot;height&quot;]);
Print(a[&quot;width&quot;]);</pre></div>

<p>To create an structure variable, declare:</p>

<div class="doc-guide-pre">
<pre>
var a := dictionary;                         // empty dictionary
var b := dictionary{&quot;height&quot;, &quot;width&quot;};          // dictionary with indexes that have no values.
var c := dictionary{&quot;height&quot;-&gt;5, &quot;width&quot;-&gt;100};  // dictionary with indexes and values.
</pre></div>

<p>Dictionaries automatically add new indexes when they are set.</p>

<div class="doc-guide-pre">
<pre>
var a:= dictionary;
a[&quot;height&quot;] := 7;
a[&quot;width&quot;] := 100;</pre></div>

<!-- ====================================================================== -->

<h3><a name="chap3">Chapter 3</a></h3>
<br/>
<p><b>Conditionals and Iteration</b></p>

<p>We'll cover two topics in this chapter: IF-statements and loops. </p>
<br/>
<p><b><a name="chap3a">If-statements</a></b></p>

<p>If-statements are absolutely needed in every script so it's necessary you
understand them. If statements simply let your script make decisions based on
the criteria you give it. The general syntax is:</p>

<div class="doc-guide-pre">
<pre>
if(statement)
  // code
elseif(statement)
  // code
elseif(statement)
  // code
else
  // code
endif</pre></div>

<p>Notice you can have any number of (optional)"elseif" statements between the
"if" and the (optional) "else".<br>
The key part in the syntax above is "statement".
When the script runs, it will check the first "statement" to see if it is true.
<br>
If it is not, it'll proceed to check the next "statement" until either it finds
one that is true, or hits the "else" statement (if any).</p>

<p>The syntax for "statement" is</p>

<div class="doc-guide-pre">
<pre>
variable1 {operator variable2} ...</pre></div>
<p>Some examples of evaluating to true expressions are:</p>

<div class="doc-guide-pre">
<pre>
var var1 := 10;
var var2 := 5;

if (var1 &gt; var2)  //TRUE

if ((var1 &#8211; 5) == var2) //TRUE

if (var2 &lt; var1) //TRUE

if ((var2+5) &gt;= var2) //TRUE<

if (var1 != var2) //TRUE
</pre></div>

<p>The logical operators in eScript are (evaluated left-to-right)</p>

<p>== equal-to<br>!= not-equal-to<br>&gt; greater-than<br>&lt; less-than
<br>&lt;= less-or-equal-to <br>&gt;= greater-or-equal-to</p>

<p>Also, you can use the Boolean operators:</p>

<table border="0" cellpadding="2" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" id="AutoNumber2">
  <tr>
    <td>&nbsp;<b>||</b> or &quot;<b>or</b>&quot;</td>
    <td>&nbsp;// <b>||</b> is the preferred method.</td>
  </tr>
  <tr>
    <td>&nbsp;<b>&amp;&amp;</b> or &quot;<b>and</b>&quot;&nbsp;&nbsp; </td>
    <td>&nbsp;// <b>&amp;&amp;</b> is the preferred method.&nbsp; </td>
  </tr>
</table>

<p>For example:</p>

<div class="doc-guide-pre">
<pre>
if ( (var1 &gt; var2) || (var1 == 5) )
  // TRUE since at least one of the individual expressions is true. This is inclusive OR so all expressions may
  // be true and the whole statement is true.

if ( (var1 &gt; var2) or (var1 ==5) )
  //this is same as above, except using "or" in place of "||" for readability.

if ( (var1 &gt; var2) &amp;&amp; (var1 == 5) )
  //FALSE, since not ALL expressions are true.

if ( (var1 &gt; var2) and (var1 == 10) )
  //TRUE, since both expressions are true. Uses "and" in place of "&amp;&amp;"
</pre></div>

<p>You will use If statements in all but the most simple scripts you write. You
should understand what you want to do before throwing a conditional statement
together, as it is possible to mess it up:</p>

<div class="doc-guide-pre">
<pre>
if ( score &lt; 60 )
  Print("You got less than 60");
elseif ( score == 50 )
  Print("You got 50!");
endif</pre></div>

<p>This is poor scripting, as if your score was 50, the first statement would be
true (printing "You got less than 60"), and the elseif(score == 50) line would
never be printed, even though it is a more specific check than the previous
line. </p>

<p>Also, in eScript anything that is zero is considered to be false, and
anything non-zero is true. So you can have an if-statement without any logical
operators:</p>

<div class="doc-guide-pre">
<pre>
var valid := 1;
if( valid )
  // do stuff
else
  // do other stuff
endif</pre></div>

<p>Or:</p>

<div class="doc-guide-pre">
<pre>
if ( !valid ) //same as "if valid is non-zero, then false"
  // do stuff
endif</pre></div>
<br/>
<p><b><a name="chap3b">Binary Operators</a></b></p>
<p><b>Elvis operator:</b></p>
<p>Short-circuit binary operator EXPR_LHS ?: EXPR_RHS<br/>
This operator evaluates the left-hand-side operand and keeps its value if it is a true value.<br/>
Otherwise, evaluates the right-hand side operand and uses its value instead.<br/>
This is a short-circuit operator: if EXPR_LHS is true, the code for EXPR_RHS won't be executed.<br/>
These can also be changed: A ?: B ?: C ?: D evaluates to the first true value from left to right.<br/>
It has lower precedence than the binary additive operators (+ -), and higher precedence than the binary comparison operators (&gt; &gt;= &lt; &lt;=).<br/>
These are equivalent with or without parenthesis:<br/>
<div class="doc-guide-pre">
<pre>
  a ?: b + c     a ?: ( b + c )   // lower precedence than addition<br/>
  a ?: b &lt;= c    (a ?: b) &lt;= c    // higher precedence than comparison<br/></pre></div>
See also https://en.wikipedia.org/wiki/Elvis_operator<br/>
For example, you could change this:<br/>
      <div class="doc-guide-pre">
<pre>var difficulty;<br/>
      if(targetsquare.objtype)<br/>
        difficulty := GetHarvestDifficulty(&quot;fish&quot;, targetsquare.x, targetsquare.y, targetsquare.objtype, who.realm);<br/>
      else<br/>
        difficulty := GetHarvestDifficulty(&quot;fish&quot;, targetsquare.x, targetsquare.y, locinfo.landtile, who.realm);<br/>
      endif<br/></pre></div>
to this:
      <div class="doc-guide-pre">
<pre>var difficulty := GetHarvestDifficulty(&quot;fish&quot;,<br/>
                                             targetsquare.x, targetsquare.y,<br/>
                                             targetsquare.objtype ?: locinfo.landtile,<br/>
                                             who.realm);</pre></div></p>


<p><b><a name="chap3b2">Conditional (Ternary) Operator</a></b></p>
<br />
<p>Added in: POL 100.1.0</p>
<br />
<p>The conditional operator is a commonly used shortcut for the if statement, to
use one of two values depending on a conditional. The operator takes three
operands: a condition followed by a question mark (?), then an expression to
execute if the condition is true followed by a colon (:), and finally the
expression to execute if the condition is false.</p>

<div class="doc-guide-pre"><pre>
  condition ? exprIfTrue : exprIfFalse
</pre></div>

The following two functions are equivalent in behavior:

<div class="doc-guide-pre"><pre>
function example()
    return condition1 ? value1
         : condition2 ? value2
         : condition3 ? value3
         : value4;
endfunction
</pre></div>
<div class="doc-guide-pre"><pre>
function example()
    if (condition1) return value1;
    elseif (condition2) return value2;
    elseif (condition3) return value3;
    else return value4; endif
endfunction
</pre></div>

<p><b><a name="chap3c">Case (switch) statements</a></b></p>

<p>It is a common occurance in scripting when you need to make a equality
decision based on a large number of possibilities (and the possibilities are
known at compile-time). To do this kind of choice with if-elseif statements
would be ugly at best and slow at worst. Luckily, eScript provides a cleaner way
to do this:</p>

<div class="doc-guide-pre">
<pre>
// declarations:
const BLUE := 1;
const YELLOW := 2;
const RED := 3;
const MAUVE := 4;

// The ugly if-elseif way:

function FunctionOne()
  var answer := WhatIsYourFavoriteColor();
  if ( answer == BLUE )
    // do stuff
  elseif ( answer == YELLOW )
    // do stuff
  elseif ( answer == RED )
    // do stuff
  elseif ( answer == MAUVE )
    // do stuff
  else
    // do something else
endfunction


// The clean case Way:

function FunctionTwo()
  var answer := WhatIsYourFavoriteColor();
  case ( answer )
    BLUE:
      //do stuff
    YELLOW:
      //do stuff
    RED:
      //do stuff
    MAUVE:
      //do stuff
    default:
      //do something else
  endcase
endfunction</pre></div>

<p>The general structure of a case statement is:<br>

<br>
<div class="doc-guide-pre">
<pre>
case ( expression )
  comparison_value:
    //code
    break;
  comparison_value:
    //code
    break;
  ...
  default:
    //code if no match was found above (optional)
    break;
endcase</pre></div>

<p>"expression" can be determined at run-time, but "comparison_value" cannot.
That means a case statement is really only good for making a decision based on
equality with any number of constant known values. "comparison_value" may not be
any type of expression that must be determined at run time, such as a
mathematical expression. Also, case statements cannot be used for non-equality
comparisons, such as greater-than, less-than, etc. The breaks are optional,
except under one circumstance- an empty case. If you have a comparison value
followed by no code (in other words, immediately followed by another comparison
value), and that comparison value is followed by code, then <i>both</i> of the
cases will execute that code. </p>

<p>For example:

<div class="doc-guide-pre">
<pre>
function FunctionThree()
  var answer := WhatIsYourFavoriteColor();
  case ( answer )
    BLUE:
    YELLOW:
      //do stuff
    RED:
      //do other stuff
    default:
      //do something else
  endcase
endfunction</pre></div>

<p></p>

<p>In this situation, the both BLUE and YELLOW will cause it to do stuff. RED
will make it do other stuff, and anything else will make it do something else.
If you meant BLUE to do nothing at all, you must put <code>break;</code>
statements in the sections which you wish to have do nothing- this will tell the
program to skip to the end of the case block.</p>

<p>The behavior in the case of an empty block (like BLUE above) is known as
<b>fall through</b>.</p>

<br/>

<p><b><a name="chap3d">Iteration</a></b></p>

<p>I mentioned "looping through an array" in the previous chapter. This is a
very common function is most scripts. There are two popular ways of doing this,
here are the code bits I used in the last chapter, one at a time:</p>

<div class="doc-guide-pre">
<pre>
var a := array{2,4,6,8};

var i;
for ( i:=1; i&lt;=a.Size(); i:=i+1)
  Print(a[i]);
endfor
</pre></div>
The output of this code is:
<div class="doc-guide-pre">
<pre>
2
4
6
8</pre></div>

<p>The "for" loop should be familiar to anyone who has done programming before.
The syntax of a for-loop is:</p>

<div class="doc-guide-pre">
<pre>
for (initialization statement ; loop-if-true statement ; increment)
  // code
endfor</pre></div>

<p>Though you can add any code you want in the initialization statement and the
increment statement, I've named them these because that is their most common
function. A for-loop normally is a count-checker. On each go around the loop, it
automatically performs the increment statement, and then checks the loop-if-true
statement to see if it should stop looping or go again. In the above example,
the variable "i" is declared. Before the first loop of the for-loop, i is
initialized to 1. If 'i' is less than or equal to the length of the array named
"a", the code in the for-block is executed. At the end of the for-loop block,
the increment statement is executed, adding 1 to 'i' and storing the new value
in 'i'. Then 'i' is again compared against the length of a (which is 4, using
the above example). 'i' is still less than that, so the code in the block is
executed again. This continues until 'i' is greater than the length of the
array. "endfor" signals the end of the for-loop block.</p>

<p>Another, more convenient method is shown in this example:</p>

<div class="doc-guide-pre">
<pre>
foreach i in ( a )
  Print(i);
endforeach</pre></div>

<p>Here, we don't need to initialize 'i', as the "foreach" loop does it for us.
This loop performs the same function as the for loop above, but in a much more
succinct (and efficient) manner. The general syntax is:</p>

<div class="doc-guide-pre">
<pre>
foreach counter_variable in ( array_name )
  {code}
endforeach
</pre></div>

<p>Bonus Variable:<br>
When you use a foreach loop, you get a _(var)_iter variable with it that tells
you what iteration you are on.</p>

<div class="doc-guide-pre">
<pre>
var a := array{&quot;A&quot;, 1000};
foreach i in ( a )

  Print(_value_iter);
  Print(value);
endforeach</pre></div>
The output of this code is:
<div class="doc-guide-pre">
<pre>
1
A
2
1000</pre></div>

<p>The last type of for loop is the basic-style for loop.</p>

<div class="doc-guide-pre">
<pre>
for i:=0 to 3
  Print(i);
endfor
</pre></div>
The output of this code is:
<div class="doc-guide-pre">
<pre>
0
1
2
3</pre></div>

<p>Another loop type that eScript supports is the "while" loop
(it's a condition-is-true check to loop):</p>

<div class="doc-guide-pre">
<pre>
while ( expression )
  // {code}
endwhile</pre></div>

<p>Pretty easy; until the expression is false, {code} is executed. You will
probably be changing one or more of the variables in "expression" or you will
never exit (there are exceptions to this rule, below).</p>

<p>Another loop type is the "repeat &#8211; until" loop. This is very similar to the
"while" loop, except that you are guaranteed for the code to be executed at
least once, since the conditional check is done at the bottom of the block
(it's a condition-is-false check to loop):</p>

<div class="doc-guide-pre">
<pre>
repeat
  // {code}
until ( expression );</pre></div>

<p>You can use while-loop in similar style with conditional check after code
(it's a condition-is-true check to loop):</p>

<div class="doc-guide-pre">
<pre>
do
  // {code}
dowhile ( expression );</pre></div>

<p>There may be times you want to exit a loop before the exit condition is
satisfied. To do this, you would put the "break" statement in your {code} block
(this works in the "for", "while", and "repeat until" loops). Also, if you want
to stop the execution of the code block, but remain in the loop, use the
"continue" keyword. This will return make the loop start from the top of the
code block again after checking the "condition" statement.</p>

<div class="doc-guide-pre">
<pre>
while ( condition != 0 )
  if ( condition == 42 )
    //oh geez! breakout!
    break;
  elseif ( condition == 13 )
    //don't do the code below the if-block
    continue;
  endif
  condition:= condition * something;
endwhile</pre></div>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap4">Chapter 4</a></h3>
<br/>
<p><b><a name="chap4a">Functions and Parameters</a></b></p>

<p>So far, all I've shown here is code segments which cannot be used by
themselves. All code you write must be placed inside functions. If you are not
familiar with the concept of functions, they are one of the fundamental building
blocks of programming- potentially reusable sections of code. In eScript, there
are two types of functions: "function" and "program". If you have programmed
before, we can compare the "program" function to an application's "main"
function. This means when the script is executed, the entry-point (first
function run) is always the top of the function designated by the keyword
"program". Here's a general syntax for both:</p>

<div class="doc-guide-pre">
<pre>
program ProgramName(parameter, ...)
  // {yourcode}
endprogram

function FunctionName(parameter, ...)
  // {your code}
endfunction</pre></div>

<p>The "endprogram" and "endfunction" keywords identify the end of that
function. These are absolutely necessary and you will get compiler errors if
they are not present (more on compiling later).</p>

<p>FunctionName can be anything, as long as it is one word and it starts with a
letter.</p>

<p>The parameter list is a little more difficult to explain. These are variable
names that were passed to this function from whatever the calling function was.
You can have any variable type as a parameter, even arrays and structures. I'll
show you some examples below. Note what makes the "program" (main function)
special is that you do not choose what gets passed to it. Since the "system"
(the POL core in this case) calls the "program" function, it chooses what
parameters to pass to it. The actual parameter list differs depending on what
type of script it is in (more on that later). But for the sake of showing you
the syntax of the program, let's assume nothing is passed to the main function.
The parameter list may have zero or more parameters in it.</p>

<p>Here's a simple example of how you can create your own functions:</p>

<div class="doc-guide-pre">
<pre>
program Main()
  var var1 := "dudes";
  var var2 := "oingo";
  var var3 := "boingo";
  var var4 := "let's";
  var var5 := 42;

  MyFunction(var1, var4); // 1
  MyFunction(var2, var3); // 2
  MyFunction(var5, var1); // 3
  MyFunction(var3);       // 4
  MyFunction();           // 5
endprogram

function MyFunction(a:= &quot;Yo yo&quot;, b := &quot;hey hey&quot;)
  Print(a + &quot; &quot; + b);
endfunction</pre></div>

<p>Here is the output from the five calls to MyFunction:</p>

<div class="doc-guide-pre">
<pre>
dudes let's
oingo boingo
42 dudes
boingo hey hey
Yo yo hey hey</pre></div>

<p>Notice in the declaration of the MyFunction function, we have a couple
variable names and an assignment for each. What this does is if no variable is
passed for that parameter, the value on the right side of that assignment is
used instead, as can be seen the 4th and 5th lines of output.</p>

<p>This is a very simple example, and does not really show why you should use
separate functions to do stuff rather in all one big function. It is good
programming style to split up tasks into separate functions, and it helps in the
areas of readability, code reuse and abstraction. </p>

<p>Functions may return one value to the calling function before they exit. This
is done with the "return" key word. The calling function receives this value by
having a variable on the left side of an assignment operator and the function
call on the right:</p>

<div class="doc-guide-pre">
<pre>
program Main()
  var hejaz;
  hejaz := SmellsLike();
  Print(hejaz);
endprogram

function SmellsLike()
  var smell := "Teen Spirit";
  return smell;
endfunction</pre></div>

Prints:
<div class="doc-guide-pre">
<pre>Teen Spirit</pre></div>

<p>To return more than one value from a function, assign those values to an
advanced data type (array, struct, dictionary) in the function and then return
the variable.</p>

<div class="doc-guide-pre">
<pre>
program Main()
  var hejaz;
  hejaz := SmellsLike();
  Print(hejaz[1]);
  Print(hejaz[2]);
endprogram

function SmellsLike()
  var smell := array{&quot;teen&quot;, &quot;spirit&quot;};
  return smell;
endfunction</pre></div>
Prints:
<div class="doc-guide-pre">
<pre>
Teen
Spirit</pre></div>
<br/>

<p><b><a name="chap4b">Including code from other files</a></b></p>

<p>Up to this point, all the code we've been messing with has been in one file.
To import in code from other files (special files called "include files" whose
file extension is ".inc". They're special because they cannot contain a
"program" main function, only any number of normal functions), near the top of
your script, before any variable declarations or functions add the line:</p>

<div class="doc-guide-pre">
<pre>include "filename_without_.inc";</pre></div>

<p>Then you may call any function that is defined in that include file from your
script (.src) file. Note that any variables in the include files that are
declared at global scope also are at global scope in your script file. This
means you must be careful that you don't try to declare any variables with the
same names as those other global variables. If you do, it will result in
compiler errors that may be difficult to track down.</p>

<p>The above line will work if the include file is in the same directory as the
source file that includes it. If you want to use some of the more standard
include files (found in /scripts/include), you'd use the syntax:</p>

<div class="doc-guide-pre">
<pre>include "include/filename";</pre></div>

<p>If you want to include a file that is in a package you would use</p>

<div class="doc-guide-pre">
<pre>include ":pkgName:filename";</pre></div>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap5">Chapter 5</a></h3>
<br/>
<p>REJOICE! Now for UO and POL stuff: *.em files and functions</p>

<p>I'm glad you stuck through all that syntax discussion and didn't skip ahead
to here. If you did skip ahead, good luck, because I won't be reviewing any of
that syntax.</p>

<br/>

<p><b><a name="chap5a">Scripts</a></b></p>

<p>Okay, this is where we start learning to write real scripts for the UO-POL
environment. Up to this point, you can write scripts that do stuff, but probably
not anything useful. To be able to write really useful scripts that do
something, you need to be able to access data about the game world. POL provides
this interface through a number of "core-functions" that are defined in the
".em" files that can be found in your /pol/scripts directory. Note *.em files
are often referred to as 'modules'.</p>

<p>But first, I need to talk about the types of scripts in the POL environment,
when they are executed and with what parameters. Here is a quick list (note the
parameter names I use like 'clicker' are only an example, the actual parameter
names can be anything, but I'm trying to be descriptive. Only the order of the
parameters below is strictly enforced):</p>

<p>CharRef = Reference to character running the script<br>MobRef = Reference to
a mobile object (NPC, character)<br>ObjRef = Reference to an item</p>

<p>Use Scripts: Run when the associated item is double clicked in the UO
window.<br>&nbsp;&nbsp;Parameters: 1: CharRef clicker , 2: ObjRef
item_clicked</p>

<p>Walk-on Scripts: Run when a player walks on the associated
item.<br>&nbsp;&nbsp;Parameters: 1: CharRef walker , 2: ObjRef
item_walked_over</p>

<p>Text Command Scripts: Run when the player uses the command ".scriptname" and
has the appropriate privileges.<br>&nbsp;&nbsp;Parameters: 1: CharRef speaker ,
2: Text after the command. i.e. ".command blah" typed would give the text as
"blah"</p>

<p>Spell Scripts: Run when the player starts a spell from the
spellbook<br>&nbsp;&nbsp;Parameters: 1: CharRef caster</p>

<p>Control Scripts: Run on creation of the associated item and on server
reboot<br>&nbsp;&nbsp;Parameters: 1: ObjRef item_under_control</p>

<p>AI Scripts: Run on an NPC, controls behavior, should never
exit<br>&nbsp;&nbsp;Parameters: 1: MobRef the_NPC</p>

<p>Skill Scripts: Run when a player clicks on a skill gem in the skills scroll
gump<br>&nbsp;&nbsp;Parameters: 1: CharRef player_performing_skill</p>

<p>Let's start learning with Text Command scripts since they're the easiest to
get running. Text commands are only run if the player speaking the command has
the necessary command level. Make sure the player you're testing with has a
command level of "gm" (usually 4) or greater. We'll be placing all our scripts in the
/scripts/textcmd/gm directory.</p>

<p>Okay, let's start with a simple script. Say you want to write a script that
will broadcast a message to all players on the server. Here is the script
"bcast.src", I will explain each line:</p>

<div class="doc-guide-pre">
<pre>
/*1*/ use uo;
/*2*/
/*3*/ program MyBroadcast(speaker, text)
/*4*/
/*4*/   foreach character in ( EnumerateOnlineCharacters() )
/*5*/     SendSysmessage(character,text);
/*6*/   endforeach
/*7*/
/*9*/ endprogram</pre></div>

<p>Line 1: the "use" keyword tells the compiler to look in the following .em
file for the definitions of any functions we may use out of that "module" (look
there now, you'll see EnumerateOnlineCharacters and SendSysmessage).</p>

<p>Line 3: Since this is a text command script, POL automatically passes a
reference to the character speaking the command, and the text after the command.
These are here as "speaker" and "text" respectively.</p>

<p>Line 4: The EnumerateOnlineCharacters function returns an array of Character
References. for each character logged into the server. The foreach loop gets the
next data element from this returned array and sticks it in the "character"
variable. </p>

<p>Line 5: The SendSysmessage function will send the "text" string to the lower
left hand corner of "character"'s UO window. </p>

<p>Line 6: Signals the end of the foreach block</p>

<p>Line 9: Signals the end of the function.</p>

<p>Here's how it goes in a sample run:</p>

<p>1. The GM player types in <i>.bcast Howdy Everyone!<br>
</i>2. POL looks in the bcast script for a "program" function and passes
a reference to the GM player as well as the text "Howdy Everyone!".<br>3.
EnumerateOnlineCharacters returns an array of 3 character references.<br>4.
SendSysmessage will send the text "Howdy Everyone!" to the lower-left-hand
corner of each of the 3 character's screens.</p>

<p>But now the GM doesn't like the fact the he receives his own broadcast. You
can edit the script so this won't happen by checking to see if the character
reference stored in the "character" variable is the same as the "speaker"
reference passed to the function by POL:</p>

<div class="doc-guide-pre">
<pre>
/*1*/ use uo;
/*2*/
/*3*/ program MyBroadcast(speaker, text)
/*4*/
/*5*/   foreach character in ( EnumerateOnlineCharacters() )
/*6*/     if ( character!= speaker )
/*7*/       SendSysmessage(character,text);
/*8*/     endif
/*9*/   endforeach
/*10*/
/*11*/ endprogram</pre></div>

<p>Now the message won't be sent to who sent it.</p>

<br/>

<p><b><a name="chap5b">Compiling</a></b></p>

<p>Alright, it's now time to get an actual running script. <i>POL cannot
directly read the source code you write,</i> it must be compiled into a readable format
before it can be executed. <br>
Note: This also means that if you chance a .inc file, you must compile all *.src
files that use it to update them.<br>
<br>
The included program "ecompile" is used for just
this. I will assume you have experience in the command shell that your operating
system of choice uses. <br>
I will use the Windows command line for reference.</p>

<p>First, take that code above and place it in a file called "bcast.src" and
place that file in the pol/scripts/textcmd/gm directory. Now go to your command
shell and change the directory to the /pol/scripts directory. Type "ecompile
/?". You should see it give you a list of usage options.</p>

<p>We don't need any of these fancy flags, just the basic usage of ecompile:
"ecompile <filename.src>"</filename.src></p>

<p>We need to tell ecompile where the file we want to compile is so at your
command line, type:</p>

<div class="doc-guide-pre">
<pre>ecompile.exe txtcmd/gm/bcast.src</pre></div>

<p>Since that's where we put the file. You should see something like:</p>

<div class="doc-guide-pre">
<pre>
EScript Compiler v1.05<br>Copyright (C) 1994-2006 Eric N.Swanson
Compiling: D:\pol\scripts\textcmd\gm\bcast.src
Writing: D:\pol\scripts\textcmd\gm\bcast.ecl</pre></div>

<p>When ecompile tells you that it's writing a .ecl file, that means the
compiling was successful and the script can now be run.</p>

<p>To run, make sure POL is
running and you're logged in with a GM character (or higher, like admin). Now
type .bcast <message>. Now, if you did everything right up to here, you should
see nothing. That's because we made it so you wouldn't receive your own
broadcast. So to test, get a couple friends on your server and ask them if they
got the message. If they did, congrats, your very first script.</message></p>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap6">Chapter 6</a></h3>
<br/>
<p><b>Built-in Properties and the POL Object Reference</b></p>

<p>Everything in POL is an object: characters, accounts, items, corpses, NPCs,
etc. </p>

<p>POL uses a class hierarchy to allow inherited properties between objects.
For example, in the UO world, everything in the world has an x, y, and z
coordinate property. This include both items on the ground, and mobiles (NPCs or
characters). You can see from the Object Reference Chart (Appendix A) that both
the Mobile object and Item object are children of the UObject Class. Notice in
the UObject class that there are a number of properties, including x y z,
serial, objtype, color, etc. These properties are inherited by all the other
class objects under UObject in the tree, thereby implicitly granting all these
basic properties to all objects in the UO world. </p>

<p>Some Classes even have functions as members (methods), an object-oriented type of
design, i.e. the Door class has the method door.Open() and door.Close() which
open and close the door, respectively. Notice how you access these methods or
"member functions" with the dot "<b>.</b>" operator. So depending on what type of
object you are working with, some members and methods may not exist to access. For example,
a door object would not have the "quality" member an Equipment object would
have, but you could access both their x-coordinates with object.x. Note that
some properties are read-only and some are read-write. The read-only props only
allow you to retrieve the value for that property, and not to change it.
Read-write props grant you all access to the property. It is for security
purposes that some built-in props are read-only. It would mess things up if you
were able to write to character.dead or character.acct (account)!</p>

<p>See the POL documenation for the object heirarchy for more information about
built-in properties and methods.</p>

<p>Please refer to the POL Object Reference Chart for all the details.</p>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap7">Chapter 7</a></h3>
<br/>
<p><b><a name="chap7a">Much Ado about CProps and Stupid Chapter Names</a></b></p>

<p>CProps (Custom Properties) are what makes scripting with eScript so flexible.
In addition to all the built-in properties in Appendix A, CProps let you store
arbitrary amounts of arbitrary data to any object. This data can be stored by
any script and recalled by any script that has a reference to that object. You
can store any type of data: string, integer, real, arrays. You should be able to
see that this ability to store and retrieve arbitrary data is a powerful one.
I'll go through an example of a couple scripts that use CProps and touch on
other aspects of scripting.</p>

<p>Let's say you want to create a one-time use item that allows the player to
teleport back to their corpse after being resurrected. There are several things
that we need to do to make this happen: create the description for the custom
item that grants this ability, the script that controls the behavior of the
item, code to store the game coordinates of the player's corpse. First, a quick
lesson on creating a custom item:</p>

<p>Open up any itemdesc.cfg file you find. This file contains the definitions
for all the items that do something in the world. We need to add our item to one
of these files. For now, we'll use the file /config/itemdesc.cfg. Here's the
definition of our item and the descriptions of each parameter:</p>

<div class="doc-guide-pre">
<pre>
Item 0xABCD
{
  Name    ring_of_returning
  Desc    Ring of Returning
  Graphic 0x108A
  Script  return_ring
}
</pre></div>

<p>Item 0xABCD : <br>Starts the definition of the new Item. The objtype number
is arbitrary, but must be unique. The range of numbers available for custom
items are 0x5000 to 0xFFFF.</p>

<p>Name&nbsp;&nbsp;ring_of_returning : <br>his is the internal name of the item,
so you could create one using the command ".create ring_of_returning" instead of
".create 0xABCD".</p>

<p>Desc&nbsp;&nbsp;Ring of Returning<br>This is the single-click description
shown when you click it.</p>

<p>Graphic&nbsp;&nbsp;0x108A<br>This is the objtype of the art item to use to
represent this item. I got this number by looking in the InsideUO program for
the ring graphic and using it's objtype number.</p>

<p>Script&nbsp;&nbsp;return_ring<br>This is the name of the script that will run
when you double-click the item.</p>

<p>A short description of the CProp functions which we will be using all of,
found in the uo.em module header file:</p>

<div class="doc-guide-pre">
<pre>
SetObjProperty(object, property_name_string, property_value);</pre></div>

<p>Stores a CProp on "object" of name "property_name_string" and value
"property_value".</p>

<div class="doc-guide-pre">
<pre>GetObjProperty(object, property_name_string);</pre></div>

<p>Returns the value of the CProp "property_name_string" that is stored on
"object". If that CProp does not exist, returns an error.</p>

<div class="doc-guide-pre">
<pre>EraseObjProperty(object, property_name_string);</pre></div>

<p>Erases the CProp "property_name_string" that is stored on "object". If that
CProp was not found, returns an error.</p>

<p>Now, let's write some code that will store the player's position after s/he
dies. The script that is run when a player dies is /scripts/misc/chrdeath.src.
Let's look a bit at that one. There is probably a bunch of code already in there
used for other purposes, but you can ignore that for now. We see that the
function parameters looks like this:</p>

<div class="doc-guide-pre">
<pre>program ChrDeath(corpse, ghost)</pre></div>

<p>The 'corpse' parameter is an itemref to the corpse of the dead player.
Remember that an Item object inherits all the properties of the UObject class,
which includes the x,y,z coordinates. We access these by using the 'dot'
operator. We want to store the x,y,z coordinates of that corpse item on the
player so the ring item script we will create can read it. The "ghost" parameter
here is a mobileref to the character that just died. It doesn't matter that he's
a ghost, we can still access and store data as normal. In chrdeath.src, you
would add:</p>

<div class="doc-guide-pre">
<pre>
SetObjProperty(ghost, "x_corpse", corpse.x);
SetObjProperty(ghost, "y_corpse", corpse.y);
SetObjProperty(ghost, "z_corpse", corpse.z);</pre></div>

<p>Okay, now everything is in place for us to write the ring of returning
script. We create a file named "return_ring.src" in the /scripts/items
directory. Here is the basic flow of the program:</p>

<ol>
  <li>Check to see if those three cprops exist on the player using the ring.
  <ul>
    <li>if the cprops exist, store the values in three variables.
    </li><li>if they don't exist, exit with an error message</li></ul>
  </li><li>Move the player to those coordinates
  </li><li>Erase the cprops on the player
  </li><li>Destroy the ring item.</li>
</ol>

<p>Here's the code:</p>

<div class="doc-guide-pre">
<pre>
use uo;
program ReturnRing(player, ring)
// remember this is a usescript, and the variables 'player' and 'ring'
// are the parameters passed to the script by the POL core.
  var x,y,z;

  x := GetObjProperty(player,"x_corpse");
  y := GetObjProperty(player, "y_corpse");
  z := GetObjProperty(player, "z_corpse");

  if ( (x== error) or (y == error) or (z == error) )
    SendSysMessage(player,&quot;Could not find your corpse&quot;);
    return 0; //exits the script
  endif


  // Note: Since pol97 MoveCharacterToLocation is depreciated use MoveObjectToLocation instead
  MoveCharacterToLocation(player,x, y, z);
  EraseObjProperty(player, "x_corpse");
  EraseObjProperty(player, "y_corpse");
  EraseObjProperty(player, "z_corpse");
  DestroyItem(ring);
endprogram</pre></div>

<p>Easy stuff right? =) Now you'd just have to compile chrdeath.src and
return_ring.src, unload chrdeath if your server was already running (.unload
chrdeath), create the new item and test it all out. Now, you might be thinking,
"If the player was wearing this ring when he died, it would go on his corpse and
wouldn't help him getting back there." You'd be right, so there's a few things
you could do, if you wanted:</p>

<ol>
  <li>Tell the player how stupid s/he is for not leaving it in his/her bank.
  Then again, he s/he found a wandering healer, the ring would do no good in the
  bank.
  </li><li>Make the ring a "newbie" item (stays on the player's ghost, would be
  usable after he resurrected).
  </li><li>Change the item to be something non-wearable, to hint to the player that
  it shouldn't be something to carry around.</li>
</ol>

<p>It is good to know how POL stored cprops on objects. If you use the ".props"
command on an object like our unfortunate player, you'd see something like:
<code><nobr>cprop x_corpse i300</nobr></code> the important part here is the 'i'
before the value. This denotes the type of the data stored. We will revisit this
next chapter.</p>

<br/>

<p><b><a name="chap7b">Cprops on accounts</a></b></p>

<p>You can also apply properties to accounts themselves (rather than specific
characters in them), but you use a different syntax to do so. Rather than
SetObjProperty(account, "Propname", value), you use these methods to set and
read custom properties:</p>

<div class="doc-guide-pre">
<pre>
account.SetProp("PropName",value);
account.GetProp("PropName");</pre></div>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap8">Chapter 8</a></h3>
<br/>
<p><b>Config File Usage and Access</b></p>

<p>Configuration files (cfg files) in POL hold static data that is read at
run-time and may be edited on the fly without any restarting or recompiling.
Because of this, they are good places to put data that may be changed often, or
for the ease of customization as you wouldn't have to edit and recompile the
script that uses it. There are two types of cfg files in POL: ones used directly
by the core executable, and ones that are read in by scripts. The former we
won't go into, but some examples include the system configuration, pol.cfg;
spell configurations, spells.cfg; skill configurations, attributes.cfg. There are
some cfg files that are used both by the core and other scripts, such as
itemdesc.cfg and npcdesc.cfg (both are read by their respective Create core
functions, but other scripts can access them for additional data, as we will
see).</p>Config files have several pieces to them which you should be familiar
with. A configuration file consists of zero or more elements, each element has a
type, a key, and zero or more properties. An example follows with the parts
labeled:

<p></p>

<div class="doc-guide-pre">
<pre>
BowcraftData 0x13B2
{
  Name Bow
  Material 16
  Difficulty 30
  PointValue 20
}</pre></div>

<p>BowcraftData is the element type. It is not used by the system and only
serves to give the scripter an idea about what the element is related to.</p>

<p>0x13B2 is the element key or simply key. This must be a string (or number)
which is used to find the element of interest in a file with many elements like
the one shown above.</p>

<p>Name Bow<br>Material 16<br>Difficulty 30<br>PointValue 20 <br>
These above, are all the
element's properties. This is where the data you are interested lives.</p>

<p>So the steps to find a specific piece of data is as follows (must use
cfgfile.em):</p>

<ol>
  <li>Read in the config file where the element is saved using
  ReadConfigFile(filename).
  </li><li>Find the specific element where the data lives by matching its key using
  <br>FindConfigElem(cfgfile, key).
  </li><li>Read in the data much like you would a cprop using
  GetConfigXXXX(elem, propname), where <br>XXXX depends if the data is integer,
  real, or string.</li>
</ol>

<p>As a quick example, let's suppose that element above is in a file called
"bowery.cfg" (in the /config directory) and we want the value of the "Material"
property of a normal bow.</p>

<div class="doc-guide-pre">
<pre>
use cfgfile;

function GetMaterial(item)
  // assume that 'item' is an itemref to a normal bow item
  var cfgfile, element,propvalue;
  cfgfile := ReadConfigFile(&quot;bowery&quot;);
  element := FindConfigElem(cfgfile, item.objtype);
  propvalue := GetConfigInt(element,&quot;material&quot;);
endfunction</pre></div>

<p>What you put in a config file is up to you, but they are best suited for
large amounts of data that differs depending on the element key. The key is
often an objtype, or just sequentially numbered, or structured however you want.
</p>

<p>For config files that are used by both the core and other scripts, there are
properties that are expected to exist for both parts. For example, in an NPC
description, the core expects the normal properties of an NPC, like stats,
color, graphic, etc. You can also add other properties that are not read by the
core, but perhaps are read by the AI script to direct it exactly how to behave
(for example, to run away from players or to attack). There is a third option,
you can add CProps to config files. Look at this example of an NPC template:</p>

<div class="doc-guide-pre">
<pre>
NpcTemplate shade
{
  Name a shade
  script killpcs
  ObjType 0x1a
  Color 0
.
.
.
  lootgroup 29
  Magicitemchance 1
  provoke 67
  CProp Undead i1
}</pre></div>

<p>Please note:</p>

<ol>
  <li>'shade' is the key for this element. If you have a mobileref to a shade,
  you could get its element in the npcdesc.cfg file by using shade.npctemplate
  to match the key.
  </li><li>The first four properties are used by the core to create the npc.
  </li><li>The next three properties are not used by the core, but other
  scripts. The first two are used by some loot creation script, the last by some provocation
  script.
  </li><li>The last property is a CProp which is added to every instance of this NPC
  template.</li>
</ol>

<p>Number 4 is very important; if you define CProps in a config file like
npcdesc.cfg or itemdesc.cfg, that CProp is automatically set on every instance
of that item that is created. This brings up the age-old dilemma of "Space
versus Time". CProps in config files are set on every one of those items, which
takes up extra memory, but is faster to access. Reading the same data from a
config file takes a little more time to do, but the data is stored in only one place
instead of many. Using CProps in a file with those bowery entries, for example,
would not do anything, since no item is created from those elements.</p>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap9">Chapter 9</a></h3>
<br/>
<p><b>Packages</b></p>

<p>You'll notice that up until now we've been putting files in the 'standard'
places, like /scripts/items for itemuse scripts, /config for config files, etc.
That's considered poor practice as it makes upgrading very difficult. To try to
help that problem, POL uses a package system where all the files to do a
specific purpose can be placed in a directory named for that purpose. For
example, if you wrote a system for a new skill, you could place all the files
needed for that new skill in a package: all the script source files, the
compiled source files, support config files, readme files, etc. In POL, there
are two types of packages, the 'standard' packages which are enabled by default
(in/pkg or, in some shards and older POL Distros, /pkg/std/), which are normal skill systems, spawner, spells, etc. Then there
are the 'optional' packages which are off by default, but can be used if desired
(in /pkg/optional or, in some shards and older POL Distros, /pkg/opt/). Normally, enabling an optional package requires some
instructions which are normally supplied with the package.</p>

<p>Each package must include a package descriptor file, pkg.cfg, which has the
following format (note # denotes a comment):</p>

<div class="doc-guide-pre">
<pre>
# Example package definition file

Enabled 1
  #Enabled 0/1 Should this package be enabled?

Nam template
  # Name of package,should match directory name

Version 1.3
  # Version v0.v1..vn  Version number for this package

Requires spawner 1.2
  # Requires pkgname {version}
  #   Other package(s) other than this one are required
  #   in order to function
  #   More than one of these can occur.
  #   Format: requires package-name {version)

Conflicts some-package
  # Conflicts pkgname
  # This package cannot co-exist with a specific package.
  # Note that version cannot be specified

### Everything below this line is currently ignored, but are a very
### good idea to include in your pkg.cfg for imformation to users


CoreRequired 96
  #CoreRequired ver note no leading '0', which would indicate octal

Maintainer John Q. Public
  # Your name
 Email johnq@public.com
  # Your email</pre></div>

<p>In the previous examples that use the ReadConfigFile function, the addition
of packages complicate things somewhat. How can you know if you want to read the
itemdesc.cfg in a specific package, the standard one in /config, or all of the
files combined into one (for easy searching) ? This is handled in the filename
format you pass to the ReadConfigFile function. The formats are:</p>

<div class="doc-guide-pre">
<pre>
ReadConfigFile(&quot;cfgname&quot;)
  - for a script not in a package, looks for /config/[cfgname].cfg
  - for a script in a package, looks for [cfgname].cfg in the same package

ReadConfigFile(&quot;:*:cfgname&quot;)
  - Reads in every config file that matches that name.

ReadConfigFile(":pkgname:cfgname")
  - looks for [pkgdir]/[cfgname].cfg

ReadConfigFile("::cfgname")
  - always looks in pol/config/[cfgname].cfg</pre></div>

<p>Note for the special files itemdesc.cfg, npcdesc.cfg, attributes.cfg, and
spells.cfg, the first of the three formats will return the "composite" config
file that includes the concatenation of the contents of all the files in all
enabled packages and the standard file.</p>

<p>There are a few other times this format is used, such as the start_script()
and the UnloadConfigFile functions. This is why the syntax for the .unloadcfg
command is ".unloadcfg :pkgname:cfgname". That brings up another point: config
files are cached by the system and must be unloaded for any online change to be
seen.</p>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap10">Chapter 10</a></h3>
<br/>
<p><b>Debugging</b></p>

<p>Many books have been written on the subject of finding and fixing software
errors and it would be wasteful to repeat their content. This chapter will show
some POL/eScript specific ways of finding bugs in your scripts. Let's first look
at compile-time scripts. Ecompile, the eScript compiler does well at giving you
good hints to where your script's syntax errors are. I say hints because of one
of the rules of programming: never trust the compiler's error messages. They are
often very useful, but often can lead you astray if you take their messages as
gospel. The error message will contain a line number around where the error is.
It may be above, on or below that line. The rest of the error message is usually
correct, if not sometimes somewhat vague. Here are some examples:</p>

<div class="doc-guide-pre">
<pre>
Don't know what to do with Unknown Token: (280,8,'elseif') in SmartParser::parseToken
Error compiling statement at D:\pd\pol\scripts\items\torch.src, Line 4</pre></div>
- this error was from a

missing semicolon in an if-block on line 5</p>

<div class="doc-guide-pre">
<pre>
Warning: Equals test result ignored. Did you mean := for assign?
near: item.graphic = 0xa12;
File: D:\pd\pol\scripts\items\torch.src, Line 5</pre></div>
- ecompile guesses right

here: we used an equals test when we wanted an assign statement</p>

<div class="doc-guide-pre">
<pre>
Warning! possible incorrect assignment.
Near: if(item.graphic := 0x0f64)</pre></div>

<p>- ecompile catches this too, normally you wouldn't want to do
an assignment in an if-condition. It is not an illegal statement, so ecompile
completes the compile but warns you about it.</p>

<div class="doc-guide-pre">
<pre>
Unhandled reserved word: 'endprogram'
Error compiling statement at D:\pd\pol\scripts\items\torch.src, Line 7
Error in IF statement starting at File: D:\pd\pol\scripts\items\torch.src, Line 4</pre></div>

<p>- this error was we
forgot an 'endif' keyword. The compile ran into the 'endprogram' keyword before
'endif', which is a syntax error.</p>

<div class="doc-guide-pre">
<pre>

Token 'item' cannot follow token ')'
Error compiling statement at D:\pd\pol\scripts\items\torch.src, Line 2</pre></div>

<p>- this error was from an
unmatched open-parenthesis in an if statement (on line 4)</p>

<div class="doc-guide-pre">
<pre>
Error compiling statement at D:\pd\pol\scripts\items\torch.src, Line 2
Error detected in program body.</pre></div>

<p>- this is a nasty one, because it
does not give you any idea what is wrong. I've only come across this error when
a variable is declared with the same name as a keyword, in this case, 'for'.</p>

<p>Run-Time errors are much harder to find. These are errors that are
syntactically correct, but produce incorrect results. The compiler will not
catch these, nor will it help you find them. Your best bet for finding these
errors is to notice where and when the script behaves incorrectly and go to that
portion of the source code and poke around. To nail down the specific problem,
you have several ways to go about it:</p>

<p>1. print() variables that you think are in doubt every now and then. Narrow
down the exact spot where the bug is.<br>2. Many core functions return 'error'
if something went wrong (i.e. tried to create an NPC in an illegal location). If
you test the return value of these functions against 'error', you can catch them
(i.e. if(CreateNPCAtLocation == error)...)<br>3. Some functions also return an
'.errortext' member that gives you additional information. Such as:</p>

<div class="doc-guide-pre">
<pre>
var house := CreateMultiAtLocation(...);
if(house == error)
  Print(house.errortext);
endif</pre></div>

<p>4. If none of these work for you, you could build the script with debug
turned on. What this does is POL will print each line of code as it executes to
the console. This is not always helpful, because if many instances of that
script are running, they'll all print to the console. Best bet with this method
is to debug on a local server where a minimum of other scripts are running. To
turn on debug mode, make sure you 'use os;' before you add the line</p>

<div class="doc-guide-pre">
<pre>set_debug(1);</pre></div>

<p>under the 'use' lines at the top of your function. Then, you need to tell
ecompile to compile with debug on. Do this with the '-i' switch. I.e. &gt;
ecompile &#8211;i test.src .</p>

<br/>

<!-- ====================================================================== -->

<h3><a name="chap11">Chapter 11</a></h3>
<br/>
<p><b>Advanced Data Types and Functions</b></p>

<p>On the variable side, in this chapter we'll go into more detail on structs,
describe dictionaries and error types, explain the concept of persistance, and
define global properties. On the functions side, we'll describe pass by
reference and use of the "parms" array to get past limitations on what can be
passed.</p>

<br/>

<p><b><a name="chap11a">Global Properties</a></b></p>

<p>By now, you're familiar with CProps- custom properties. You're an old hand at
applying them to items, to people, heck, even to accounts. But, what if you
don't want to apply it to anything at all? Is there a way to save information
and let it just float in space, accessible from everywhere?</p>

<p>Yes, you can, and to do so you use global properties, otherwise known as
GProps. You manipulate them in much the same way as you do CProps, except you
don't need to tell it where to go. These are the three fundamental functions
you'll need to use GProps:</p>

<div class="doc-guide-pre">
<pre>
GetGlobalProperty("PropertyName");
SetGlobalProperty("PropertyName","PropertyValue");
EraseGlobalProperty("PropertyName");</pre></div>

<p>And as always, you can use variables to define the name or value of the
Global Property.</p>

<div class="doc-guide-pre">
<pre>
var globname := "Prop1";
SetGlobalProperty(globname ,"No");</pre></div>

<p>The above code would set the value "No" to the global property "Prop1". You
can also but more than just simple variables as the value, for both GProps and
CProps. Here's an example:</p>


<div class="doc-guide-pre">
<pre>
var propname := "Property1";
var propvalue := array{0 , 1 , "two"};
SetGlobalProperty(propname, propvalue);</pre></div>

<p>And viola, Property1 now has as its value an array containing 0, 1, and
"two". Later, if you were to do the following:</p>

<div class="doc-guide-pre">
<pre>
var ourprop := GetGlobalProperty("Property1");
Print(outprop[1]);</pre></div>

<p>the output would be "0". This also works for CProps.</p>

<p>So now we have an interesting question- if simple variables and arrays can
both be stored in props, what else can? To answer that question, we'll take a
look at the concept of <b>persistance</b>.</p>

<p>If a variable type can be stored in a prop, it is said to be Persistable. As
of POL090, there are no data types that cannot be persisted, so this section is
mostly so you'll know what was meant when you read old changes.txts and see
notices that certain things can now be persisted. In POL089, there was one type
of data that could not be persisted- the mysterious ERROR type. This was the
type you got if you tried to go something that should have returned a value but
failed- instead you got <error>or <uninitialized object="">or somesuch. This caused
a problem if you then tried to set the variable that just got assigned an error
as a CProp- it wouldn't stick. It wouldn't persist. Nowadays, error types do
persist, and they do so as though they were a struct.</uninitialized></error></p>

<p>I'll go into a little more detail on data types you've already seen, now.
</p>

<br/>

<p><b><a name="chap11b">Arrays</a></b></p>
<p>The prefered way to initialize an empty array is to use:</p>

<div class="doc-guide-pre">
<pre>var a := array;</pre></div>

<p>This works the same as </p>

<div class="doc-guide-pre">
<pre>var a := {};</pre></div>

<p>used to. </p>

<p>Arrays have a couple of methods that go with them- functions you can use to
manipulate them directly. They are:</p>

<p>array.Size() - This returns the number of elements in the
array.<br>array.Insert(index , value) - inserts a new element, value, at the
specified index.<br>array.Erase(index) - deletes the element with the
specified index.<br>array.Shrink(nelems) - erases all but the first nelems
elements in the array.<br>array.Append(value) - adds this element to the end
of the array.<br>array.Reverse() - reverses the order of the
array.<br>array.Sort() - sorts the array.</p>

<p>Here's an example:</p>

<div class="doc-guide-pre">
<pre>
1  var colours := array;
2  colours[1] := "green";
3  colours[2] := "blue";
4  var csize := colours.Size();
5  colours[5] := "shiny";
6  csize := colours.Size();</pre></div>

<p>So, let's take this step by step. Line 1 defines the array. Lines 2 and 3 put
the first elements into it. At line 4, the variables look like this:</p>

<div class="doc-guide-pre">
<pre>
  colours[1] = "green"
  colours[2] = "blue"
  csize = 2
</pre></div>

<p>Line 5 adds a new elements at position 5, which EScript handles just fine
because it's cool like that. So what do the variables look like as of line 6?</p>

<div class="doc-guide-pre">
<pre>
  colours[1] = "green"
  colours[2] = "blue"
  colours[3] = <uninitialized object>    <--- this is one of those error types.
  colours[4] = <uninitialized object>
  colours[5] = "shiny"
  csize = 5
</pre></div>

<p>Note that when it counts how many things are in the array, it includes the
"empty" spaces! In other words, array.Size() could more accurately be said to
return the last valid array index. I'll also point out that if you attempt to
look at an array position off the end of the array (colours[10], for instance)
it will also return &lt;uninitialized object&gt; on you.</p>

<p>Now, let's continue the previous example.</p>

<div class="doc-guide-pre">
<pre>
7 colours.Insert(3, "bronze");
8 csize := colours.Size();</pre></div>

<p>What happens here? We're inserting the value "bronze" at position 3 in the
array. Here's what the array looks like after that:</p>

<div class="doc-guide-pre">
<pre>
colours[1] = "green"
colours[2] = "blue"
colours[3] = "bronze"
colours[4] = <uninitialized object>
colours[5] = <uninitialized object>
colours[6] = "shiny"
csize = 6
</pre></div>

<p>Note that an insert pushes the remaining elements down one slot, EVEN IF it
is inserted into a position that was empty. If you don't want to push like that,
don't use insert, just use an assignment: colours[3] := "bronze".</p>

<div class="doc-guide-pre">
<pre>
9 colours.Erase(3);</pre></div>

<p>This will return the array to what it looked like before the insert.</p>

<div class="doc-guide-pre">
<pre>10 colours.Erase(3);</pre></div>

<p>This removes one of the empty slots in the array. Using the erase method
pulls everything up one slot- just the opposite of insert. So after line 10, we
have:</p>

<div class="doc-guide-pre">
<pre>
colours[1] = "green"
colours[2] = "blue"
colours[3] = <uninitialized object>
colours[4] = "shiny"
csize = 6
</pre></div>

<div class="doc-guide-pre">
<pre>11 colours.shrink(3);</pre></div>

<p>This will reduce the array to the first 3 elements- in this case,

<div class="doc-guide-pre">
<pre>
colours[1] = "green"
colours[2] = "blue"
colours[3] = <uninitialized object>
</pre></div>

<div class="doc-guide-pre">
<pre>
12 colours.Append("brown");
13 colours.Reverse();</pre></div>

<p>This will end up yielding:
<div class="doc-guide-pre">
<pre>
colours[1] = "brown"
colours[2] = <uninitialized object>
colours[3] = "blue"
colours[4] = "green"
</pre></div>

<div class="doc-guide-pre">
<pre>
14 colours.shrink(2);
15 colours.reverse();</pre></div>

<div class="doc-guide-pre">
<pre>
colours[1] = <uninitialized object>
colours[2] = "brown"
</pre></div>

<p>Quite frankly, I'm not entirely sure how sort works. I'll provide two
examples from my testing and let people experiment with it on their own.</p>

<div class="doc-guide-pre">
<pre>
a := array;
a[1] := 4;
a[2] := 7;
a[3] := "show";
a[4] := "tunes";
a.Sort();</pre></div>

<p>printing out the array elements in order yielded:</p>

<div class="doc-guide-pre">
<pre>show tunes 4 7</pre></div>

<p>However...</p>

<div class="doc-guide-pre">
<pre>
a := array;
a[1] := 4;
a[2] := 7;
a[3] := "show";
a[5] := "tunes";
a.Sort();</pre></div>

<p>printing this out in order gave me:</p>

<p>printing out the array elements in order yielded:</p>

<div class="doc-guide-pre">
<pre>tunes <uninitialized object> show 4 7</pre></div>

<p>Note also that an array element can be any data type- I can hold a number
there, or a struct, or an error, or another array. Try to sort an array of
arrays at your own risk.</p>

<br/>

<p><b><a name="chap11c">A Quick Review of Structs</a></b></p>

<p>A struct is similar to an array in that it contains a collection of values
rather than just one. However, rather than an ordered list, structs are stored
in, well, a structure. To create one, first you have to initialize it:</p>

<div class="doc-guide-pre">
<pre>var a := struct;</pre></div>

<p>This lets the compiler know that it is going to be of data type struct and
treats it accordingly. The nice thing about structs is that they can be treated
like a lot of the internal objects. For instance, a player has certain elements,
like his position. These are accessed with the '.' - player.x, player.y, and
player.z, for instance. Similarly, we can assign the struct elements that are
accessed the same way. Here's an example:</p>

<div class="doc-guide-pre">
<pre>
var elevator := struct;
elevator.+floor;
elevator.+riders;
elevator.+shaft;

elevator.floor := 1;       // It's on the first floor.
elevator.riders := array;  // We'll keep an array of the people on it, using
                           // append and erase to keep track of them.
elevator.shaft := 3;       // It's in the third shaft from the left.
</pre></div>

<p>So, someone gets on and pushes 3.</p>

<div class="doc-guide-pre">
<pre>
elevator.riders.Append("John");

elevator.floor := 3;       // Order of precidence on . is left to right, so
elevator.riders.Erase(1);  // first it finds elevator.riders, sees that it is
	                         // an array, and then appends.
	                         // We've gone upstairs.
	                         // John gets off the elevator.
</pre></div>

<br/>

<p><b><a name="chap11d">Dictionaries</a></b></p>

<p>Dictionaries are similar to both structs and arrays- they are, sort of, a
bridge between the two types. You create a dictionary, unsurprisingly enough,
with this syntax:</p>

<div class="doc-guide-pre">
<pre>var thing := dictionary;</pre></div>

<p>Then, you treat it like an array except, instead of just ordered numbers, a
dictionary can contains words or numbers as its keys.</p>

<div class="doc-guide-pre">
<pre>
thing["green"] := "blue";
thing["number"] := 4;
thing[3] := array{1 , 3};</pre></div>

<p>Internally, structs are actually dictionaries- so, basically, these do the
same thing:</p>

<div class="doc-guide-pre">
<pre>
var thing1 := struct;
thing.+first;
thing.first := "one";

var thing2 := dictionary;
thing2["first"] := "one";</pre></div>

<p>In addition, it means that the dictionary methods work equally well on both.
These methods are:</p>

<p>dictionary.Size() - returns the number of elements.<br>dictionary.Erase(key) - erases an element.</p>

<table border="0" cellpadding="0" cellspacing="0">
  <tbody>
  <tr>
    <td valign="top">dictionary.Insert(key , value) - </td>
    <td>adds an element. Dictionaries are not really ordered, so it's not
      entirely accurate to say the item is "inserted".<br>This is the same as
      doing dictionary["key"] := value;</td>
  </tr>
  </tbody>
</table>

<p>dictionary.Exists(key) - returns true if there that key exists.<br>
dictionary.Keys() - returns a
list of all the keys.</p>

<p>Functions, for the most part, return one value if any. Frequently, we want to
pass back more than one piece of data, we can return an array, or struct, or
dictionary.</p>

<p>This also works the other way. The start_script <code>os</code> method only
takes two parameters- the name of the script, and one thing to pass it. What if
you want the script to take in multiple variables for information? You send it
an array, of course, and custom has it that this array ends up getting named
"parms".</p>

<p>Here's an example. Let's say you have a script, called testscript. It wants
as parameters who called it as well as two items that the user has clicked on.
If you wanted to call it from another script, you could do the following:</p>

<div class="doc-guide-pre">
<pre>start_script(&quot;testscript&quot; , array{who, target1, target2});</pre></div>

<p>or,</p>

<div class="doc-guide-pre">
<pre>
var parms := array;
parms[1] := who;
parms[2] := target1;
parms[3] := target2;
start_script("testscript" , parms);</pre></div>

<p>Then, in testscript.src:</p>

<div class="doc-guide-pre">
<pre>
program testscript(parms)
  if ( parms[2] ) //tests if it got passed an array
    char := parms[1];
    firsttarget := parms[2];
    secondtarget := parms[3];
  else
    char := parms;
  endif
  // do stuff
endprogram</pre></div>

<p>The program knows how to react whether it was simply sent a scalar or if it
was sent an array.</p>

<br/>

<p><b><a name="chap11e">Function Calls by Reference</a></b></p>

<p>The final section of this chapter will cover passing variables to functions
by reference rather than by value. What's the difference?</p>

<p>Normally, we pass variables by value. Which is to say, it sends the contents
of the original variable to the new one (copy), and the new one doesn't care anymore
about the old one. So, when we have:</p>

<div class="doc-guide-pre">
<pre>
var first := "one";
var second := 2;
FallFunction(first ,second);</pre></div>

<p>and...</p>

<div class="doc-guide-pre">
<pre>
function CallFunction (this, that)
  print(this);  // will print "one"
  print(that);  // will print "2"
  this := "green";
endfunction</pre></div>

<p>when all is said and done, the values of first and second do not change.</p>

<p>However, if we pass by REFERENCE, what we are actually sending to the
function is the location in memory of the variable itself, rather than just
sending along its contents. You do this by prefixing the variable name with
"byref" in the function declaration. So for instance:</p>

<div class="doc-guide-pre">
<pre>
var a := 4;
var b := 6;
var c := 9;
Foo(a, b, c);

function Foo(pa, byref pb, pc)
  pa := 3;
  pb := 5;
  pc := 8;
endfunction</pre></div>

<p>After the call to function foo is completed, a and c are unchanged but the
value of b has become 5, because pb is a reference to the variable b itself, not
a copy of its contents. </p>

<p>You have to be careful with this, as it is easy to change a variable you
didn't intend to, but it is very powerful. It is also more efficient than pass
by value, because it does not have to make and store a copy of the variable's
value, and then destroy it when the function is done.</p>

<p>
<br/>
<p><b><a name="chap11f">Interpolated Strings</a></b></p>
<br />
<p>Added in: POL 100.1.0</p>
<br />
<p>Interpolated strings provide a clean and concise way to construct a string
using multiple expressions. The following two statements are equivalent:</p>

<div class="doc-guide-pre"><pre>
  var a := 1, b := 2;
  Print("The sum of " + CStr(a) + " and " + CStr(b) + " is " + CStr(a + b));
</pre></div>

<div class="doc-guide-pre"><pre>
  var a := 1, b := 2;
  Print($"The sum of {a} and {b} is {a + b}");
</pre></div>

<p>Interpolated strings allow <i>any</i> expression inside them, including other
interpolated strings. This might get messy to read though!</p>

<div class="doc-guide-pre"><pre>
  Print($"My master is: {master.name ?: $"({self.name} has no master)"}");
</pre></div>

<p>Interpolated strings also support an optional formatting string that mimics
the same format used in the String.format method. Specify the formatting string
via <pre>$"{expression : format}"</pre>. The following two statements are
equivalent:</p>

<div class="doc-guide-pre"><pre>
  Print("Your graphic: " + Hex(who.graphic));
</pre></div>

<div class="doc-guide-pre"><pre>
  Print($"Your graphic: {who.graphic : #x}");
</pre></div>

<p>To use a normal brace inside an interpolated string (ie. "escape" the brace),
specify <i>two</i> of the same brace:</p>

<div class="doc-guide-pre"><pre>
  Print($"{{ hello }}"); // will print "{ hello }"
</pre></div>

<br />
<br/>

<!-- ====================================================================== -->

</p><h3><a name="app2">Appendix A: GUMP Tag Descriptions</a></h3>
<br/>

<table border="0" cellpadding="0" cellspacing="1">
  <tbody>
  <tr>
    <td width="30%"><b>Tags</b></td>
    <td width="30%"><b>Syntax/Description</b></td>
    <td width="40%">&nbsp;</td></tr>
  <tr>
    <td>----</td>
    <td>------------------</td>
    <td></td></tr>
  <tr>
    <td>noclose</td>
    <td>[NONE]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;gump can't be closed by
      clicking the right mousebutton. Selection via an exit enabled button must
      be made.</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>nomove</td>
    <td>[NONE]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The gump can't be moved
      around the screen.</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>nodispose</td>
    <td>[NONE]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The gump can't be closed
      by hitting ESC. If this setting isn't specified and the user hits ESC,
      then the dialog is closed but no message is sent to the server. The server
      never thinks the player is done with the gump. Therefore ALL gump dialogs
      should specify 'nodispose'.</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>tilepic</td>
    <td>[X] [Y] [T]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Displays a tile on the
      gump.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[X] -&gt; X coord of the
      tile.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Y] -&gt; Y coord of the
      tile.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[T] -&gt; Tile (see insideUO)</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>resizepic</td>
    <td>[X1] [Y1] [G] [X2] [Y2]</td>
    <td></td></tr>
  <tr>
    <td colspsn="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Defines the gump
      layout background.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[X1] -&gt; Start
      X coord of the gump.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Y1] -&gt;
      Start Y coord of the gump.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[G]
      -&gt; Graphics to be used as gump
      background.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Y1] -&gt; End X coord
      of the gump.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Y2] -&gt; End Y coord
      of the
      gump.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Notes:<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Be
      careful, not all gumps 'stretch' and have to be
      used<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;with the correct
      sizing.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;5100 = stretchable gray
      background gump.</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>page</td>
    <td>[N]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[N] -&gt;
      Pagenumber.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Notes:<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Defines
      the page layout. Page 0 affects all
      following<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pages (items will not be
      removed when moving to<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;other
      pages), i.e. the 'background' that is always displayed.
      <br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;page 1 is what will be displayed
      when the gump first opens (on<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;top
      of what is defined in page 0).</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>button</td>
    <td>[X] [Y] [G1] [G2] [E] [P] [R]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;X -&gt; X coord of the
      button.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Y -&gt; Y coord of the
      button.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;G1 -&gt; Normal
      graphics.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;G2 -&gt; Clicked
      graphics.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;E -&gt; Exit? (10)<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;P -&gt; Pagenumber
    (jump to page)<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;R -&gt;
      Returnvalue.<br><br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[EXIT] = button
      will make a menuexit and return a value.
      (01)<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[PAGENUMBER] = button will
      change to page #. (#)<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[RETURNVALUE]
      = Returnvalue of button. (See EXIT)
      (#)<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if you have radio buttons on
      the screen with return values,
      <br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;those return values will be used
      instead.</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>radio</td>
    <td>[X],[Y],[GUMPGFX],[CLICKGUMPGFX],[PREVALUE],[RETURNVALUE]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Only one radio button
      can be selected on a page. RETURNVALUE
      <br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;will be returned when the gump
      exits.<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(See also BUTTON).</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>tilepic</td>
    <td>[X],[Y],[TILENUMBER]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Display the selected
      tile (normal item art).</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>text</td>
    <td>[X],[Y],[COLORCODE],[STRINGNUMBER]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Display
      text<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The string numbering requires
      some thought however, UO <br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;considers
      the first string passed to it as 0 (zero) but POL considers the first one
      in an array to be 1 (one) so this is the way to do
      it:<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;text 0 0 32 0 &lt;---&gt;
      array[1] := "This I want printed".</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>gumppic</td>
    <td>[X],[Y],[GUMPGFX]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Display the selected
      gump.</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>textentry</td>
    <td>[X] [Y] [WIDTH] [HEIGHT] [COLOR] [RETURNVALUE] [INITSTRINGNUMBER]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Display an editable text
      entry line<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[RETURNVALUE] is the key
      to use to retrieve the
      string<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[INITSTRINGNUMBER] is the
      initial string to use, usual base
      <br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0&lt;&gt;1 stuff</td></tr>
  <tr>
    <td></td>
    <td>&nbsp;</td>
    <td></td></tr>
  <tr>
    <td>checkbox</td>
    <td>[X],[Y],[GUMPGFX],[CLICKGUMPGFX],[PREVALUE],[RETURNVALUE]</td>
    <td></td></tr>
  <tr>
    <td colspan="3">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;RETURNVALUE of all the
      checked boxes can be found in result.keys[].</td></tr></tbody></table>
<br/>
<br/>

<!-- ====================================================================== -->

<h3><a name="appB">Appendix B: Operator Precedence</a></h3>
<br/>

<table>
  <thead>
    <tr>
      <th>Precedence</th>
      <th>Title</th>
      <th>Symbols</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>Highest</td>
      <td>Subscript</td>
      <td><code>[]</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Member Access</td>
      <td><code>.</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Postfix</td>
      <td><code>++</code>, <code>--</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Prefix</td>
      <td><code>-</code>, <code>+</code>, <code>++</code>, <code>--</code>, <code>~</code>, <code>not</code>, <code>!</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Multiplicative, Shifts, Bitwise AND</td>
      <td><code>*</code>, <code>/</code>, <code>%</code>, <code>&lt;&lt;</code>, <code>&gt;&gt;</code>, <code>&amp;</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Additive, Bitwise OR/XOR</td>
      <td><code>+</code>, <code>-</code>, <code>|</code>, <code>^</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Elvis</td>
      <td><code>?:</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Named checks</td>
      <td><code>in</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Comparison</td>
      <td><code>&lt;</code>, <code>&gt;</code>, <code>&lt;=</code>, <code>&gt;=</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Equality</td>
      <td><code>==</code>, <code>!=</code>, <code>&lt;&gt;</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Conjunction</td>
      <td><code>&amp;&amp;</code>, <code>and</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Disjunction</td>
      <td><code>||</code>, <code>or</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Conditional Operator</td>
      <td><code>? :</code></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Member Modification/Query</td>
      <td><code>.+</code>, <code>.-</code>, <code>.?</code></td>
    </tr>
    <tr>
      <td>Lowest</td>
      <td>Assignment</td>
      <td><code>:=</code>, <code>+=</code>, <code>-=</code>, <code>*=</code>, <code>/=</code>, <code>%=</code></td>
    </tr>
  </tbody>
</table>
<br/>
<br/>

<!-- ====================================================================== -->
<h3><a name="appX">Appendix X: Revision History</a></h3>
<br/>

<p>
	v0.1: 6/27/2000<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Initial preliminary document<br>
	v0.2: 7/19/2000<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Added sections on Case, built-in props, Object Ref chart<br>
	v0.3: 7/21/2000<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Added Gump Tag descriptions, CProp Chapter, Package Chapter<br>
	v0.4: 7/22/2000<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Added Debugging Chapter<br>
	v0.5: 7/25/2000<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Cleaned up, added contact info, first public version.<br>
	v0.6: 11/01/2000<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Turned into HTML.
		Corrected an error in the CASE section.
		Added info about CProps and accounts.
		Added Chapter 11. -Madman<br>
	v0.6a: 11/06/2000<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Added Table of Contents, cell borders on some tables. -Madman<br>
	v0.6b: 15/06/2006<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Fixed some eScript comparisons and assignments.
		Removed some 'local's. Added 'do..dowhile()'. -Shinigami<br>
	v0.6c: 7/07/2006<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Removed object property/method lists,
		updated data structures and iterations and trillions of smaller things. -Austin<br>
	v0.6d: 19/02/2009<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		Reformated. -Turley<br>
	v0.6e: 12/10/2015<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Cleaned HTML code. - Bodom</p>
<p>v0.7: 07/09/2018<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Added additional information regarding AND, OR, XOR, Ones Compliment operators supplied by Yukiko<br>
</p>
<p>v0.8: 01/11/2019<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Added documentation for finding the ordinal location of an element in an array by ThisIsMe<br>
</p>
<p>v0.9: 05/01/2020<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Added documentation for Elvis Operator by Syzygy<br>
<p>v0.10: 11/03/2020<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Added documentation for Interpolated Strings by Kevin<br>
<p>v0.11: 14/03/2020<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Added documentation for Conditional Operator, Operator Precendences by Kevin<br>
</p>
