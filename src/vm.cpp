#include <cstring>
#include <iostream>

#include <vm.h>
#include <errors.h>

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
    // Store the parameters.
    this->debug = debug;
    this->program = &program;

    // Init the virtual machine.
    init();

    // If debug is enabled, open up the output file.
    if (debug) {
        outputFile = std::ofstream(OUTPUT_FILE);
        if (outputFile.is_open() == false) {
            FJP::exitProgramWithError(FJP::IOErrors::ERROR_01, ERROR_CODE);
        }

        // This is the header of the file.
        outputFile << "\t\t\t\tPC\tEBP\tESP\tstack\n";
        outputFile << "initial values\t\t\t" << EIP << "\t" << EBP << "\t" << ESP << '\n';
    }

    // Executes the program. Keep fetching and executing
    // instructions until the vm gets halted.
    while (halt == 1 && EBP != 0) {
        fetch();
        execute();
    }
    // Close up the output file if debug is enabled.
    if (outputFile.is_open() == true) {
        outputFile.close();
    }
}

void FJP::VirtualMachine::init() {
    ESP  = 0; // stack pointer
    EBP  = 1; // base pointer
    EIP  = 0; // instruction pointer
    halt = 1; // flag if the virtual machine is halted or not

    // Number of functions that have been called = 0.
    // No function has been called yet.
    returnAddressCount = 0;

    // Clear out the entire stack as well as the return addresses.
    memset(returnAddresses, 0, sizeof(returnAddresses));
    memset(stackMemory, 0, sizeof(stackMemory));
}

void FJP::VirtualMachine::fetch() {
    // Fetch the very next instruction from the code.
    instruction = (*program)[EIP];
    EIP++;
}

void FJP::VirtualMachine::execute() {
    if (debug) {
        // If debug is enabled, print out the current
        // instruction that is about to be executed.
        outputFile << (EIP - 1) << "\t" << op_code_to_str(instruction.op) << "\t" << instruction.l << "\t" << instruction.m << "\t";
    }

    // Execute the current instruction
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
        case DEC:
            execute_DEC(instruction.l, instruction.m);
            break;
    }
    if (debug) {
        // Print out the current values of all three registers.
        outputFile << EIP << "\t" << EBP << "\t" << ESP << "\t";

        // Print out the content of the stack. The frames
        // are separated by the '|' symbol
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
    // Return the base pointer of the frame
    // 'levels' above (down the stack)
    while (l > 0) {
        base = stackMemory[base + 1];
        l--;
    }
    return base;
}

bool FJP::VirtualMachine::checkIfOverflows(std::function<int(int, int)> operation, int x, int y) {
    // Performs the operation.
    int result = operation(x, y);

    // Check if an overflow error has occurred.
    return (x < 0 && y < 0 && result > 0) || (x > 0 && y > 0 && result < 0);
}

void FJP::VirtualMachine::execute_LIT(int l, int m) {
    // So the compiler doesn't complain about an unused variable.
    (void)l;

    // Make sure that the stack is not all taken up
    if (ESP == STACK_SIZE) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }

    // Push the 'm' value on the top of the stack.
    ESP++;
    stackMemory[ESP] = m;
}

void FJP::VirtualMachine::execute_OPR(int l, int m) {
    // So the compiler doesn't complain about an unused variable.
    (void)l;

    // Perform the corresponding operation.
    switch (m) {
        // return
        case FJP::OPRType::OPR_RET:
            ESP = EBP - 1;
            EIP = stackMemory[ESP + 4];
            EBP = stackMemory[ESP + 3];
            returnAddressCount--;
            break;
        // x = -x
        case FJP::OPRType::OPR_INVERT_VALUE:
            stackMemory[ESP] = -1 * stackMemory[ESP];
            break;
        // x + y
        case FJP::OPRType::OPR_PLUS:
            ESP--;
            if (checkIfOverflows([&](int x, int y) {return x + y; }, stackMemory[ESP], stackMemory[ESP + 1])) {
                FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_02, ERROR_CODE);
            }
            stackMemory[ESP] = stackMemory[ESP] + stackMemory[ESP + 1];
            break;
        // x - y
        case FJP::OPRType::OPR_MINUS:
            ESP--;
            stackMemory[ESP] = stackMemory[ESP] - stackMemory[ESP + 1];
            break;
        // x * y
        case OPR_MUL:
            ESP--;
            if (checkIfOverflows([&](int x, int y) {return x * y; }, stackMemory[ESP], stackMemory[ESP + 1])) {
                FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_02, ERROR_CODE);
            }
            stackMemory[ESP] = stackMemory[ESP] * stackMemory[ESP + 1];
            break;
        // x / y
        case FJP::OPRType::OPR_DIV:
            ESP--;
            if (stackMemory[ESP + 1] == 0) {
                FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_01, ERROR_CODE);
            }
            stackMemory[ESP] = stackMemory[ESP] / stackMemory[ESP + 1];
            break;
        // x % 2
        case FJP::OPRType::OPR_ODD:
            stackMemory[ESP] = stackMemory[ESP] % 2;
            break;
        // x % y
        case FJP::OPRType::OPR_MOD:
            ESP--;
            stackMemory[ESP] = stackMemory[ESP] % stackMemory[ESP + 1];
            break;
        // x == y
        case FJP::OPRType::OPR_EQ:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] == stackMemory[ESP + 1]);
            break;
        // x != y
        case FJP::OPRType::OPR_NEQ:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] != stackMemory[ESP + 1]);
            break;
        // x < y
        case OPR_LESS:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] < stackMemory[ESP + 1]);
            break;
        // x <= y
        case FJP::OPRType::OPR_LESS_EQ:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] <= stackMemory[ESP + 1]);
            break;
        // x > y
        case FJP::OPRType::OPR_GRT:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] > stackMemory[ESP + 1]);
            break;
        // x >= y
        case FJP::OPRType::OPR_GRT_EQ:
            ESP--;
            stackMemory[ESP] = (stackMemory[ESP] >= stackMemory[ESP + 1]);
            break;
    }
}

