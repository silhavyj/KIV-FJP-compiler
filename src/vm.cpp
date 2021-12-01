#include <cstring>
#include <iostream>

#include "vm.h"
#include "errors.h"

FJP::VirtualMachine *FJP::VirtualMachine::instance = nullptr;

FJP::VirtualMachine* FJP::VirtualMachine::getInstance() {
    if (instance == nullptr) {
        instance = new VirtualMachine;
    }
    return instance;
}

FJP::VirtualMachine::VirtualMachine() {
}

void FJP::VirtualMachine::execute(FJP::GeneratedCode &program, bool debug) {
    this->debug = debug;
    this->program = &program;

    init();

    if (debug) {
        outputFile = std::ofstream(OUTPUT_FILE);
        if (outputFile.is_open() == false) {
            FJP::exitProgramWithError(FJP::IOErrors::ERROR_01, ERROR_CODE);
        }
        outputFile << "\t\t\t\tPC\tEBP\tESP\tstack\n";
        outputFile << "initial values\t\t\t" << PC << "\t" << EBP << "\t" << ESP << '\n';
    }

    while (halt == 1 && EBP != 0) {
        fetch();
        execute();
    }
    if (outputFile.is_open() == true) {
        outputFile.close();
    }
}

void FJP::VirtualMachine::init() {
    ESP  = 0;
    EBP  = 1;
    PC   = 0;
    halt = 1;
    returnAddressCount = 0;

    memset(returnAddresses, 0, sizeof(returnAddresses));
    memset(stackMemory, 0, sizeof(stackMemory));
}

void FJP::VirtualMachine::fetch() {
    instruction = (*program)[PC];
    PC++;
}

void FJP::VirtualMachine::execute() {
    if (debug) {
        outputFile << (PC - 1) << "\t" << op_code_to_str(instruction.op) << "\t" << instruction.l << "\t" << instruction.m << "\t";
    }
    switch (instruction.op) {
        case LIT:
            execute_LIT(instruction.l, instruction.m);
            break;
        case OPR:
            execute_OPR(instruction.l, instruction.m);
            break;
        case LOD:
            execute_LOD(instruction.l, instruction.m);
            break;
        case STO:
            execute_STO(instruction.l, instruction.m);
            break;
        case CAL:
            execute_CAL(instruction.l, instruction.m);
            break;
        case INC:
            execute_INC(instruction.l, instruction.m);
            break;
        case JMP:
            execute_JMP(instruction.l, instruction.m);
            break;
        case JPC:
            execute_JPC(instruction.l, instruction.m);
            break;
        case SIO:
            execute_SIO(instruction.l, instruction.m);
            break;
        case LDA:
            execute_LDA(instruction.l, instruction.m);
            break;
        case STA:
            execute_STA(instruction.l, instruction.m);
            break;
    }
    if (debug) {
        outputFile << PC << "\t" << EBP << "\t" << ESP << "\t";

        int index = 0;
        for (int i = 1; i <= ESP; i++) {
            if (index < returnAddressCount && returnAddresses[index] < i) {
                outputFile << "| ";
                index++;
            }
            outputFile << stackMemory[i] << " ";
        }
        outputFile << "\n";
    }
}

int FJP::VirtualMachine::base(int l, int base) {
    while (l > 0) {
        base = stackMemory[base + 1];
        l--;
    }
    return base;
}

bool FJP::VirtualMachine::checkIfOverflows(std::function<int(int, int)> operation, int x, int y) {
    int result = operation(x, y);
    return (x < 0 && y < 0 && result > 0) || (x > 0 && y > 0 && result < 0);
}

void FJP::VirtualMachine::execute_LIT(int l, int m) {
    (void)l;
    if (ESP == STACK_SIZE) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }
    ESP++;
    stackMemory[ESP] = m;
}

