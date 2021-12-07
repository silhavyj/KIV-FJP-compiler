#pragma once

#include <vector>

#include <isa.h>

namespace FJP {

    class GeneratedCode {
    private:
        std::vector<Instruction> code;
    public:
        GeneratedCode();
        int getSize() const;
        Instruction &operator[](size_t index);
        void addInstruction(FJP::Instruction instruction);
    };
}