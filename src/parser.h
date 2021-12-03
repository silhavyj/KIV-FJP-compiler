#pragma once

#include <map>
#include <memory>

#include "lexer.h"
#include "code.h"
#include "symbol_table.h"

namespace FJP {

    class Parser {
    private:
        static constexpr int DEFAULT_INT_VALUE = 0;
        static constexpr int DEFAULT_BOOL_VALUE = 0;
        static constexpr int ERR_CODE = 2;
        static constexpr const char *OUTPUT_FILE = "code.pl0-asm";
        static constexpr int ADDRESS_LEN = 3;
        static constexpr int FRAME_INIT_VAR_COUNT = 4;

    private:
        static Parser *instance;
        FJP::Lexer *lexer;
        FJP::GeneratedCode generatedCode;
        FJP::SymbolTable symbolTable;
        FJP::Token token;
        int arSize;
        std::map<std::string, std::list<int>> undefinedLabels;

    private:
        Parser();
        Parser(Parser &) = delete;
        void operator=(Parser const &) = delete;

        void storeCodeInstructionsIntoFile();

        void processBlock();
        void processConst();
        void processVariable(int &frameVariableCount);
        void processFunction();
        void processStatement();
        void processAssignment(bool expectSemicolon = true);
        void processExpression();
        void processNop();
        void processCall();
        void processScope();
        void processIf();
        void processCondition();
        void processWhile();
        void processDoWhile();
        void processFor();
        void processForeach();
        void processLabel(const std::string label);
        void processGoto();
        void processTerm();
        void processFactor();
        void processRead();
        void processWrite();

    public:
        static Parser *getInstance();
        FJP::GeneratedCode parse(FJP::Lexer *lexer, bool debug = false);
    };
}