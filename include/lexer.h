#pragma once

#include <list>
#include <unordered_set>

#include <ilexer.h>
#include <token.h>

namespace FJP {

    /// This class implements the functionality of a lexer.
    /// It takes an input file and converts it into a stream
    /// of tokens which are then consumed by the parser.
    class Lexer : public ILexer {
    private:
        /// Lexer error code (used when terminating the application).
        static constexpr int ERR_CODE = 1;

        /// Output file containing tokens in a JSON format.
        static constexpr const char *OUTPUT_FILE = "tokens.json";

    private:
        /// Characters used to indicate the start of a comment.
        static constexpr const char *COMMENT_START = "/*";

        /// Characters used to indicate the end of a comment.
        static constexpr const char *COMMENT_END   = "*/";

        /// Maximum allowed length of an identifier.
        static constexpr int MAX_IDENTIFIER_LEN = 16;

        /// The instance of the class.
        static Lexer *instance;

        /// Current index (pointer) within the source input file
        /// (which character is currently being processed)
        long currentCharIndex;

        /// Current line within the input file, so we can keep
        /// track of which token is on which line.
        int currentLineNumber;

        /// The content of the input file.
        /// TODO it'd be better if the file was read as a stream instead.
        std::string fileContent;

        /// List of all tokens extracted from the input file.
        std::list<Token> tokens;

        /// Iterator used to send tokens off to the parser when requested.
        std::list<Token>::const_iterator currentTokenIt;

        /// Helper set of alphabetic keywords of the language.
        /// It is used to find keywords in the input file.
        std::unordered_set<std::string> alphabetic_keywords;

    private:
        /// Construction - creates an instance of the class
        Lexer();

        /// Delete copy constructor of the class
        Lexer(Lexer &) = delete;

        /// Delete assign operator of the class
        void operator=(Lexer const &) = delete;

        /// Processes all tokens. It goes thought the input file
        /// in order to parse it into tokens.
        /// \param debug flag if we want to output the tokens into a JSON format
        void processAllTokens(bool debug);

        /// Parses a next token. This method is periodically used
        /// in the processAllTokens method.
        /// \return next token that was parsed from the input file.
        FJP::Token parseNextToken();

        /// Skips all white characters in the input file.
        void skipWhiteCharacters();

        /// Skips all comments in the input file.
        void skipComments();

        /// Keeps consuming characters from the input file as long as
        /// the validation_fce is satisfied. For example, this is used
        /// when parsing numbers or variable names - keep consuming
        /// characters as long as they are digits.
        /// param validation_fce validation function used to consume characters
        /// \return the result of the consumption of the input characters (name, number, ...)
        std::string getValue(int (validation_fce)(int));

        /// Returns true/false depending on whether the whole
        /// input file has been processed or not.
        /// \return true/false - depending on whether it's the end of file.
        bool isEndOfFile() const;

    public:
        /// Returns the instance of the class.
        /// \return the instance of the class
        static Lexer *getInstance();

        /// Initializes the lexer. It reads up the content of
        /// the input file and initializes all variables needed
        /// for parsing it.
        /// \param filename path to the input file (source code)
        /// \param debug if this flag is on, the tokens will be output into a file in a JSON format.
        void init(std::string filename, bool debug = false) override;

        /// Returns the next token. All tokens have been parsed
        /// beforehand and they are no trodden as a stream. This
        /// method is called by the parser when performing a recursive descent.
        /// \return the next token in the stream of tokens.
        FJP::Token getNextToken() override;

        /// Goes back one token within the stream of tokens.
        void returnToPreviousToken() override;
    };
}