void FJP::VirtualMachine::execute_LOD(int l, int m) {
    // Make sure that the stack is not all taken up
    if (ESP == STACK_SIZE) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }

    // Load the value from the address to the top of the stack.
    ESP++;
    stackMemory[ESP] = stackMemory[base(l, EBP) + m];
}

void FJP::VirtualMachine::execute_STO(int l, int m) {
    // Stores the value at the address.
    stackMemory[base(l, EBP) + m] = stackMemory[ESP];
    ESP--;
}

void FJP::VirtualMachine::execute_CAL(int l, int m) {
    // Push all necessary values on the stack
    // before jumping to the first address of the function.
    stackMemory[ESP + 1] = 0;
    stackMemory[ESP + 2] = base(l, EBP);
    stackMemory[ESP + 3] = EBP;
    stackMemory[ESP + 4] = EIP;

    // Set the new base pointer.
    EBP = ESP + 1;

    // Set the instruction pointer to the
    // first address of the function.
    EIP = m;

    // Store the return address.
    returnAddresses[returnAddressCount] = ESP;
    returnAddressCount++;
}

void FJP::VirtualMachine::execute_INC(int l, int m) {
    // Just so the compiler doesn't complain about an unused value.
    (void)l;

    // Make sure that there is enough free space on the stack.
    if (m + ESP > STACK_SIZE) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }

    // Allocate 'm' spots on the stack.
    ESP = ESP + m;
}

void FJP::VirtualMachine::execute_DEC(int l, int m) {
    // Just so the compiler doesn't complain about an unused value.
    (void)l;

    // Make sure that there is at least the number of elements on
    // the stack that we want to reduce it by
    if (ESP - m < 0) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }

    // Reduce the size of the stack (the opposite of INC)
    ESP = ESP - m;
}

void FJP::VirtualMachine::execute_JMP(int l, int m) {
    // Just so the compiler doesn't complain about an unused value.
    (void)l;

    // Set the value of the instruction pointer.
    EIP = m;
}

void FJP::VirtualMachine::execute_JPC(int l, int m) {
    (void)l;
    if (stackMemory[ESP] == 0) {
        EIP = m;
    }
    ESP--;
}

void FJP::VirtualMachine::execute_LDA(int l, int m) {
    // Just so the compiler doesn't complain about an unused value.
    (void)m;

    // Calculate the address within the corresponding frame.
    int frameAddress = base(l, EBP) + stackMemory[ESP];

    // Make sure the source address exists within the stack.
    if (frameAddress > ESP || frameAddress < 0) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }

    // Load the value up from the address to the top of the stack.
    stackMemory[ESP] = stackMemory[frameAddress];
}

void FJP::VirtualMachine::execute_STA(int l, int m) {
    // Just so the compiler doesn't complain about an unused value.
    (void)m;

    // Make sure that there are at least two values
    // on the stack - the value and the address
    if (ESP < 2) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }

    // Calculate the address within the corresponding frame.
    int frameAddress = base(l, EBP) + stackMemory[ESP - 1];

    // Make sure the target address exists within the stack.
    if (frameAddress > ESP || frameAddress < 0) {
        FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
    }

    // Store the value from the top of the stack at the address.
    stackMemory[frameAddress] = stackMemory[ESP];
    ESP -= 2;
}

void FJP::VirtualMachine::execute_SIO(int l, int m) {
    // Just so the compiler doesn't complain about an unused value.
    (void)l;

    // Perform an I/O operation
    switch (m) {
        // write
        case FJP::SIO_TYPE::SIO_WRITE:
            std::cout << stackMemory[ESP] << '\n';
            ESP--;
            break;
        // read
        case FJP::SIO_TYPE::SIO_READ:
            if (ESP == STACK_SIZE) {
                FJP::exitProgramWithError(FJP::RuntimeErrors::ERROR_00, ERROR_CODE);
            }
            ESP++;
            std::cin >> stackMemory[ESP];
            break;
        // halt
        case FJP::SIO_TYPE::SIO_HALT:
            halt = 0;
            break;
        default:
            break;
    }
}