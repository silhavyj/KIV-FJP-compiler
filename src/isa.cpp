#include "isa.h"

std::string FJP::op_code_to_str(OP_CODE op) {
    switch (op) {
        case LIT:
            return "LIT";
        case OPR:
            return "OPR";
        case LOD:
            return "LOD";
        case STO:
            return "STO";
        case CAL:
            return "CAL";
        case INC:
            return "INC";
        case JMP:
            return "JMP";
        case JPC:
            return "JPC";
        case SIO:
            return "SIO";
        case LDA:
            return "LDA";
        case STA:
            return "STA";
    }
    return "unknown";
}

std::ostream &FJP::operator<<(std::ostream &out, const Instruction &instruction) {
    out << op_code_to_str(instruction.op) << " "
        << instruction.l << " "
        << instruction.m;
    return out;
}
