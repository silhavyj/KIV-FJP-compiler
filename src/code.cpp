#include <code.h>

FJP::GeneratedCode::GeneratedCode() {
}

int FJP::GeneratedCode::getSize() const {
    return static_cast<int>(code.size());
}

FJP::Instruction &FJP::GeneratedCode::operator[](size_t index) {
    return code[index];
}

void FJP::GeneratedCode::addInstruction(FJP::Instruction instruction) {
    code.push_back(instruction);
}