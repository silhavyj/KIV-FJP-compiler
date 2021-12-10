#pragma once

#include <iostream>

namespace FJP {

    /// Enumeration of all instructions available.
    /// These instructions are based on the PL0 instruction set.
    /// However, some additional custom instructions were added as well.
    enum OP_CODE {
        LIT = 1, ///< Pushes a constant on the top of the stack.
        OPR,     ///< Performs an operation on the top of the stack.
        LOD,     ///< Loads a value from an address to the top of the stack.
        STO,     ///< Stores a value from the top of the stack to a particular address.
        CAL,     ///< Calls a function.
        INC,     ///< Allocates 'x' positions (slots/variables) on the stack.
        DEC,     ///< Deallocates 'x' positions (slots/variables) off the stack.
        JMP,     ///< Jumps to an address.
        JPC,     ///< Conditional jump. It jumps if there is a 1 on the top of the stack (result of an operation).
        SIO,     ///< System I/O operation (read/write).
        LDA,     ///< Loads data on the top of the stack from an address which is stored on the top of the stack.
        STA      ///< Stores the value which is on the top of the stack at an address which is at the second position from the top of the stack.
    };

    /// Enumeration of different operations supported
    /// within this programming language. These codes are used
    /// as a parameter of the OPR instruction.
    enum OPRType {
        OPR_RET = 0,      ///< return
        OPR_INVERT_VALUE, ///< x = -x
        OPR_PLUS,         ///< x + y
        OPR_MINUS,        ///< x - y
        OPR_MUL,          ///< x * y
        OPR_DIV,          ///< x / y
        OPR_ODD,          ///< x % 2
        OPR_MOD,          ///< x % y
        OPR_EQ,           ///< x == y
        OPR_NEQ,          ///< x != y
        OPR_LESS,         ///< x < y
        OPR_LESS_EQ,      ///< x <= y
        OPR_GRT,          ///< x > y
        OPR_GRT_EQ        ///< x >= y
    };

    /// Definition of all system I/O operations
    /// available within the program. These codes are
    /// used as a parameter of the SIO instruction.
    enum SIO_TYPE {
        SIO_WRITE = 1,    ///< writes a number to the stdio
        SIO_READ,         ///< reads a number from the stdio
        SIO_HALT          ///< halts the system (termination of the program)
    };

    /// Definition of an instruction. An instruction
    /// consists of an OP code, its level or depth, and its parameter.
    struct Instruction {
        OP_CODE op;      ///< OP code of the instruction
        int l;           ///< the level or depth which the operation is supposed to take into account when being executed
        int m;           ///< the second parameter of the instruction (constance value, type of an operation, address, etc.)
    };

    /// Prints out an instructions
    /// \param out the output stream the instruction will be printed out into.
    /// \param instruction the instruction itself
    /// \return modified output stream (extended by the instruction)
    std::ostream &operator<<(std::ostream &out, const Instruction &instruction);

    /// Converts an OP code into a string value. This method
    /// is used when an instruction is being printed out.
    /// \param op the OP code of the instruction
    /// \return string value of the OP code
    std::string op_code_to_str(OP_CODE op);
}