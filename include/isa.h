#pragma once

#include <iostream>

namespace FJP {

    enum OP_CODE {
        LIT = 1,
        OPR,
        LOD,
        STO,
        CAL,
        INC,
        DEC,
        JMP,
        JPC,
        SIO,
        LDA,
        STA
    };

    enum OPRType {
        OPR_RET = 0,
        OPR_INVERT_VALUE,
        OPR_PLUS,
        OPR_MINUS,
        OPR_MUL,
        OPR_DIV,
        OPR_ODD,
        OPR_MOD,
        OPR_EQ,
        OPR_NEQ,
        OPR_LESS,
        OPR_LESS_EQ,
        OPR_GRT,
        OPR_GRT_EQ
    };

    enum SIO_TYPE {
        SIO_WRITE = 1,
        SIO_READ,
        SIO_HALT
    };

    struct Instruction {
        OP_CODE op;
        int l;
        int m;
    };

    std::ostream &operator<<(std::ostream &out, const Instruction &instruction);
    std::string op_code_to_str(OP_CODE op);
}