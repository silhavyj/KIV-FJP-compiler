#pragma once

#include <string>

#include <token.h>

namespace FJP {

    /// This class represents the interface of a lexer.
    /// It lists out functions that need to be implemented within
    /// a class which implements this interface.
    class ILexer {
    public:
        /// Initializes the lexer. It reads up the content of
        /// the input file and initializes all variables needed
        /// for parsing it.
        /// \param filename path to the input file (source code)
        /// \param debug if this flag is on, the tokens will be output into a file in a JSON format.
        virtual void init(std::string filename, bool debug = false) = 0;

        /// Returns the next token. All tokens have been parsed
        /// beforehand and they are no trodden as a stream. This
        /// method is called by the parser when performing a recursive descent.
        /// \return the next token in the stream of tokens.
        virtual FJP::Token getNextToken() = 0;

        /// Goes back one token within the stream of tokens.
        virtual void returnToPreviousToken() = 0;
    };
}