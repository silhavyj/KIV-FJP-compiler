#pragma once

#include <ilexer.h>
#include <code.h>

namespace FJP {

    /// This class represents the interface of a parser.
    /// It lists out functions that need to be implemented within
    /// a class which implements this interface.
    class IParser {
    public:
        /// Performs recursive descent and generates code for the virtual machine.
        /// \param lexer instance of Lexer (getting a stream of tokens)
        /// \param debug flag if we want to output the program into a file
        /// \return generated code written in an extended/customized version of PL0
        virtual FJP::GeneratedCode parse(FJP::ILexer *lexer, bool debug = false) = 0;
    };
}