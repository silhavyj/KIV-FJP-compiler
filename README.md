# KIV/FJP Compiler

## Introduction

Within this module, we decided to implement a compiler for a **programming language of our own**. The syntax of our programming language is based off of PL0 may slightly resemble the C programming language. As an outcome of this project, we compile source code written in our programming language into an extended/customized version of the **PL0 instruction set** (https://en.wikipedia.org/wiki/PL/0). Also, in order to test the correctness of the compiler, we decided to implement a **virtual machine** that executes the compiled code, so we can see and analyze the output.

As for the programming language, we decided to go with **C++** without the use of any external libraries such as Bison (https://www.gnu.org/software/bison/) or ANTRL (https://www.antlr.org/). This decision was made by the fact that we wanted to put the knowledge obtained from the lectures into practice and implement the recursive descent algorithm on our own.

## How to compile and run the application

### Requirements

Since we did not want to be limited by the Linux platform, we used the `cmake` tool (https://cmake.org/) to compile and build our application. In order to successfully build the application, you need to have `cmake` installed on your machine, whether it is Linux, Windows, or MacOS. You need to make sure that you can execute the `cmake` commad from your terminal. If you are on Windows, you may need to add the path to `cmake` into your environment variables.

### Compilation on Linux

If you are on Linux, all you are required to do is to navigate into the root folder of the project structure and execute the following commands in the terminal.

The first command makes the file executable, and the second command builds the whole application.

```
chmod +x build.sh
```

```
./build.sh
```

Upon successful execution, a `build` folder should be created containing the `fjp` file, which represents the executable application.

### Compilation on Windows