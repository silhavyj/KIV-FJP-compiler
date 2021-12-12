#pragma once

#include <map>
#include <memory>

#include <ilexer.h>
#include <iparser.h>
#include <code.h>
#include <symbol_table.h>

namespace FJP {

    /// This class implements the functionality of a parser.
    /// It uses the lexer and its stream of tokens as an input.
    class Parser : public IParser {
    private:
        /// Default value of an integer variable.
        static constexpr int DEFAULT_INT_VALUE = 0;

        /// Default value of a bool variable (false).
        static constexpr int DEFAULT_BOOL_VALUE = 0;

        /// Parser error code (used when terminating the application).
        static constexpr int ERR_CODE = 2;

        /// Output file containing the stack trace as the program is being executed.
        static constexpr const char *OUTPUT_FILE = "code.pl0-asm";

        /// Number of digits an address is aligned to when being printed out.
        static constexpr int ADDRESS_LEN = 3;

        /// Number of variables stored onto the stack upon a function call.
        static constexpr int FRAME_INIT_VAR_COUNT = 4;

    private:
        /// The instance of the class.
        static Parser *instance;

        /// An instance of Lexer (set upon initialization).
        FJP::ILexer *lexer;

        /// Output of the parser - generated code
        FJP::GeneratedCode generatedCode;

        /// Symbol table used during the recursive descent algorithm.
        FJP::SymbolTable symbolTable;

        /// Current token retrieved from the lexer.
        FJP::Token token;

        /// Next free address within one function call.
        /// This address is used to store variables/arrays.
        int nextFreeAddress;

        /// Associative array of labels that need to be defined.
        /// Their address is not yet known.
        std::map<std::string, std::list<int>> undefinedLabels;

        /// Last processed variable - used in multi assignment.
        FJP::Symbol lastProcessVariable;

    private:
        /// Constructor - creates an instance of the class.
        Parser();

        /// Deleted copy constructor of the class.
        Parser(Parser &) = delete;

        /// Deleted assign operator of the class.
        void operator=(Parser const &) = delete;

        /// Stores the instructions of the output code into a file.
        void storeCodeInstructionsIntoFile();

        /// Processes a 'block' (recursive descent).
        void processBlock();

        /// Processes a 'const' (recursive descent).
        void processConst();

        /// Processes a 'variable' (recursive descent).
        void processVariable(int &frameVariableCount);

        /// Processes a 'function' (recursive descent).
        void processFunction();

        /// Processes a 'statement' (recursive descent).
        void processStatement();

        /// Processes an 'assignment' (recursive descent).
        /// \param expectSemicolon flag saying if there should a semicolon at the end of the assignment
        void processAssignment(bool expectSemicolon = true);

        /// Processes an 'expression' (recursive descent).
        void processExpression();

        /// Processes a 'call' (recursive descent).
        void processCall();

        /// Processes a 'scope' (recursive descent).
        void processScope();

        /// Processes an 'if' statement (recursive descent).
        void processIf();

        /// Processes a 'condition' (recursive descent).
        void processCondition();

        /// Processes a 'while' loop (recursive descent).
        void processWhile();

        /// Processes a 'do-while' loop (recursive descent).
        void processDoWhile();

        /// Processes a 'for' loop (recursive descent).
        void processFor();

        /// Processes a 'repeat-until' loop (recursive descent).
        void processRepeatUntil();

        /// Processes a 'foreach' loop (recursive descent).
        void processForeach();

        /// Processes a 'switch' statement (recursive descent).
        void processSwitch();

        /// Processes a 'ternary operator' (recursive descent).
        void processTernaryOperator();

        /// Processes a 'case' in a switch statement (recursive descent).
        /// \param variable the main variable used in the switch statement
        /// \param breaks list of all break statements within the switch statement
        void processCases(Symbol &variable, std::list<int> &breaks);

        /// Processes a 'label' (recursive descent).
        /// \param label name of the label (used in goto)
        void processLabel(const std::string label);

        /// Processes a 'goto' statement (recursive descent).
        void processGoto();

        /// Processes a 'term' (recursive descent).
        void processTerm();

        /// Processes a 'factor' (recursive descent).
        void processFactor();

        /// Processes a 'read' operation (recursive descent).
        void processRead();

        /// Processes a 'write' operation (recursive descent).
        void processWrite();

    public:
        /// Return the instance of the class.
        /// \return the instance of the class
        static Parser *getInstance();

        /// Performs recursive descent and generates code for the virtual machine.
        /// \param lexer instance of Lexer (getting a stream of tokens)
        /// \param debug flag if we want to output the program into a file
        /// \return generated code written in an extended/customized version of PL0
        FJP::GeneratedCode parse(FJP::ILexer *lexer, bool debug = false) override;
    };
}