void FJP::VirtualMachine::execute_OPR(int l, int m) {
    (void)l;
    switch (m) {
        case FJP::OPRType::OPR_RET:
            ESP = EBP - 1;
            PC = stackMemory[ESP + 4];
            EBP = stackMemory[ESP + 3];
            returnAddressCount--;
            break;
        case FJP::OPRType::OPR_INVERT_VALUE:
            stackMemory[ESP] = -1 * stackMemory[ESP];
            break;
        case FJP::OPRType::OPR_PLUS:
            ESP--;
            if (checkIfOverflows([&](int x, int y) {return x + y; }, stackMemory[ESP], stackMemory[ESP + 1])) {
                FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_02, ERROR_CODE);
            }
            stackMemory[ESP] = stackMemory[ESP] + stackMemory[ESP + 1];
            break;
        case FJP::OPRType::OPR_MINUS:
            ESP--;
            stackMemory[ESP] = stackMemory[ESP] - stackMemory[ESP + 1];
            break;
        case OPR_MUL:
            ESP--;
            if (checkIfOverflows([&](int x, int y) {return x * y; }, stackMemory[ESP], stackMemory[ESP + 1])) {
                FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_02, ERROR_CODE);
            }
            stackMemory[ESP] = stackMemory[ESP] * stackMemory[ESP + 1];
            break;
        case FJP::OPRType::OPR_DIV:
            ESP--;
            if (stackMemory[ESP + 1] == 0) {
                FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_01, ERROR_CODE);
            }
            stackMemory[ESP] = stackMemory[ESP] / stackMemory[ESP + 1];
            break;
        case FJP::OPRType::OPR_ODD:
            stackMemory[ESP] = stackMemory[ESP] % 2;
            break;
        case FJP::OPRType::OPR_MOD:
            ESP--;
            stackMemory[ESP] = stackMemory[ESP] % stackMemory[ESP + 1];
            break;
        case FJP::OPRType::OPR_EQ:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] == stackMemory[ESP + 1]);
            break;
        case FJP::OPRType::OPR_NEQ:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] != stackMemory[ESP + 1]);
            break;
        case OPR_LESS:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] < stackMemory[ESP + 1]);
            break;
        case FJP::OPRType::OPR_LESS_EQ:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] <= stackMemory[ESP + 1]);
            break;
        case FJP::OPRType::OPR_GRT:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] > stackMemory[ESP + 1]);
            break;
        case FJP::OPRType::OPR_GRT_EQ:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] >= stackMemory[ESP + 1]);
            break;
    }
}

void FJP::VirtualMachine::execute_LOD(int l, int m) {
    if (ESP == STACK_SIZE) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }
    ESP++;
    stackMemory[ESP] = stackMemory[base(l, EBP) + m];
}

void FJP::VirtualMachine::execute_STO(int l, int m) {
    stackMemory[base(l, EBP) + m] = stackMemory[ESP];
    ESP--;
}

void FJP::VirtualMachine::execute_CAL(int l, int m) {
    stackMemory[ESP + 1] = 0;
    stackMemory[ESP + 2] = base(l, EBP);
    stackMemory[ESP + 3] = EBP;
    stackMemory[ESP + 4] = PC;

    EBP = ESP + 1;
    PC = m;

    returnAddresses[returnAddressCount] = ESP;
    returnAddressCount++;
}

void FJP::VirtualMachine::execute_INC(int l, int m) {
    (void)l;
    ESP = ESP + m;
}

void FJP::VirtualMachine::execute_JMP(int l, int m) {
    (void)l;
    PC = m;
}

void FJP::VirtualMachine::execute_JPC(int l, int m) {
    (void)l;
    if (stackMemory[ESP] == 0) {
        PC = m;
    }
    ESP--;
}

void FJP::VirtualMachine::execute_LDA(int l, int m) {
    (void)m;
    int frameAddress = base(l, EBP) + stackMemory[ESP];
    if (frameAddress > ESP || frameAddress < 0) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }
    stackMemory[ESP] = stackMemory[frameAddress];
}

void FJP::VirtualMachine::execute_STA(int l, int m) {
    (void)m;
    if (ESP < 2) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }
    int frameAddress = base(l, EBP) + stackMemory[ESP - 1];
    stackMemory[frameAddress] = stackMemory[ESP];
    ESP -= 2;
}

void FJP::VirtualMachine::execute_SIO(int l, int m) {
    (void)l;
    switch (m) {
        case FJP::SIO_TYPE::SIO_WRITE:
            std::cout << stackMemory[ESP] << '\n';
            ESP--;
            break;
        case FJP::SIO_TYPE::SIO_READ:
            if (ESP == STACK_SIZE) {
                FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
            }
            ESP++;
            std::cin >> stackMemory[ESP];
            break;
        case FJP::SIO_TYPE::SIO_HALT:
            halt = 0;
            break;
        default:
            break;
    }
}