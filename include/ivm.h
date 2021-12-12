#pragma once

#include <code.h>

namespace FJP {

    /// This class represents the interface of a virtual machine.
    /// It lists out functions that need to be implemented within
    /// a class which implements this interface.
    class IVM {
    public:
        /// Executes a program given as a parameter.
        /// \param program the instance of a program to be executed
        /// \param debug flag if want to create an output file - stacktrace.txt
        virtual void execute(FJP::GeneratedCode &program, bool debug = false) = 0;
    };
}