#pragma once

#include <list>
#include <unordered_set>

#include <token.h>

namespace FJP {

    class Lexer {
    private:
        static constexpr int ERR_CODE = 1;
        static constexpr const char *OUTPUT_FILE = "tokens.json";

    private:
        static constexpr const char *COMMENT_START = "/*";
        static constexpr const char *COMMENT_END   = "*/";
        static constexpr int MAX_IDENTIFIER_LEN = 16;

        static Lexer *instance;

        long currentCharIndex;
        int currentLineNumber;
        std::string fileContent;

        std::list<Token> tokens;
        std::list<Token>::const_iterator currentTokenIt;
        std::unordered_set<std::string> alphabetic_keywords;

    private:
        Lexer();
        Lexer(Lexer &) = delete;
        void operator=(Lexer const &) = delete;

        void processAllTokens(bool debug);
        FJP::Token parseNextToken();
        void skipWhiteCharacters();
        void skipComments();
        std::string getValue(int (validation_fce)(int));

    public:
        static Lexer *getInstance();
        void init(std::string filename, bool debug = false);
        FJP::Token getNextToken();
        void returnToPreviousToken();
        bool isEndOfFile() const;
    };
}
