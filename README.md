# KIV/FJP Compiler

# Table of contents

- [Introduction](#introduction)
- [How to compile and run the application](#how-to-compile-and-run-the-application)
- [Different options to run the application](#different-options-to-run-the-application)
- [Debug outputs of the program](#debug-outputs-of-the-program)

## Introduction

Within this module, we decided to implement a compiler for a **programming language of our own**. The syntax of our programming language is based off of PL0 and may slightly resemble the C programming language. As an outcome of this project, we compile source code written in our programming language into an extended/customized version of the **PL0 instruction set** (https://en.wikipedia.org/wiki/PL/0). Also, in order to test the correctness of the compiler, we decided to implement a **virtual machine** that executes the compiled code, so we can see and analyze the output.

As for the programming language, we decided to go with **C++** without the use of any external libraries such as Bison (https://www.gnu.org/software/bison/) or ANTRL (https://www.antlr.org/). This decision was made by the fact that we wanted to put the knowledge obtained from the lectures into practice and implement the recursive descent algorithm on our own.

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
start build.bat
```

An alternative way to do it is to double-click on the file.

### Execution

Upon successful compilation, a `build` folder should be created containing a `fjp` file, which represents the executable application.

#### Execution on Linux

```
./fjp
```

#### Execution on Windows

```
fjp.exe
```

After executing one of the commands listed above (depending on your operating system) you should be prompted with the following output.

```
ERR: Input file is not specified!
     Run './fjp --help'
```

If you get the error message shown above, you have successfully built the application.

## Different options to run the application

As suggested by the error shown above, the application expects an input file to be provided by the user. However, if you do so, nothing happens. You can use `--help` to check out what options the application supports.

```
./fjp --help
```

```
FJP compiler
Usage:
  ./fjp <input> [OPTION...]

  -d, --debug  generates the following files: tokens.json, code.pl0-asm, 
               stacktrace.txt
  -r, --run    executes the program
  -h, --help   print help
```

For example, if you only were to compile the code, see the instructions and not execute it, you would run the application with the `-d` option. Additionally, you could add the `-r` option in order to execute the program as well. Here are some examples of you can run the application.

```
./fjp my-program --debug
./fjp my-program -dr
./fjp my-program -r
./fjp my-program --run --debug
```

Parsing these parameters was tacked with the help of this library https://github.com/jarro2783/cxxopts.

## Debug outputs of the program

If the application is run with the `--debug` option. The following files will be generated. As an example, consider the following piece of code written in our programming language (my-program).

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

### `tokens.json`

This fill will contain all tokens recognized and parsed from the input file. The format of the file is JSON. As far as the example is concerned, the content of the file would look like this:

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

### `code.pl0`

The instructions into which the source code has been compiled look like this. The first column represents the address the instruction sits at. This is comes in handy when analyzing jump instructions, conditional jumps, function calls, etc.

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

### `stacktrace.txt`

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

On the left side, we can see the instruction that's currently being executed. We can also see the content of registers `EIP`, `EBP`, and `ESP`. On the right side, we can see the current content of the stack. Every function call (every frame) is separated by the `|` symbol.