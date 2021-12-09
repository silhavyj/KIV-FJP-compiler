#pragma once

#include <fstream>
#include <functional>

#include <isa.h>
#include <code.h>

namespace FJP {

    /// Virtual machine used to execute the code generated by the parser.
    /// It internally creates a virtual stack in order to be able to perform all
    /// the operations. Also, if enabled, it generates an output file containing
    /// stack trace information of the program as it is being executed.
    class VirtualMachine {
    private:
        static constexpr int STACK_SIZE = 1024; ///< size of the virtual stack (1 KB)
        static constexpr int ERROR_CODE = 3;    ///< error code of the VM (runtime exception)

        static constexpr const char *OUTPUT_FILE = "stacktrace.txt"; ///< name of the output file

    private:
        static VirtualMachine *instance;  ///< the instance of the VirtualMachine class

        int stackMemory[STACK_SIZE];      ///< internal stack used to perform operations as defined in the program
        int returnAddresses[STACK_SIZE];  ///< list of return addresses
        int returnAddressCount;           ///< number of return addresses stored in the list
        int ESP;                          ///< stack pointer
        int EBP;                          ///< base pointer
        int EIP;                          ///< instruction pointer
        int halt;                         ///< flag indicating the end of the program
        bool debug;                       ///< flag pass in from the main function - create the stack trace output file or not
        FJP::Instruction instruction;     ///< current instruction
        FJP::GeneratedCode *program;      ///< program to be executed (input data of the virtual machine)
        std::ofstream outputFile;         ///< output file (stream) - stack trace

    private:
        /// Constructor - creates an instance of the class
        VirtualMachine();

        /// Deleted copy constructor of the class
        VirtualMachine(VirtualMachine &) = delete;

        /// Deleted assign operator of the class
        void operator=(VirtualMachine const &) = delete;

        /// Initializes the virtual machine - sets the registers,
        /// clears out the stack, and so on.
        void init();

        /// Fetches the next instruction to be executed.
        void fetch();

        /// Executes the current instruction.
        void execute();

        /// Calculates the address where a variable is actually stored
        /// within the stack (it might be stored in a different frame/depth/level)
        int base(int l, int base);

        /// Executes the LIT instruction.
        /// The LIT instruction pushes a constant value on the top of the stack
        /// \param l unused
        /// \param m the constant to be pushed onto the stack
        void execute_LIT(int l, int m);

        /// Executes the OPR instruction.
        /// The OPR instruction can perform the following operations:
        /// m = 0  -> return
        /// m = 1  -> x = -x
        /// m = 2  -> x + y
        /// m = 3  -> x - y
        /// m = 4  -> x * y
        /// m = 5  -> x / y
        /// m = 6  -> x % 2
        /// m = 7  -> x % y
        /// m = 8  -> x == y
        /// m = 9  -> x != y
        /// m = 10 -> x < y
        /// m = 11 -> x <= y
        /// m = 12 -> x > y
        /// m = 12 -> x >= y
        /// \param l unused
        /// \param m type of the operations
        void execute_OPR(int l, int m);

        /// Executes the LOD instruction.
        /// The LOD instruction loads a value from a certain address to the top of the stack.
        /// \param l depth/level of the symbol (address)
        /// \param m address of the symbol
        void execute_LOD(int l, int m);

        /// Executes the STO instruction.
        /// The STO instruction stores the value from the top of the stack to a
        /// certain address within the stack.
        /// \param l depth/level of the symbol (address)
        /// \param m address of the symbol
        void execute_STO(int l, int m);

        /// Executes the CAL instruction.
        /// The CAL instruction calls a function stored at its assigned address
        /// \param l depth/level of the function
        /// \param m address of the function
        void execute_CAL(int l, int m);

        /// Executes the INC instruction.
        /// The INC instruction allocates some space on the stack. It's done
        /// by adding a certain number to the stack pointer, e.g. +12 allocates
        /// 12 positions (slots) on the stack.
        /// \param l unused
        /// \param m number by which the stack pointer will be increased
        void execute_INC(int l, int m);

        /// Executes the JMP instruction.
        /// The JPM instruction does nothing but jumps to a specific address
        /// \param l unused
        /// \param m the target address to jump to
        void execute_JMP(int l, int m);

        /// Executes the JPC instruction.
        /// The JPC instruction represents a conditional jump based
        /// on the value stored on the top of the stack. It jumps to a
        /// given address if there is 1 on the top of the stack.
        /// \param l level/depth of th target address
        /// \param m target address
        void execute_JPC(int l, int m);

        /// Executes the SIO instruction.
        /// The SIO instruction executes a system I/O operation based on the its m value.
        /// m = 0 -> prints the value on the top of the stack out to the screen
        /// m = 1 -> reads the value from the user and pushes it onto the stack
        /// m = 2 -> halts the system (the program terminates)
        /// \param l unused
        /// \param m type of the i/O operation
        void execute_SIO(int l, int m);

        /// Executes the LDA instruction.
        /// The LDA instruction loads a value from a certain address
        /// and pushes it onto the stack. The instruction expects the
        /// address to be stored on the top of the stack beforehand.
        /// \param l level/depth of the source address
        /// \param m unused
        void execute_LDA(int l, int m);

        /// Executes the STA instruction.
        /// The STA instruction stores a value at a certain address.
        /// The number as well as the address are meant to be pushed
        /// onto the stack beforehand (the address is pushed before the value)
        /// \param l level/depth of the target address
        /// \param m unused
        void execute_STA(int l, int m);

        /// Executes the DEC instruction.
        /// The DEC instruction does the opposite to the INC instruction.
        /// It decrements the ESP register by a value passed in the m parameter.
        /// \param l unused
        /// \param m the value by which we want to decrement the stack pointer
        void execute_DEC(int l, int m);

        /// Checks if the operation would cause an overflow exception.
        /// \param operation the operation to be performed on the x and y parameters
        /// \param x first parameter of the operation
        /// \param y second parameter of the operation
        /// \return true/false depending on whether the operation would
        ///         cause an overflow exception
        bool checkIfOverflows(std::function<int(int, int)> operation, int x, int y);

    public:
        /// Returns the instance of the VirtualMachine class.
        /// \return the instance of the class
        static VirtualMachine *getInstance();

        /// Executes a program given as a parameter.
        /// \param program the instance of a program to be executed
        /// \param debug flag if want to create an output file - stacktrace.txt
        void execute(FJP::GeneratedCode &program, bool debug = false);
    };
}