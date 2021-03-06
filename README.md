# KIV/FJP Compiler

- [Introduction](#introduction)
- [How to compile and run the application](#how-to-compile-and-run-the-application)
  * [Requirements](#requirements)
  * [Compilation](#compilation)
    + [Compilation on Linux](#compilation-on-linux)
    + [Compilation on Windows](#compilation-on-windows)
  * [Execution](#execution)
    + [Execution on Linux](#execution-on-linux)
    + [Execution on Windows](#execution-on-windows)
    + [Different options to run the application](#different-options-to-run-the-application)
- [Debug outputs of the program](#debug-outputs-of-the-program)
  * [tokens.json](#tokensjson)
  * [code.pl0](#codepl0)
  * [stacktrace.txt](#stacktracetxt)
- [Grammar](#grammar)
- [Supported features](#supported-features)
- [Descriptions & examples of implemented features](#descriptions---examples-of-implemented-features)
  * [Comments](#comments)
  * [Identifiers](#identifiers)
  * [Definition of integer/bool constants](#definition-of-integer-bool-constants)
  * [Definition of integer/bool variables](#definition-of-integer-bool-variables)
  * [Assignment](#assignment)
  * [Basic arithmetic and logic](#basic-arithmetic-and-logic)
  * [Loop (arbitrary)](#loop--arbitrary-)
  * [Simple condition (if without else)](#simple-condition--if-without-else-)
  * [Definition of a subroutine (procedure, function, method) and its call](#definition-of-a-subroutine--procedure--function--method--and-its-call)
  * [For loop](#for-loop)
  * [Do-while loop](#do-while-loop)
  * [Repeat-until loop](#repeat-until-loop)
  * [Foreach loop](#foreach-loop)
  * [Else branch of an if statement](#else-branch-of-an-if-statement)
  * [Data type boolean and logical operations with it](#data-type-boolean-and-logical-operations-with-it)
  * [Branching condition (switch, case)](#branching-condition--switch--case-)
  * [Multi-assignment (a = b = c = d = 3;)](#multi-assignment--a---b---c---d---3--)
  * [Conditional assignment / ternary operator (min = (a < b) ? a : b;)](#conditional-assignment---ternary-operator--min----a---b----a---b--)
  * [Commands for input and output (read, write - needs appropriate instructions to be used)](#commands-for-input-and-output--read--write---needs-appropriate-instructions-to-be-used-)
  * [GOTO command (watch out for remote jumps)](#goto-command--watch-out-for-remote-jumps-)
  * [Array and working with its elements](#array-and-working-with-its-elements)
  * [Instanceof operator](#instanceof-operator)
- [Project layout](#project-layout)
  * [UML diagram of the structure of the lexer](#uml-diagram-of-the-structure-of-the-lexer)
  * [UML diagram of the structure of the parser](#uml-diagram-of-the-structure-of-the-parser)
  * [UML diagram of the structure of the VirtualMachine](#uml-diagram-of-the-structure-of-the-virtualmachine)
- [List of PL0 instructions](#list-of-pl0-instructions)
- [Conclusion](#conclusion)

## Introduction

Within this module, we decided to implement a compiler for a **programming language of our own**. The syntax of our 
programming language is based off of PL0, and it may slightly resemble the C programming language. As an outcome of 
this project, we compile source code written in our programming language into an extended/customized version of the **PL0 instruction set** (https://en.wikipedia.org/wiki/PL/0). Also, in order to test the correctness of the compiler, we decided to implement a **virtual machine** that executes the compiled code, so we can see and analyze the output.

As for the programming language, in which the compiler was implemented, we decided to go with **C++** without the use of any external libraries such as Bison (https://www.gnu.org/software/bison/) or ANTRL (https://www.antlr.org/). This decision was made by the fact that we wanted to put the knowledge obtained from the lectures into practice and implement the recursive descent algorithm on our own.

## How to compile and run the application

### Requirements

Since we did not want to be limited by the Linux platform, we used the `cmake` tool (https://cmake.org/) to compile and build our application. In order to successfully build the application, you need to have `cmake` installed on your machine, whether it is Linux, Windows, or macOS. You need to make sure that you can execute the `cmake` command from your terminal. If you are on Windows, you may need to add the path to `cmake` into your environment variables.

### Compilation

#### Compilation on Linux

If you are on Linux, all you are required to do is to navigate into the root folder of the project structure and execute the following commands in the terminal.

The first command makes the file executable, and the second command builds the whole application.

```
chmod +x build.sh
```

```
./build.sh
```

#### Compilation on Windows

If you are on Windows, the process of building the application is quite similar to the Linux one. All you need to do is to navigate into the root folder of the project structure and execute the following command in the command line.

```
.\build.bat
```

### Execution

Upon successful compilation, a `build` folder should be created containing a `fjp` file, which represents the executable application.

#### Execution on Linux

```
./fjp
```

#### Execution on Windows

```
.\fjp.exe
```

After executing one of the commands listed above (depending on your operating system) you should be prompted with the following output.

```
ERR: Input file is not specified!
     Run './fjp --help'
```

If you get the error message shown above, you have successfully built the application.

#### Different options to run the application

As suggested by the error shown above, the application expects an input file to be provided by the user. However, if you do so, nothing happens (see further down below). You can use `--help` to check out what options the application supports.

```
./fjp --help
```

```
FJP compiler
Usage:
  ./fjp <input> [OPTION...]

  -d, --debug  generates the following files: tokens.json, code.pl0, 
               stacktrace.txt
  -r, --run    executes the program
  -h, --help   prints help
```

For example, if you only were to compile the code, see the output instructions, and not execute them, you would run 
the application with the `-d` option. Additionally, you could add the `-r` option in order to execute the program as well. Here are some examples of how you can run the application.

```
./fjp my-program --debug
./fjp my-program -dr
./fjp my-program -r
./fjp my-program --run --debug
```

Parsing these parameters was tacked with the help of this library https://github.com/jarro2783/cxxopts.

## Debug outputs of the program

If the application is run with the `--debug` option. The following files will be generated. As an example, consider the following piece of code written in our custom programming language (my-program).

```
START

const int N=3;
int counter;

function foo() {
    if (counter < N) {
        write(counter);
        counter := counter + 1;
        call foo();
    }
}

{
    counter := 0;
    call foo();
}

END
```
This program recursively prints out numbers 0, 1, and 2.

```
./fjp my-program -dr
```

### tokens.json

This file contains all tokens recognized and parsed from the input file. The format of the file is JSON. As far as the example is concerned, the content of the file should look like this:

```
[{
    "typeId": "48",
    "type": "START",
    "lineNumber": "2",
    "value": "START"
},
{
    "typeId": "32",
    "type": "const",
    "lineNumber": "2",
    "value": "const"
},
{
    "typeId": "33",
    "type": "int",
    "lineNumber": "2",
    "value": "int"
},
.
.
.
{
    "typeId": "16",
    "type": "}",
    "lineNumber": "18",
    "value": "}"
},
{
    "typeId": "54",
    "type": "END",
    "lineNumber": "18",
    "value": "END"
}]

```

Each record is made up of `typeId`, which represents the id of a token, `type`, which is the text representation of 
a token, `lineNumber`, which indicates the line the token was found on, and lastly, `value`, which holds the value 
of a token - used when it's a number or an identifier.

### code.pl0

The instructions into which the source code has been compiled look as shown below. The first column represents the 
address of an instruction. This is comes in handy when analyzing jump instructions, conditional jumps, function 
calls, etc. This additional information can be turned off by commenting out line 10 in `src/parser.cpp` - `#define PRINT_ADDRESSES`.

```
[#000] INC 0 5
[#001] JMP 0 16
[#002] INC 0 4
[#003] JMP 0 4
[#004] LOD 1 4
[#005] LIT 0 3
[#006] OPR 0 10
[#007] JPC 0 15
[#008] LOD 1 4
[#009] SIO 0 1
[#010] LOD 1 4
[#011] LIT 0 1
[#012] OPR 0 2
[#013] STO 1 4
[#014] CAL 1 2
[#015] OPR 0 0
[#016] LIT 0 0
[#017] STO 0 4
[#018] CAL 0 2
[#019] OPR 0 0
```

### stacktrace.txt

Lastly, if the program was also executed, the `-d` option generates a stacktrace which shows the contents of the stack as the program was being executed. This is very helpful as we get to see what the program exactly does at any given time. The stacktrace of the example program looks like this.

```
				EIP	EBP	ESP	stack
initial values			0	1	0
0	INC	0	5	1	1	5	0 0 0 0 0 
1	JMP	0	16	16	1	5	0 0 0 0 0 
16	LIT	0	0	17	1	6	0 0 0 0 0 0 
17	STO	0	4	18	1	5	0 0 0 0 0 
18	CAL	0	2	2	6	5	0 0 0 0 0 
2	INC	0	4	3	6	9	0 0 0 0 0 | 0 1 1 19 
3	JMP	0	4	4	6	9	0 0 0 0 0 | 0 1 1 19 
4	LOD	1	4	5	6	10	0 0 0 0 0 | 0 1 1 19 0 
5	LIT	0	3	6	6	11	0 0 0 0 0 | 0 1 1 19 0 3 
6	OPR	0	10	7	6	10	0 0 0 0 0 | 0 1 1 19 1 
7	JPC	0	15	8	6	9	0 0 0 0 0 | 0 1 1 19 
8	LOD	1	4	9	6	10	0 0 0 0 0 | 0 1 1 19 0 
9	SIO	0	1	10	6	9	0 0 0 0 0 | 0 1 1 19 
10	LOD	1	4	11	6	10	0 0 0 0 0 | 0 1 1 19 0 
11	LIT	0	1	12	6	11	0 0 0 0 0 | 0 1 1 19 0 1 
12	OPR	0	2	13	6	10	0 0 0 0 0 | 0 1 1 19 1 
13	STO	1	4	14	6	9	0 0 0 0 1 | 0 1 1 19 
14	CAL	1	2	2	10	9	0 0 0 0 1 | 0 1 1 19 
2	INC	0	4	3	10	13	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 
3	JMP	0	4	4	10	13	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 
4	LOD	1	4	5	10	14	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 1 
5	LIT	0	3	6	10	15	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 1 3 
6	OPR	0	10	7	10	14	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 1 
7	JPC	0	15	8	10	13	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 
8	LOD	1	4	9	10	14	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 1 
9	SIO	0	1	10	10	13	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 
10	LOD	1	4	11	10	14	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 1 
11	LIT	0	1	12	10	15	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 1 1 
12	OPR	0	2	13	10	14	0 0 0 0 1 | 0 1 1 19 | 0 1 6 15 2 
13	STO	1	4	14	10	13	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 
14	CAL	1	2	2	14	13	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 
2	INC	0	4	3	14	17	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 
3	JMP	0	4	4	14	17	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 
4	LOD	1	4	5	14	18	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 2 
5	LIT	0	3	6	14	19	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 2 3 
6	OPR	0	10	7	14	18	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 1 
7	JPC	0	15	8	14	17	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 
8	LOD	1	4	9	14	18	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 2 
9	SIO	0	1	10	14	17	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 
10	LOD	1	4	11	14	18	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 2 
11	LIT	0	1	12	14	19	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 2 1 
12	OPR	0	2	13	14	18	0 0 0 0 2 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 3 
13	STO	1	4	14	14	17	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 
14	CAL	1	2	2	18	17	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 
2	INC	0	4	3	18	21	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 | 0 1 14 15 
3	JMP	0	4	4	18	21	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 | 0 1 14 15 
4	LOD	1	4	5	18	22	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 | 0 1 14 15 3 
5	LIT	0	3	6	18	23	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 | 0 1 14 15 3 3 
6	OPR	0	10	7	18	22	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 | 0 1 14 15 0 
7	JPC	0	15	15	18	21	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 | 0 1 14 15 
15	OPR	0	0	15	14	17	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 | 0 1 10 15 
15	OPR	0	0	15	10	13	0 0 0 0 3 | 0 1 1 19 | 0 1 6 15 
15	OPR	0	0	15	6	9	0 0 0 0 3 | 0 1 1 19 
15	OPR	0	0	19	1	5	0 0 0 0 3 
19	OPR	0	0	0	0	0	
```

On the left-hand side, we can see the instruction that's currently being executed. We can also see the content of registers `EIP`, `EBP`, and `ESP`. On the-right hand side, we can see the current content of the stack. Every function call (every frame) is separated by the `|` symbol.

## Grammar

A more formal way to define how you should write a program in our programming language could be seen below.

```
<program> --> 'START' <block> 'END'
<block> --> (<const>* <var>* <function>*) <statement>


<const> --> <int_const> | <bool_const>
<int_const> --> <int_const_decl> (',' <int_const_decl2>)* ';'
<int_var_decl> --> 'int' <ident> '=' <num>
<int_const_decl2> --> <ident> '=' <num>
<bool_const> --> <bool_const_decl> (',' <bool_const_decl2>)* ';'
<bool_var_decl> --> 'bool' <ident> '=' <boo_val>
<bool_const_decl2> --> <ident> '=' <boo_val>
<boo_val> --> 'true' | 'false'

<var> --> <int_var> | <bool_var> 
<int_var> --> <int_var_decl> (',' <int_var_decl2>)* ';'
<int_var_decl> --> 'int' <ident> | 'int' <ident> '[' <num>  | <ident_const> ']' ( '=' '{' <num> (',' <num>)* '}')?
<int_var_decl2> --> <ident> | <ident>'['<num> | <ident_const> ']'
<bool_var> --> <bool_var_decl> (',' <bool_var_decl2>)* ';'
<int_var_decl> --> 'bool' <ident> | 'bool' <ident> '[' <num>  | <ident_const> ']' ( '=' '{' <boo_val> (',' <boo_val>)* '}')?
<int_var_decl2> --> <ident> | <ident>'['<boo_val> | <ident_const> ']'
<num> --> [0-9]+

<ident> --> _*[a-zA-Z]+

<function> --> 'function' <ident> '('')' '{' <block> '}'
<statement> --> ';' | <assignment> | <call> | <scope> | <if> | <while> | <do-while> | <for> | <foreach> | <repeat-until> | <switch> | <goto> | <read> | <write>
<assignment> ->  <ident> (':=' <ident>)* ':=' <expression> ';' | <ternary-operator> | <label>
<ternary-operator> --> <ident>':=' '#' <condition> '?' <expression> ':' <expression> ';'
<label> --> <ident> ':'
<call> --> 'call' <ident> '(' ')' ';'
<scope> --> '{' <statement> '}' ('else' <statement>)?
<if> --> 'if' '(' <condition> ')' <statement>
<while> --> 'while' '(' <condition> ')' <statement>
<do-while> --> 'do' '{' <statement> '}' 'while' '(' <condition> ')'
<for> --> 'for' '(' <assignment> ';' <condition> ';' <assignment> ')' <statement>
<goto> --> 'goto' <ident>';'
<foreach> --> 'foreach' '(' <ident> : <ident_array> ')' <statement>
<switch> --> 'switch' '(' <ident> ')' '{' <case>* '}'
<case> --> 'case' ( <num> | <boo_val> ) ':' <statement> ('break' ';')?
<read> --> 'read' '(' <ident> ')' ';' | 'read' '(' <ident> '[' <expression> ']' ')' ';'
<write> --> 'write' '(' <ident> ')' ';' |  'write' '(' <ident> '[' <expression> ']' ')' ';'

<condition> --> ! <expression> | <expression> == <expression> || <expression> != <expression> | <expression> <
<expression> | <expression> <= <expression> | <expression> > <expression> | <expression> >= <expression> | <expression>
&& <expression> | <expression> || <expression>
<expression> --> ('+' | '-')? <term> ( ('+' | '-') <term> )
<term> --> <factor> { ('*' | '/') <factor> }
<factor> --> <ident> | <num> | <boo_val> | '(' <expression> ')' | <instanceof> | <ident> '[' <expression> ']'
<instanceof> --> <ident> 'instanceof' 'int' | <ident> 'instanceof' 'bool' | <ident> 'instanceof' 'int[]' | <ident>
'instanceof' 'bool[]' | <ident> 'instanceof' 'function'
```

## Supported features

In order to make things simpler to understand, we provide a list of every feature available within our programming language. Overall, we support the following:

* definition of integer variables
* definition of integer constants
* assignment
* basic arithmetic and logic (+, -, *, /, AND, OR, negation and parentheses, operators for comparing numbers)
* loop (arbitrary)
* simple condition (if without else)
* definition of a subroutine (procedure, function, method) and its call

---

* for
* do-while
* repeat-until
* foreach
* else
* data type boolean and logical operations with it
* branching condition (switch, case)
* multi-assignment (a = b = c = d = 3;)
* conditional assignment / ternary operator (min = (a < b) ? a : b;)
* commands for input and output (read, write - needs appropriate instructions to be used)

---

* GOTO command (watch out for remote jumps)
* array and working with its elements

---

* instanceof operator

---

Every single feature is described in more detail within the next sections.

## Descriptions & examples of implemented features

Every program is encapsulated by the `START` and `END` symbols. Each program must contain at least one statement - check out folder `examples`. Therefore, the shortest program you can create using out programming language looks like this:

```
START
;
END
```

```
[#000] INC 0 4
[#001] JMP 0 2
[#002] OPR 0 0
```

### Comments

We do support comments in our programming language. However, there are no single-line comments. If you want to comment something out, you're required to use the `/*` ... `*\` syntax.

### Identifiers

An identifier follows the same rules as in, for example, the C programming language. These rules are laid out as:
    
   - an identifier cannot start with a number
   - an identifier must contain at least one character
   - an identifier must consist of only upper-case, lower-case, numbers, or underscore characters.

### Definition of integer/bool constants

```
const int x = 15, z = 155, y = 0;
const bool test1 = false, correct = true;
```

When creating a constant variable, you must prefix it with the `const` keyword. Immediate initialization is expected as well. As for integers, you can use any positive integer including zero. Though, booleans are internally interpreted as integers (zeros/ones), you can only initialize them with `true` or `false`. Creating constant arrays is not supported either.

### Definition of integer/bool variables

```
const int N = 100;
int x, z, y, arr[2] = {1,2}, arr2[5];
bool test1, correct, visited[1] = {true}, seen[N];
```

When creating a variable, you are not allowed to initialize it right away. You have to do it through the `assignment` statement later on. However, when creating an array you are given the option to initialize it. If you decide to do so, you must provide literals for all elements.

### Assignment

```
x := 15 + 6 - 1 + correct * arr[15];
arr[i] := arr[i+1];
correct := false;
```

Unlike declarations, you are obligated to use the `:=` character when assigning a value (expression) to a variable. 
You should be able to combine integers and booleans since boolean is internally represented as an integer of a value 
of zero or one.

### Basic arithmetic and logic

For this part we were strongly inspired by the PL0 programming language as described in its grammar over at https://en.wikipedia.org/wiki/PL/0.

We provide basic arithmetic operations such as `+`, `-`, `*`, and `/`. You should be able to use any variables, 
constants, or literals you want. You can also include the `instanceof` operator into an expression. Technically, the instanceof operator represents nothing but a boolean value (true/false - 1/0).

As far as conditions and simple logic are concerned, we support the following:

```
! <expression>
<expression> == <expression>
<expression> != <expression>
<expression> < <expression>
<expression> <= <expression>
<expression> > <expression>
<expression> => <expression>
<expression> && <expression>
<expression> || <expression>
```

### Loop (arbitrary)

As for the arbitrary yet mandatory loop, we decided to implement a `while` loop. The syntax of a `while` loop is fairly similar to other programming languages.

```
while (!false)
    ;
```

```
while ((15 + x) >= y) {
    .
    .
    .
}
```

### Simple condition (if without else)

The way we implemented an `if` statement is the same as other programming languages do it. In general, an `if` statement has a similar syntax to a `while`. It expects a condition to be passed in.

```
if (x != 15) {
    .
    .
}
```


```
if (true && false)
    ;
```

### Definition of a subroutine (procedure, function, method) and its call

Every function declaration has the following structure.

```
function foo() {
    .
    .
    .
}

call foo();
```
There aren't any parameters being passed into a function nor any return values. Hence, every function is 
technically a void function that returns no values. As shown below, we do allow nested functions.

```
START
function A() {
    function B() {
        function C() {
            write(155);
        }
        call C();
    }
    call B();
}

call A();
END
```

Every function can hold its constants and variables that are visible only to its nested functions.

---

### For loop

The syntax of our `for` loop is also fairly similar to programming languages such as Java or C/C++. The variable which is used to iterate within the for loop must be declared beforehand. You're not allowed to declare a variable in the for loop itself.

```
for (j := 1; j < 4; j := j + 1)
    write(arr[j-1]);
```

### Do-while loop

Unlike other loops, the do-while loop requires the use of curly brackets. As you may want to add multiple statements into the body of the loop, you may as well need to add an additional pair of curly brackets. This is a result of the fact that each loop expects one statement provided in its body. If you want to add multiple statements you're required to wrap them into a pair of curly brackets.

```
iterations := 2;
do {{
    call foo();
    iterations := iterations - 1;
}} while (iterations > 0);
```

### Repeat-until loop

A repeat-until loop is almost the same as a do-while loop. The only difference is in how `while` and `until` are interpreted. A do-while loop keeps on executing its body as long as the while condition is satisfied. A repeat-until loop keeps on executing its body for as long as its until condition is NOT satisfied. From an implementation point of view, this was tackled by inverting the result of the condition on the top of the stack.

```
x := 0;
repeat {{
    write(x);
    x := x + 1;
}} until (x != 15);
```

### Foreach loop

A `foreach` loop is meant to be used to iterate over the elements of an array. All the user is supposed to do, is to declare a variable into which the elements will be copied as we go over the array.
Internally, it creates a temporary index variable on the stack, so we can keep track of the index of the current element.

```
START
const int N = 3;
int i, x[N] = {11,12,13};

{
    foreach (i : x)
        ;
    write(i);
}
END
```

### Else branch of an if statement

An `else` statement allows branching off based on the result of the condition evaluated in the if statement. Within 
an else statement, we can use an if statement again.

```
START
int x;
{
    x:=15;
    if (x == 0)
        write(1);
    else if (x == 13)
        write(0);
    else {
        write(x);
    }
}
END
```

### Data type boolean and logical operations with it

As shown before, you can create a const boolean as well as a variable of that type. Throughout the compilation process, booleans are treated as integers 0 or 1. Thus, you can combine both integers operations with boolean operations as any boolean variable will be interpreted as an integer anyway.

```
START
{
    if (1 && true) write(1);
    if (false || 0) write(2);
    if (false || -12) write(3);
    if (!false) write(4);
}
END
```

A boolean variable is considered to be `true` if and only if the corresponding integer value does not equal to zero. A zero value is treated as `false`.

### Branching condition (switch, case)

```
x:=0;
lbl:
switch(x) {
    case 0:
        x:=15;
    case 1:
        x:=2;
        break;
    case 3:
        x:=5;
        break;
    case 15:
        {
            x:=3;
            goto lbl;
        }
        break;
}
```
Only a variable can be passed into a switch statements - either an integer or boolean. Internally, we keep jumping 
over cases until we find one that matches the current value of the control variable. Then, we execute the body of 
the case. Each case may also contain a `break` instruction which causes an immediate termination of the switch 
statement.
If the control value is modified within the body of a case statement, its new value will be used when searching for 
another case that matches the value. This, of course, will not take place if a break statement is present.

### Multi-assignment (a = b = c = d = 3;)

Within our programming language, we also supposed multi-assignment. The variables on the left side are supposed to 
be primitive data types - integers or booleans. This will not work with array elements. As in a normal assignment, you're required to use the `:=` character. After the very last variable, an expression must follow.

```
a := d := b := c := ((((15 / 5) - 2/2)*5)/g[0/30])*(g instanceof int);
```

All variables a, d, b, c will be assigned the same value, which is the final result of the expression.

### Conditional assignment / ternary operator (min = (a < b) ? a : b;)

Another feature we decided to implement is a ternary operator.

```
 x := # 15 > 0 ? 15 - 3 : 101* 2 + 1;
```

In order to distinguish between an assignment and a ternary operator, you must prefix the ternary operator with a hash symbol. That way the parser will know that what follows next should be parsed as a ternary operator. The condition is terminated by a `?` symbol, and the expressions are separated using the `:` symbol.

### Commands for input and output (read, write - needs appropriate instructions to be used)

As for reading from the standard input and printing off to the standard output. We chose to use a made up `SIO` 
instruction. 

`SIO 0 1` will print the value on the top of the stack out to the standard output. Similarly, `SIO 0 2` will read an integer value form the standard input and place it on the top of the stack. Both instructions are supported by our virtual machine. 

The following program creates an array of three integers, reads all three values from the standard input and prints them out to the standard output.

```
START
const int N = 3;
function test() {
    bool arr[N];
    int i;
    {
        for (i := 1; i <= N; i := i + 1)
            read(arr[i-1]);

        for (i := 0; i < N; i := i + 1)
            write(arr[i]);
    }
}
{
    call test();
}
END
```

### GOTO command (watch out for remote jumps)

`GOTO` allows the user to jump literally anywhere they want. As a result of this behavior, this instruction is deprecated in most modern programming languages. Despite not being used very often these days, we still decided to implement it as proof of concept. It may come in handy when exiting multiple nested loops. It is the user who is responsible for all possible consequences, and therefore, they are strongly advised to use it cautiously.

```
START
{
    goto exit;
    exit:
}
END
```

```
START
{
    loop:
    goto loop;
}
END
```

The second example represents an infinite loop.

### Array and working with its elements

As another datatype, or rather a collection, we decided to implement arrays. An array is statically allocated on the 
stack and can contain 1 or more elements. There are two kinds of arrays we support, an array of booleans and an 
array of integers. The user must specify the size of an array upon declaration.


```
const int N = 15;
int array[10];
bool visited[N];
```

The size of an array is supposed to be either a literal value or a constant integer. Each element of an array can be accessed via an index surrounded squared brackets, as known from other programming languages.

```
write(array[1]);
read(array[i+1]);
x := array[0] + array[x+1];
```
Furthermore, when creating an array, the user is given the option to initialize it with default elements. 

```
int nodes[5] = {1,2,3,4,5};
```

### Instanceof operator

The instanceof operator is used to find out if a variable is of a certain type or not. For instance, we can test 
whether `x` is of a type of `function`, `array`, `int`, or `bool`. The return value is always `true` or `false`, which 
is then interpreted as `1` or `0`.

```
START
bool b, c, d, e, f;
function a() {

}
{
    b := a instanceof bool[];
    c := a instanceof int[];
    d := a instanceof function;
    e := a instanceof int;
    f := a instanceof bool;

    write(b);
    write(c);
    write(d);
    write(e);
    write(f);
}
END
```

```
0
0
1
0
0
```

## Project layout

```
.
????????? doc              # Contains UML diagrams
??????? ????????? uml-01.svg
??????? ????????? uml-02.svg
??????? ????????? uml-03.svg
????????? doxygen          # Contains documentation generated by Doxygen
??????? ????????? html
????????? doxygen.conf     # Doxygen configuration
????????? examples         # Test programs
????????? grammar.txt      # Grammar of our programming language
????????? include          # Header files
??????? ????????? code.h
??????? ????????? errors.h
??????? ????????? ilexer.h
??????? ????????? iparser.h
??????? ????????? isa.h
??????? ????????? ivm.h
??????? ????????? lexer.h
??????? ????????? logger.h
??????? ????????? parser.h
??????? ????????? symbol_table.h
??????? ????????? token.h
??????? ????????? vm.h
????????? lib              # Header-only libraries (static linking)
??????? ????????? cxxopts.hpp
????????? Makefile         # Makefile for building the application
????????? README.md        # This readme file
????????? src              # Source files
    ????????? code.cpp
    ????????? errors.cpp
    ????????? isa.cpp
    ????????? lexer.cpp
    ????????? logger.cpp
    ????????? main.cpp
    ????????? parser.cpp
    ????????? symbol_table.cpp
    ????????? token.cpp
    ????????? vm.cpp
```

Throughout the project, we mostly used object-oriented programming (OOP for short). The entire compiler consists of 
three main parts. These are `Lexer`, which parses an input file into a stream of tokens, `Parser`, which carries out 
the recursive descent algorithm down an AST, and `VM` (virtual machine), which performs execution onto the generated 
code 
(instructions written in PL0).

### UML diagram of the structure of the lexer

<img src="doc/uml-01.svg" width="350" height="350">

### UML diagram of the structure of the parser

<img src="doc/uml-02.svg" width="500" height="450">

### UML diagram of the structure of the VirtualMachine

<img src="doc/uml-03.svg" width="450" height="350">

## List of PL0 instructions

| Instruction | Description                                                                                                                 |
|-------------|:----------------------------------------------------------------------------------------------------------------------------|
| LIT         | Pushes a constant on the top of the stack.                                                                                  |
| OPR         | Performs an operation on the top of the stack.                                                                              |
| LOD         | Loads a value from an address to the top of the stack.                                                                      |
| STO         | Stores a value from the top of the stack to a particular address.                                                           |
| CAL         | Calls a function.                                                                                                           |
| INC         | Allocates 'x' positions (slots/variables) on the stack.                                                                     |
| JMP         | Jumps to an address.                                                                                                        |
| JPC         | Conditional jump. It jumps if there is a 1 on the top of the stack (result of an operation).                                |
| SIO         | System I/O operation (read/write).                                                                                          |
| LDA         | Loads data on the top of the stack from an address which is stored on the top of the stack.                                 |
| STA         | Stores the value which is on the top of the stack at the address which is at the second position from the top of the stack. |

## Conclusion

We created a several test programs in order to test out the functionality of the compiler. All features work as 
expected. However, some potential enhancements would be welcomed as well. For instance, the input file should be 
read as a stream of bytes instead of loading up the entire file into RAM. This could become an issue as far as larger 
files are concerned. Also, we would like to add some other features into the programming language such as dynamic allocations or working with strings. Overall, out of all features we picked out for implementation, we managed to accomplish all of them. For more information on the implementation, please check out `doxygen/html/index.html`.