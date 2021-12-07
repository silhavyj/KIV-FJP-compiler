#pragma once

#include <fstream>
#include <functional>

#include <isa.h>
#include <code.h>

namespace FJP {
    class VirtualMachine {
    private:
        static constexpr int STACK_SIZE = 1024;
        static constexpr int ERROR_CODE = 3;
        static constexpr const char *OUTPUT_FILE = "stacktrace.txt";

    private:
        static VirtualMachine *instance;

        int stackMemory[STACK_SIZE];
        int returnAddresses[STACK_SIZE];
        int returnAddressCount;
        int ESP;
        int EBP;
        int PC;
        int halt;
        bool debug;
        FJP::Instruction instruction;
        FJP::GeneratedCode *program;
        std::ofstream outputFile;

    private:
        VirtualMachine();
        VirtualMachine(VirtualMachine &) = delete;
        void operator=(VirtualMachine const &) = delete;

        void init();
        void fetch();
        void execute();
        int base(int l, int base);

        void execute_LIT(int l, int m);
        void execute_OPR(int l, int m);
        void execute_LOD(int l, int m);
        void execute_STO(int l, int m);
        void execute_CAL(int l, int m);
        void execute_INC(int l, int m);
        void execute_JMP(int l, int m);
        void execute_JPC(int l, int m);
        void execute_SIO(int l, int m);
        void execute_LDA(int l, int m);
        void execute_STA(int l, int m);
        void execute_DEC(int l, int m);

        bool checkIfOverflows(std::function<int(int, int)> operation, int x, int y);

    public:
        static VirtualMachine *getInstance();
        void execute(FJP::GeneratedCode &program, bool debug = false);
    };
}