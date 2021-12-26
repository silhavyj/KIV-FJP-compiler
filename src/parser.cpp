#include <cassert>
#include <fstream>
#include <list>

#include <logger.h>
#include <isa.h>
#include <parser.h>
#include <errors.h>

#define PRINT_ADDRESSES

#ifdef PRINT_ADDRESSES
# include <iomanip>
#endif

FJP::Parser *FJP::Parser::instance = nullptr;

FJP::Parser* FJP::Parser::getInstance() {
    if (instance == nullptr) {
        instance = new Parser;
    }
    return instance;
}

FJP::Parser::Parser() : lexer(nullptr), nextFreeAddress(0) {
}

FJP::GeneratedCode FJP::Parser::parse(FJP::ILexer *lexer, bool debug) {
    // Make sure that the lexer is not nullptr.
    assert(lexer != nullptr);

    this->lexer = lexer;
    generatedCode = FJP::GeneratedCode();

    // START
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::START) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_37, ERR_CODE, token.lineNumber);
    }

    // <block>
    token = lexer->getNextToken();
    processBlock();

    // END
    if (token.tokenType != FJP::TokenType::END) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_36, ERR_CODE, token.lineNumber);
    }

    // Make sure that all undefined labels were eventually defined.
    for (auto &label : undefinedLabels) {
        std::string errMsg = "label '";
        errMsg += label.first;
        errMsg += "' has not been defined";
        FJP::exitProgramWithError(__FUNCTION__, errMsg.c_str(), ERR_CODE, token.lineNumber);
    }

    // If the debug flag is on, spill the generated code out into a file.
    if (debug == true) {
        storeCodeInstructionsIntoFile();
    }

    // Return the generated program co it can be executed by the virtual machine.
    return generatedCode;
}

void FJP::Parser::storeCodeInstructionsIntoFile() {
    // Open up the output file.
    std::ofstream file = std::ofstream (std::string(OUTPUT_FILE));

    // Make sure the file has been successfully opened.
    if (file.is_open() == false) {
        FJP::exitProgramWithError(FJP::IOErrors::ERROR_01, ERR_CODE);
        return;
    }

    // Store the generated code into the file.
    for (int i = 0; i < generatedCode.getSize(); i++) {
        file
             #ifdef  PRINT_ADDRESSES
                << "[#" << std::setw(ADDRESS_LEN) << std::setfill('0') << i << "] "
             #endif
             << generatedCode[i];
        if (i < generatedCode.getSize() - 1) {
            file << "\n";
        }
    }

    // Close the file at the end.
    file.close();
}

void FJP::Parser::processBlock() {
    // Default allocation size of the frame - 0, base(l, EBP), EBP, EIP.
    int frameVariableCount = FRAME_INIT_VAR_COUNT;

    // Address of the next variable that will be created within the frame.
    nextFreeAddress = FRAME_INIT_VAR_COUNT;
    symbolTable.createFrame();

    // Added a new instruction that will allocate a certain amount
    // of variable on the stack. The particular value will be specified
    // once the block has been completely parsed.
    generatedCode.addInstruction({FJP::OP_CODE::INC, 0, 0});
    int incInstructionAddress = generatedCode.getSize() - 1;

    // Process all const and variable declarations.
    processConst();
    processVariable(frameVariableCount);

    // Add a JMP instruction, so we can skip all functions and jump straight
    // at the first instruction of the "main function".
    int jmpAddress = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});

    // Process all functions.
    processFunction();

    // Now we know the first address of the actual program,
    // so we can set the jump address of the instruction that we added before.
    generatedCode[jmpAddress].m = generatedCode.getSize();

    // Also, since all variables must have been already declared, we can
    // specifically say how many variables need to be allocated on the stack.
    generatedCode[incInstructionAddress].m = frameVariableCount;

    // Process a statements.
    processStatement();

    // Destroy the frame since we're about to return from the function.
    nextFreeAddress -= FRAME_INIT_VAR_COUNT;
    symbolTable.destroyFrame();

    // Add a return operation as a return from the function.
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_RET});
}

// const int <identifier> = 15, <identifier> = 155;
// const bool <identifier> = true;
void FJP::Parser::processConst() {
    // const
    while (token.tokenType == FJP::TokenType::CONST) {
        token = lexer->getNextToken();

        int value;
        std::string identifier;
        FJP::TokenType dataType = token.tokenType;

        // int / bool
        if (!(dataType == FJP::TokenType::INT || dataType == FJP::TokenType::BOOL)) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_29, ERR_CODE, token.lineNumber);
        }

        while (true) {
            // identifier
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::IDENTIFIER) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_44, ERR_CODE, token.lineNumber);
                return;
            }
            // Make sure the name of the identifier isn't already taken.
            identifier = token.value;
            if (symbolTable.existsSymbol(identifier) == true) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_43, ERR_CODE, token.lineNumber);
                return;
            }
            // '='
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::CONST_INIT) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_21, ERR_CODE, token.lineNumber);
                return;
            }
            token = lexer->getNextToken();

            switch (dataType) {
                // If the type is an integer, the next token has to be a number.
                case FJP::TokenType::INT:
                    if (token.tokenType != FJP::TokenType::NUMBER) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_07, ERR_CODE, token.lineNumber);
                    }
                    // Convert the token value into an int and set to the identifier in the symbol table.
                    value = atoi(token.value.c_str());
                    symbolTable.addSymbol({FJP::SymbolType::SYMBOL_CONST, identifier, value, 0, 0, 0});
                    break;

                // If the type is a bool, the next token has to be either true or false.
                case FJP::TokenType::BOOL:
                    if (!(token.tokenType == FJP::TokenType::TRUE || token.tokenType == FJP::TokenType::FALSE)) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_10, ERR_CODE, token.lineNumber);
                    }
                    // Convert the token value into an int (0 = false, 1 = true) and set to the identifier in the symbol table.
                    symbolTable.addSymbol({FJP::SymbolType::SYMBOL_CONST, identifier, token.tokenType == FJP::TokenType::TRUE, 0, 0, 0});
                    break;
                default:
                    break;
            }
            // ','
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::COMMA)
                break;
        }
        // ';'
        if (token.tokenType != FJP::TokenType::SEMICOLON) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
        }

        // Read the next token so it can be processed.
        token = lexer->getNextToken();
    }
}

// <int>/<bool> <identifier>, <identifier>;
// int x, arr[10];
// bool a, b, c, d[2], e[111];
void FJP::Parser::processVariable(int &frameVariableCount) {
    // int/bool
    FJP::TokenType dataType = token.tokenType;
    if (!(dataType == FJP::TokenType::INT || dataType == FJP::TokenType::BOOL)) {
        return;
    }

    int arrayAddress;  // start address of an array
    int arraySize = 0; // size of an array
    int arrayDepth;    // level/depth of an array

    int number;
    std::string identifier;
    FJP::Symbol symbol;

    while (true) {
        // identifier
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::IDENTIFIER) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_04, ERR_CODE, token.lineNumber);
        }

        // Make sure the name is not already taken.
        identifier = token.value;
        if (symbolTable.existsSymbol(identifier) ==  true) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_43, ERR_CODE, token.lineNumber);
        }

        switch (dataType) {
            // int (default value is 0)
            case FJP::TokenType::INT:
                symbolTable.addSymbol({FJP::SymbolType::SYMBOL_INT, identifier, DEFAULT_INT_VALUE, symbolTable.getDepthLevel(), nextFreeAddress, 0 });
                break;

            // bool (default value is false)
            case FJP::TokenType::BOOL:
                symbolTable.addSymbol({FJP::SymbolType::SYMBOL_BOOL, identifier, DEFAULT_BOOL_VALUE, symbolTable.getDepthLevel(), nextFreeAddress, 0 });
                break;
            default:
                break;
        }
        nextFreeAddress++;
        frameVariableCount++;

        // Store the level of the variable in case it is an array - it will need to be
        // escalated from an ordinary integer/bool.
        arrayDepth = symbolTable.getDepthLevel();

        // '['
        token = lexer->getNextToken();
        if (token.tokenType == FJP::TokenType::LEFT_SQUARED_BRACKET) {
            arrayAddress = nextFreeAddress - 1;
            token = lexer->getNextToken();

            switch (token.tokenType) {
                // number e.g. arr[10]
                case FJP::TokenType::NUMBER:
                    arraySize = atoi(token.value.c_str());
                    if (arraySize <= 0) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_40, ERR_CODE, token.lineNumber);
                    }
                    nextFreeAddress += (arraySize - 1); // -1 because the int is already there, it has just been escalated to an array
                    frameVariableCount += (arraySize - 1);
                    symbolTable.makeArray(identifier, arraySize);
                    break;

                // identifier  e.g. arr[N]
                case FJP::TokenType::IDENTIFIER:
                    // Make sure the symbol does exist in the symbol table.
                    symbol = symbolTable.findSymbol(token.value);
                    if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_45, ERR_CODE, token.lineNumber);
                    }

                    // An array can be initialized only with a constance value.
                    if (symbol.symbolType != FJP::SymbolType::SYMBOL_CONST) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_06, ERR_CODE, token.lineNumber);
                    }

                    // Arrays of a size less than 1 are not allowed.
                    if (symbol.value < 1) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_40, ERR_CODE, token.lineNumber);
                    }
                    nextFreeAddress += (symbol.value - 1);
                    frameVariableCount += (symbol.value - 1);
                    symbolTable.makeArray(identifier, symbol.value);
                    arraySize = symbol.value;
                    break;
                default:
                    FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_26, ERR_CODE, token.lineNumber);
                    break;
            }

            // ']'
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_16, ERR_CODE, token.lineNumber);
            }
            token = lexer->getNextToken();

            // '='
            if (token.tokenType == FJP::TokenType::CONST_INIT) {
                // '{'
                token = lexer->getNextToken();
                if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
                    FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_18, ERR_CODE, token.lineNumber);
                }

                // If the user decides to initialize the array, they need
                // to initialize all elements, not just a subarray.
                for (int i = 0; i < arraySize; i++) {
                    token = lexer->getNextToken();
                    switch (dataType) {
                        // 'int'
                        case FJP::TokenType::INT:
                            if (token.tokenType != FJP::TokenType::NUMBER) {
                                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_07, ERR_CODE, token.lineNumber);
                            }
                            number = atoi(token.value.c_str());
                            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, number});
                            generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - arrayDepth, arrayAddress + i});
                            break;

                        // 'bool'
                        case FJP::TokenType::BOOL:
                            // If it's an array of booleans, the only allowed values are true and false.
                            if (!(token.tokenType == FJP::TokenType::TRUE || token.tokenType == FJP::TokenType::FALSE)) {
                                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_10, ERR_CODE, token.lineNumber);
                            }
                            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, token.tokenType == FJP::TokenType::TRUE});
                            generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - arrayDepth, arrayAddress + i});
                            break;
                        default:
                            break;
                    }
                    // Make sure the whole array has been initialized.
                    token = lexer->getNextToken();
                    if (token.tokenType != FJP::TokenType::COMMA && i < arraySize - 1) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_08, ERR_CODE, token.lineNumber);
                    }
                }
                // '}'
                if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
                    FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_19, ERR_CODE, token.lineNumber);
                }
                token = lexer->getNextToken();
            }
        }
        // ','
        if (token.tokenType != FJP::TokenType::COMMA) {
            break;
        }
    }
    // ';'
    if (token.tokenType !=  FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
        return;
    }

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();

    // Recursively call the same function in case there are more lines of variable declarations.
    processVariable(frameVariableCount);
}

// function <identifier>() { <block> }
void FJP::Parser::processFunction() {
    std::string identifier;

    // function
    while (token.tokenType == FJP::TokenType::FUNCTION) {
        // <identifier>
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::IDENTIFIER) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_44, ERR_CODE, token.lineNumber);
        }

        // Make sure that the name of the identifier isn't already taken up.
        identifier = token.value;
        if (symbolTable.existsSymbol(identifier) ==  true) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_43, ERR_CODE, token.lineNumber);
        }

        // Add the symbol into the symbol table.
        symbolTable.addSymbol({FJP::SymbolType::SYMBOL_FUNCTION, token.value, generatedCode.getSize(), symbolTable.getDepthLevel(), nextFreeAddress, 0});

        // '('
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
        }

        // ')'
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
        }

        // '{'
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_18, ERR_CODE, token.lineNumber);
        }

        // Process the body of the function.
        token = lexer->getNextToken();
        processBlock();

        // '}'
        if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_19, ERR_CODE, token.lineNumber);
        }

        // Load up the next token is it can be processed.
        token = lexer->getNextToken();
    }
}

void FJP::Parser::processStatement() {
    // ';' - nop
    if (token.tokenType == FJP::TokenType::SEMICOLON) {
        token = lexer->getNextToken();
        return;
    }
    if (processAssignment())  return;
    if (processCall())        return;
    if (processScope())       return;
    if (processIf())          return;
    if (processWhile())       return;
    if (processDoWhile())     return;
    if (processFor())         return;
    if (processRepeatUntil()) return;
    if (processForeach())     return;
    if (processSwitch())      return;
    if (processGoto())        return;
    if (processRead())        return;
    if (processWrite())       return;
}

// <identifier> := <expression>;
// <identifier> := <identifier> := <identifier> := <expression>;
bool FJP::Parser::processAssignment(bool expectSemicolon) {
    // <identifier>
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        return false;
    }

    // Make sure the identifier exists in the symbol table. If it's not found,
    // it is treated as a label (goto).
    // label:
    FJP::Symbol variable = symbolTable.findSymbol(token.value);
    if (variable.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        // Process the label and return the function.
        return processLabel(token.value);
    }

    bool isAnotherAssign = true;
    switch (variable.symbolType) {
        // If the identifier is an integer or bool.
        case FJP::SymbolType::SYMBOL_INT:
        case FJP::SymbolType::SYMBOL_BOOL:
            // ':='
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::ASSIGN) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_14, ERR_CODE, token.lineNumber);
            }
            // <identifier>
            token = lexer->getNextToken();
            if (token.tokenType == FJP::TokenType::IDENTIFIER) {

                // Check if the token two steps ahead is also an identifier. If it's
                // not an identifier, it has to be treated as an expression.
                // For example <identifier> := <identifier> or <identifier> := <identifier> + 55
                token = lexer->getNextToken();
                if (token.tokenType != FJP::TokenType::ASSIGN) {
                    lexer->returnToPreviousToken();
                    lexer->returnToPreviousToken();
                    token = lexer->getNextToken();
                    isAnotherAssign = false;
                }
                if (isAnotherAssign) {
                    // Recursively process another assignment.
                    processAssignment();

                    // Copy the value from the previous variable into the current one (chain of assignments).
                    generatedCode.addInstruction({FJP::OP_CODE::LOD, lastProcessVariable.level, lastProcessVariable.address});
                    if (variable.symbolType == FJP::SymbolType::SYMBOL_BOOL) {
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });
                        generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
                    }
                    generatedCode.addInstruction({ FJP::OP_CODE::STO, symbolTable.getDepthLevel() - variable.level, variable.address });
                }
            }

            // <expression>
            processExpression();

            // If the identifier was a boolean, convert the result of the expression into 1/0.
            if (variable.symbolType == FJP::SymbolType::SYMBOL_BOOL) {
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
            }
            generatedCode.addInstruction({ FJP::OP_CODE::STO, symbolTable.getDepthLevel() - variable.level, variable.address });
            lastProcessVariable = variable;
            break;

        // If the symbol is an array.
        case FJP::SymbolType::SYMBOL_INT_ARRAY:
        case FJP::SymbolType::SYMBOL_BOOL_ARRAY:
            // '['
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::LEFT_SQUARED_BRACKET) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_17, ERR_CODE, token.lineNumber);
            }

            // <expression>
            token = lexer->getNextToken();
            processExpression();

            // Calculate the address of the element in the array (base + offset).
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, variable.address });
            generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS });

            // ']'
            if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_16, ERR_CODE, token.lineNumber);
            }

            // ':='
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::ASSIGN) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_14, ERR_CODE, token.lineNumber);
            }

            // <expression>
            token = lexer->getNextToken();
            processExpression();

            // If the identifier was a boolean, convert the result of the expression into 1/0.
            if (variable.symbolType == FJP::SymbolType::SYMBOL_BOOL_ARRAY) {
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
            }
            generatedCode.addInstruction({FJP::OP_CODE::STA, symbolTable.getDepthLevel() - variable.level, 0});
            break;
        default:
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_29, ERR_CODE, token.lineNumber);
            break;
    }

    // ';'
    if (expectSemicolon) {
        if (token.tokenType != FJP::TokenType::SEMICOLON) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
        }
        // Load up the next token, so it can be processed.
        token = lexer->getNextToken();
    }
    return true;
}

// <identifier> :
bool FJP::Parser::processLabel(const std::string label) {
    // We don't have to check if the name is already taken as it was done above.
    symbolTable.addSymbol({FJP::SymbolType::SYMBOL_LABEL, label, 0, 0, generatedCode.getSize(), 0});

    // ':'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::COLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_11, ERR_CODE, token.lineNumber);
    }

    // If the label has been on the list of undefined labels (labels yet to be declared).
    // Change the jump address of the goto instruction and delete the label from the list.
    auto itr = undefinedLabels.find(label);
    if (itr != undefinedLabels.end()) {
        for (auto index : itr->second) {
            generatedCode[index].m = generatedCode.getSize();
        }
        undefinedLabels.erase(label);
    }
    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}

// call <identifier>();
bool FJP::Parser::processCall() {
    // 'call'
    if (token.tokenType != FJP::TokenType::CALL) {
        return false;
    }

    // <identifier>
    token = lexer->getNextToken();
    Symbol function = symbolTable.findSymbol(token.value);

    // Make sure the symbol exists in the symbol table.
    if (function.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_45, ERR_CODE, token.lineNumber);
    }

    // Make sure the identifier is a function.
    if (function.symbolType != FJP::SymbolType::SYMBOL_FUNCTION) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_23, ERR_CODE, token.lineNumber);
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    // ')'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // ';'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
    }

    // Create a JMP instruction to jump to the first address of the function.
    generatedCode.addInstruction({FJP::OP_CODE::CAL, symbolTable.getDepthLevel() - function.level, function.value});

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}

// { <statement>* }
bool FJP::Parser::processScope() {
    // '{'
    if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
        return false;
    }
    token = lexer->getNextToken();
    do {
        // Keep processing statements until you come across with a '}'
        processStatement();
    } while (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET);

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}

// if ( <condition> ) <statement>
// if ( <condition> ) <statement> else <statement>
bool FJP::Parser::processIf() {
    // 'if'
    if (token.tokenType != TokenType::IF) {
        return false;
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    // <condition>
    token = lexer->getNextToken();
    processCondition();

    // Insert a JPC instruction (conditional jump).
    // If the result of the condition is false, we'll just skip the body of the if statement.
    int currentInstruction1 = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0});

    // ')'
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // <statement>
    token = lexer->getNextToken();
    processStatement();

    // else
    if (token.tokenType == FJP::TokenType::ELSE) {
        // Add a JMP instruction, so we can skip the else branch in case the condition is satisfied.
        int currentInstruction2 = generatedCode.getSize();
        generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});

        // Set the address of the JPC instruction to jump to
        // in case the condition isn't satisfied - the else branch.
        generatedCode[currentInstruction1].m = generatedCode.getSize();

        // <statement>
        token = lexer->getNextToken();
        processStatement();

        // Set the address to jump to in case the condition is satisfied - skip the else branch.
        generatedCode[currentInstruction2].m = generatedCode.getSize();
    } else {
        // Set the address of the JPC instruction - skip the body of the if statement.
        generatedCode[currentInstruction1].m = generatedCode.getSize();
    }
    return true;
}

// while ( <condition> ) <statement>
bool FJP::Parser::processWhile() {
    // while
    if (token.tokenType != FJP::TokenType::WHILE) {
        return false;
    }

    // Start address of the while loop.
    int startWhileLoop = generatedCode.getSize();

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    // <condition>
    token = lexer->getNextToken();
    processCondition();
    int endOfWhileCondition = generatedCode.getSize();

    // ')'
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // Add a JPC instruction (conditional jump) in order to skip the body of the
    // while loop in case the condition is not satisfied.
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0 });

    // <statement>
    token = lexer->getNextToken();
    processStatement();

    // Jump back to the condition of the while loop.
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, startWhileLoop});

    // Set the target jump address of the JPC instruction (in order to skip the body of the loop).
    generatedCode[endOfWhileCondition].m = generatedCode.getSize();
    return true;
}

// do { <statement> } while ( <condition> );
bool FJP::Parser::processDoWhile() {
    // do
    if (token.tokenType != FJP::TokenType::DO) {
        return false;
    }
    // Start address of the body of the do-while loop.
    int doWhileStart = generatedCode.getSize();

    // '{'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_18, ERR_CODE, token.lineNumber);
    }

    // <statement>
    token = lexer->getNextToken();
    processStatement();

    // '}'
    if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_19, ERR_CODE, token.lineNumber);
    }

    // 'while'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::WHILE) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_38, ERR_CODE, token.lineNumber);
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // <condition>
    token = lexer->getNextToken();
    processCondition();

    // If the result of condition is true (1) jump to the first address
    // of the body of the do-while loop.
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_EQ});
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, doWhileStart });

    // ')'
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // ';'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
    }

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}

// repeat <statement> until ( <condition> );
bool FJP::Parser::processRepeatUntil() {
    // repeat
    if (token.tokenType != FJP::TokenType::REPEAT) {
        return false;
    }

    // Start address of the body of the repeat-until loop.
    int repeatUntilStart = generatedCode.getSize();

    // '{'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_18, ERR_CODE, token.lineNumber);
    }

    // <statement>
    token = lexer->getNextToken();
    processStatement();

    // '}'
    if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_19, ERR_CODE, token.lineNumber);
    }

    // until
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::UNTIL) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_39, ERR_CODE, token.lineNumber);
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    // <condition>
    token = lexer->getNextToken();
    processCondition();

    // If the result of condition is false (0) jump to the first address
    // of the body of the repeat-until loop.
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, repeatUntilStart });

    // ')'
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // ';'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
    }

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}

// foreach (<identifier> : <identifier) <statement>
bool FJP::Parser::processForeach() {
    // foreach
    if (token.tokenType != FJP::TokenType::FOREACH) {
        return false;
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    // <identifier>
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_24, ERR_CODE, token.lineNumber);
    }

    // Make sure the identifier exists in the symbol table.
    FJP::Symbol iterVariable = symbolTable.findSymbol(token.value);
    if (iterVariable.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_45, ERR_CODE, token.lineNumber);
    }

    // Make sure the identifier is one of the supported data types.
    switch (iterVariable.symbolType) {
        case FJP::SymbolType::SYMBOL_INT:
        case FJP::SymbolType::SYMBOL_BOOL:
            break;
        default:
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_32, ERR_CODE, token.lineNumber);
    }

    // ':'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::COLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_11, ERR_CODE, token.lineNumber);
    }

    // <identifier>
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_24, ERR_CODE, token.lineNumber);
    }

    // Make sure the identifier exists in the symbol table.
    FJP::Symbol dataArray = symbolTable.findSymbol(token.value);
    if (dataArray.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_09, ERR_CODE, token.lineNumber);
    }

    switch (iterVariable.symbolType) {
        // If the type of the iterator is an integer. The array has to be an array of integers.
        case FJP::SymbolType::SYMBOL_INT:
            if (dataArray.symbolType != FJP::SymbolType::SYMBOL_INT_ARRAY) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_42, ERR_CODE, token.lineNumber);
            }
            break;

        // If the type of the iterator is bool. The array has to be an array of booleans as well.
        case FJP::SymbolType::SYMBOL_BOOL:
            if (dataArray.symbolType != FJP::SymbolType::SYMBOL_BOOL_ARRAY) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_42, ERR_CODE, token.lineNumber);
            }
            break;
        default:
            break;
    }

    // ')'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();

    // Create a temporary "variable" on the stack that will be used to store
    // the current index as we iterate through the array.
    int indexAddress = nextFreeAddress;
    generatedCode.addInstruction({FJP::OP_CODE::INC, 0, 1});

    // Initialize the current index to 0 (the first element of the array).
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
    generatedCode.addInstruction({FJP::OP_CODE::STO, 0, indexAddress});

    // Start of the condition of the foreach loop (checking whether we've reached
    // the end of the array or not).
    int startForeachBody = generatedCode.getSize();

    // Check if we have just reached the end of the array (current index == array.size).
    generatedCode.addInstruction({FJP::OP_CODE::LOD, 0, indexAddress});
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, dataArray.size});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ});

    // Skip the body of the foreach loop if the end of the array has been reached.
    int exitForeachAddress = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0});

    // Load array[index] to the top of the stack. We need to take the base address
    // of the array and add the current index to it, so we get the address of
    // the current element.
    generatedCode.addInstruction({FJP::OP_CODE::LOD, 0, indexAddress});
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, dataArray.address});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
    generatedCode.addInstruction({FJP::OP_CODE::LDA, symbolTable.getDepthLevel() - dataArray.level, 0});

    // Store the value (array[index]) into the iterator.
    generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - iterVariable.level, iterVariable.address});

    // Increment the current index (moving on to the next element).
    generatedCode.addInstruction({FJP::OP_CODE::LOD, 0, indexAddress});
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 1});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
    generatedCode.addInstruction({FJP::OP_CODE::STO, 0, indexAddress});

    // <statement>
    processStatement();

    // Jump to the incrementation part of the loop (index++).
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, startForeachBody});

    // Set the address to jump to in case the condition is not satisfied (end of the foreach loop).
    generatedCode[exitForeachAddress].m = generatedCode.getSize();

    // Remove the temporary variable (index) off the stack.
    generatedCode.addInstruction({FJP::OP_CODE::INC, 0, -1});
    return true;
}

// for ( <assignment> ; <condition> ; <assignment> ) <statement>
bool FJP::Parser::processFor() {
    // for
    if (token.tokenType != FJP::TokenType::FOR) {
        return false;
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    // <assignment>
    token = lexer->getNextToken();
    processAssignment();

    // Store the start address of the condition of the for loop;
    int startCondition = generatedCode.getSize();

    // <condition>
    processCondition();

    // Generate a JPC instruction to skip the body of the for loop if the
    // condition is not satisfied. Also, create a JMP instruction in order to
    // skip the incrementation part of the for loop - we need to jump straight
    // to the body of the loop.
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0 });
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});

    // Store the start address of the update part of the for loop (e.g. i++).
    int startUpdatePart = generatedCode.getSize();

    // ';'
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
    }

    // <assignment>
    token = lexer->getNextToken();
    processAssignment(false);

    // After the second assignment (incrementation part), we need to jump back to the condition.
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, startCondition});

    // Store the address of the end of the incrementation part of the for loop.
    int endUpdatePart = generatedCode.getSize();

    // ')'
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // <statement>
    token = lexer->getNextToken();
    processStatement();

    // Once the body has been executed, jump straight to the update part (incrementation part).
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, startUpdatePart});

    // Set the address to jump to in case the condition isn't satisfied (end of the loop).
    generatedCode[startUpdatePart - 2].m = generatedCode.getSize();

    // Set the address to jump to after the condition has been processed (the body of the for loop).
    generatedCode[startUpdatePart - 1].m = endUpdatePart;
    return true;
}

// switch ( <identifier> ) { <cases> }
bool FJP::Parser::processSwitch() {
    // List of the address of break statements that will need to be set
    // once the end address is known (the end of the switch statement).
    std::list<int> breaks;

    // switch
    if (token.tokenType != FJP::TokenType::SWITCH) {
        return false;
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    // <identifier>
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_44, ERR_CODE, token.lineNumber);
    }

    // Make sure the identifier exists in the symbol table.
    Symbol variable = symbolTable.findSymbol(token.value);
    if (variable.symbolType == FJP::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_45, ERR_CODE, token.lineNumber);
    }

    // ')'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // '{'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_18, ERR_CODE, token.lineNumber);
    }

    // <cases>
    token = lexer->getNextToken();
    processCases(variable, breaks);

    // '}'
    if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_19, ERR_CODE, token.lineNumber);
    }

    // Set the jump address of all the break statements. The jumps address
    // is the end of the switch statement.
    for (const auto &item : breaks) {
        generatedCode[item].m = generatedCode.getSize();
    }

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}


// case <literal> : <statement> (<break>;)?
void FJP::Parser::processCases(Symbol &variable, std::list<int> &breaks) {
    // case
    if (token.tokenType != FJP::TokenType::CASE) {
        return;
    }

    // Check if the token is indeed a literal (number, true, false).
    token = lexer->getNextToken();
    int token_value;
    if (token.tokenType != FJP::TokenType::NUMBER &&
        token.tokenType != FJP::TokenType::TRUE &&
        token.tokenType != FJP::TokenType::FALSE) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_34, ERR_CODE, token.lineNumber);
    }

    // Parse the literal. If it is a number, convert it into an integer, and if
    // it is a boolean, convert it into 1/0.
    if (token.tokenType == FJP::TokenType::NUMBER) {
        token_value = atoi(token.value.c_str());
    } else {
        token_value = token.value == "true";
    }

    // Make sure the literal is the same datatype as the variable used in the switch statement.
    if (((variable.symbolType == SYMBOL_INT) && token.tokenType != NUMBER) ||
        ((variable.symbolType == SYMBOL_BOOL) && !(token.tokenType == TRUE || token.tokenType == FALSE))) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_35, ERR_CODE, token.lineNumber);
    }

    // ':'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::COLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_11, ERR_CODE, token.lineNumber);
    }

    // Compare the literal against the variable used in the switch statement.
    generatedCode.addInstruction({FJP::OP_CODE::LOD, symbolTable.getDepthLevel() - variable.level, variable.address});
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, token_value});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_EQ});

    // If it doesn't match up, skip the body of the case statement and jump onto the next case.
    int jpc_address = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0});

    // <statement>
    token = lexer->getNextToken();
    processStatement();

    // Set the jump address to jump to in case the literal doesn't mach the value of the variable
    // used in the switch statement (jump onto the next case).
    generatedCode[jpc_address].m = generatedCode.getSize();

    // <break>
    if (token.tokenType == FJP::TokenType::BREAK) {
        // Add the address of the break statement to the list of all breaks.
        // The jump address will be known after the entire switch has been processed.
        breaks.push_back(generatedCode.getSize());
        generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});

        // ';'
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::SEMICOLON) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
        }

        // Also, we need to skip the break itself in case we're jumping onto the next case.
        token = lexer->getNextToken();
        generatedCode[jpc_address].m++;
    }

    // Process the next statement.
    processCases(variable, breaks);
}

// goto <identifier> ;
bool FJP::Parser::processGoto() {
    // goto
    if (token.tokenType != FJP::TokenType::GOTO) {
        return false;
    }

    // <identifier>
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_24, ERR_CODE, token.lineNumber);
    }

    // Find the identifier in the symbol table.
    FJP::Symbol symbol = symbolTable.findSymbol(token.value);

    // If the identifier is not found, added it to the list
    // of labels that are still yet be declared (later on).
    // If the identifier is found, make sure its type is label.
    if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        undefinedLabels[token.value].push_back(generatedCode.getSize());
    } else if (symbol.symbolType != FJP::SymbolType::SYMBOL_LABEL) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_33, ERR_CODE, token.lineNumber);
    }

    // ';'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
    }

    // Added a JMP instruction to jump to the address of the label.
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, symbol.address});

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}

// ! <expression>
// <expression> == <expression>
// <expression> != <expression>
// <expression> < <expression>
// <expression> <= <expression>
// <expression> > <expression>
// <expression> => <expression>
// <expression> && <expression>
// <expression> || <expression>
void FJP::Parser::processCondition() {
    // '!'
    if (token.tokenType == FJP::TokenType::EXCLAMATION_MARK) {
        // <expression>
        token = lexer->getNextToken();
        processExpression();

        // Compare the result of the expression to 0, and store the result
        // of the comparison on the top of the stack.
        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
        generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_EQ});
    } else {
        // <expression>
        processExpression();

        FJP::OPRType instructionType = FJP::OPRType::OPR_RET;
        FJP::TokenType logicalOperation = FJP::TokenType::UNKNOWN;
        switch (token.tokenType) {
            // '=='
            case FJP::TokenType::EQUALS:
                instructionType = FJP::OPRType::OPR_EQ;
                break;
            // '!='
            case FJP::TokenType::NOT_EQUALS:
                instructionType = FJP::OPRType::OPR_NEQ;
                break;
            // '<'
            case FJP::TokenType::LESS:
                instructionType = FJP::OPRType::OPR_LESS;
                break;
            // '<='
            case FJP::TokenType::LESS_OR_EQUAL:
                instructionType = FJP::OPRType::OPR_LESS_EQ;
                break;
            // '>'
            case FJP::TokenType::GREATER:
                instructionType = FJP::OPRType::OPR_GRT;
                break;
            // '>='
            case FJP::TokenType::GREATER_OR_EQUAL:
                instructionType = FJP::OPRType::OPR_GRT_EQ;
                break;
            // '&&'
            case FJP::TokenType::LOGICAL_AND:
            // '||'
            case FJP::TokenType::LOGICAL_OR:
                logicalOperation = token.tokenType;
                break;
            default:
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_28, ERR_CODE, token.lineNumber);
        }

        // <expression>
        token = lexer->getNextToken();
        processExpression();

        // Based on the type of comparison, person an operation
        // wit the two values (the results of the two expressions)
        // on the top of the stack. The result is going to be 1/0.
        switch (logicalOperation) {
            // If the operation is something other than '&&' or '||'.
            case FJP::TokenType::UNKNOWN:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, instructionType});
                break;
            // '&&' - it needs to be done only by the operations supported by PL0
            case FJP::TokenType::LOGICAL_AND:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_MUL});
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ});
                break;
            // '||' - it needs to be done only by the operations supported by PL0
            case FJP::TokenType::LOGICAL_OR:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ});
                break;
            default:
                break;
        }
    }
}

// <identifier> := # <condition> ? <expression> : <expression> ;
// note: everything up to <condition> has been parsed in processExpression()
void FJP::Parser::processTernaryOperator() {
    // <condition>
    token = lexer->getNextToken();
    processCondition();

    // '?'
    if (token.tokenType != FJP::TokenType::QUESTION_MARK) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_20, ERR_CODE, token.lineNumber);
    }

    // Add JMP instructions, so we can jump to the appropriate
    // expression based on the result of the condition.
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, generatedCode.getSize() + 2});
    int jmpInstructionAddress = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});
    int jmp2InstructionAddress = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});

    // <expression>
    token = lexer->getNextToken();
    processExpression();

    // ':'
    if (token.tokenType != FJP::TokenType::COLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_11, ERR_CODE, token.lineNumber);
    }

    // Set the address of the first expression (it's now known since it's been parsed).
    generatedCode[jmpInstructionAddress].m = generatedCode.getSize();

    // <expression>
    token = lexer->getNextToken();
    processExpression();

    // Set the address of the second expression (it's now known since it's been parsed).
    generatedCode[jmp2InstructionAddress].m = generatedCode.getSize();
}

// # <ternary operator>
// (+-)? <term> (+-)? <term> (+-)? <term> ...
void FJP::Parser::processExpression() {
    // '#' -> treat it as a ternary operator
    if (token.tokenType == FJP::TokenType::HASH_MARK) {
        processTernaryOperator();
        return;
    }

    FJP::TokenType currTokenType;

    // '+' / '-'
    if (token.tokenType == FJP::TokenType::PLUS || token.tokenType == FJP::TokenType::MINUS) {
        currTokenType = token.tokenType;

        // <term>
        token = lexer->getNextToken();
        processTerm();

        // If the sign is a minus, invert the value on the top of the stack.
        if (currTokenType == FJP::TokenType::MINUS) {
            generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_INVERT_VALUE});
        }
    } else {
        // <term>
        processTerm();
    }
    while (token.tokenType == FJP::TokenType::PLUS || token.tokenType == FJP::TokenType::MINUS) {
        currTokenType = token.tokenType;

        // <term>
        token = lexer->getNextToken();
        processTerm();

        // Add instruction to cary out the operation on top of the stack.
        switch (currTokenType) {
            // '+'
            case FJP::TokenType::PLUS:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
                break;
            // '-'
            case FJP::TokenType::MINUS:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_MINUS});
                break;
            default:
                break;
        }
    }
}

// <factor> * / <factor>
void FJP::Parser::processTerm() {
    FJP::TokenType currTokenType;

    // <factor>
    processFactor();

    // * /
    while (token.tokenType == FJP::TokenType::ASTERISK || token.tokenType == FJP::TokenType::SLASH) {
        currTokenType = token.tokenType;

        // <factor>
        token = lexer->getNextToken();
        processFactor();

        // Add instruction to perform the operation on the top of the stack.
        switch (currTokenType) {
            // '*'
            case FJP::TokenType::ASTERISK:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_MUL});
                break;
            // '/'
            case FJP::TokenType::SLASH:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_DIV});
                break;
            default:
                break;
        }
    }
}

void FJP::Parser::processFactor() {
    FJP::Symbol symbol;
    int numberValue;
    bool nextToken = true;

    switch (token.tokenType) {
        // <identifier>
        case FJP::TokenType::IDENTIFIER:
            // Make sure the identifier exists within the symbol table.
            symbol = symbolTable.findSymbol(token.value);
            if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_45, ERR_CODE, token.lineNumber);
            }
            // Check if the next token is 'instanceof'
            token = lexer->getNextToken();
            if (token.tokenType == FJP::TokenType::INSTANCEOF) {
                token = lexer->getNextToken();
                switch (token.tokenType) {
                    // instanceof 'int'
                    case FJP::TokenType::INT:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.symbolType == FJP::SYMBOL_INT});
                        break;
                    // instanceof 'bool'
                    case FJP::TokenType::BOOL:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.symbolType == FJP::SYMBOL_BOOL});
                        break;
                    // instanceof 'int[]'
                    case FJP::TokenType::INT_ARRAY:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.symbolType == FJP::SYMBOL_INT_ARRAY});
                        break;
                    // instanceof 'bool[]'
                    case FJP::TokenType::BOOL_ARRAY:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.symbolType == FJP::SYMBOL_BOOL_ARRAY});
                        break;
                    // instanceof 'function'
                    case FJP::TokenType::FUNCTION:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.symbolType == FJP::SYMBOL_FUNCTION});
                        break;
                    default:
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_31, ERR_CODE, token.lineNumber);
                        break;
                }
            } else {
                // Do not load up the next token.
                nextToken = false;

                // Check the type of the identifier
                switch (symbol.symbolType) {
                    // <int/bool>
                    case FJP::SymbolType::SYMBOL_INT:
                    case FJP::SymbolType::SYMBOL_BOOL:
                        generatedCode.addInstruction({FJP::OP_CODE::LOD, symbolTable.getDepthLevel() - symbol.level, symbol.address});
                        break;
                    // <const>
                    case FJP::SymbolType::SYMBOL_CONST:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.value});
                        break;
                    // <int[]/bool[]>
                    case FJP::SymbolType::SYMBOL_INT_ARRAY:
                    case FJP::SymbolType::SYMBOL_BOOL_ARRAY:
                        // '['
                        if (token.tokenType != FJP::TokenType::LEFT_SQUARED_BRACKET) {
                            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_17, ERR_CODE, token.lineNumber);
                        }

                        // <expression>
                        token = lexer->getNextToken();
                        processExpression();

                        // Calculate the address of the element within the array. Add up the base address and the index.
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.address});
                        generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});

                        // Load the value to the top of the stack.
                        generatedCode.addInstruction({FJP::OP_CODE::LDA, symbolTable.getDepthLevel() - symbol.level, 0});

                        // ']'
                        if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_16, ERR_CODE, token.lineNumber);
                        }
                        token = lexer->getNextToken();
                        break;
                    default:
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_29, ERR_CODE, token.lineNumber);
                        break;
                }
            }
            break;
        // <number>
        case FJP::TokenType::NUMBER:
            numberValue = atoi(token.value.c_str());
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, numberValue});
            break;
        // <true/false>
        case FJP::TokenType::TRUE:
        case FJP::TokenType::FALSE:
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, token.tokenType == FJP::TokenType::TRUE});
            break;
        // '('
        case FJP::TokenType::LEFT_PARENTHESIS:
            // <expression>
            token = lexer->getNextToken();
            processExpression();
            // ')'
            if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
            }
            break;
        default:
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_22, ERR_CODE, token.lineNumber);
            break;
    }
    if (nextToken == true) {
        token = lexer->getNextToken();
    }
}

bool FJP::Parser::processRead() {
    // 'read'
    if (token.tokenType != FJP::TokenType::READ) {
        return false;
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    // <identifier>
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_44, ERR_CODE, token.lineNumber);
    }

    // Make sure that the identifier exists in the symbol table.
    FJP::Symbol symbol = symbolTable.findSymbol(token.value);
    if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_45, ERR_CODE, token.lineNumber);
    }

    // Check out the type of the identifier.
    switch (symbol.symbolType) {
        // int
        case FJP::SymbolType::SYMBOL_INT:
            generatedCode.addInstruction({FJP::OP_CODE::SIO, 0, FJP::SIO_TYPE::SIO_READ});
            generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - symbol.level, symbol.address});
            break;
        // bool
        case FJP::SymbolType::SYMBOL_BOOL:
            generatedCode.addInstruction({FJP::OP_CODE::SIO, 0, FJP::SIO_TYPE::SIO_READ});
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });

            // Convert an integer into a boolean (1/0).
            generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
            generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - symbol.level, symbol.address});
            break;
        // array
        case FJP::SymbolType::SYMBOL_INT_ARRAY:
        case FJP::SymbolType::SYMBOL_BOOL_ARRAY:
            // '['
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::LEFT_SQUARED_BRACKET) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_17, ERR_CODE, token.lineNumber);
            }

            // <expression>
            token = lexer->getNextToken();
            processExpression();

            // Calculate the address of the element (base + offset).
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.address });
            generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS });
            generatedCode.addInstruction({FJP::OP_CODE::SIO, 0, FJP::SIO_TYPE::SIO_READ});

            // If it is an array of booleans, interpret the numbers as booleans (1/0).
            if (symbol.symbolType == FJP::SymbolType::SYMBOL_BOOL_ARRAY) {
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
            }

            // ']'
            generatedCode.addInstruction({FJP::OP_CODE::STA, symbolTable.getDepthLevel() - symbol.level, 0});
            if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_16, ERR_CODE, token.lineNumber);
            }
            break;
        default:
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_41, ERR_CODE, token.lineNumber);
            break;
    }
    // ')'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // ';'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
    }

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}

bool FJP::Parser::processWrite() {
    // 'write'
    if (token.tokenType != FJP::TokenType::WRITE) {
        return false;
    }

    // '('
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_12, ERR_CODE, token.lineNumber);
    }

    int number;
    FJP::Symbol symbol;

    token = lexer->getNextToken();
    switch (token.tokenType) {
        // <identifier>
        case FJP::TokenType::IDENTIFIER:
            // Make sure that the identifier has been declared (exists in the symbol table).
            symbol = symbolTable.findSymbol(token.value);
            if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
                FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_45, ERR_CODE, token.lineNumber);
            }

            // Check out the type of the identifier.
            switch (symbol.symbolType) {
                // <int/bool>
                case FJP::SymbolType::SYMBOL_INT:
                case FJP::SymbolType::SYMBOL_BOOL:
                    generatedCode.addInstruction({FJP::OP_CODE::LOD, symbolTable.getDepthLevel() - symbol.level, symbol.address});
                    break;

                // <int[]/bool[]>
                case FJP::SymbolType::SYMBOL_INT_ARRAY:
                case FJP::SymbolType::SYMBOL_BOOL_ARRAY:
                    // '['
                    token = lexer->getNextToken();
                    if (token.tokenType != FJP::TokenType::LEFT_SQUARED_BRACKET) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_17, ERR_CODE, token.lineNumber);
                    }

                    // <expression>
                    token = lexer->getNextToken();
                    processExpression();

                    // Calculate the address of the element (base + offset).
                    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.address});
                    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
                    generatedCode.addInstruction({FJP::OP_CODE::LDA, symbolTable.getDepthLevel() - symbol.level, 0});

                    // ']'
                    if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_16, ERR_CODE, token.lineNumber);
                    }
                    break;
                // <const>
                case FJP::SymbolType::SYMBOL_CONST:
                    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.value});
                    break;
                default:
                    FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_41, ERR_CODE, token.lineNumber);
                    break;
            }
            break;
        // <number>
        case FJP::TokenType::NUMBER:
            number = atoi(token.value.c_str());
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, number});
            break;
        // <true/false>
        case FJP::TokenType::TRUE:
        case FJP::TokenType::FALSE:
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, token.tokenType == FJP::TokenType::TRUE});
            break;
        default:
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_30, ERR_CODE, token.lineNumber);
            break;
    }

    // Print off the value from the top of the stack.
    generatedCode.addInstruction({FJP::OP_CODE::SIO, 0, FJP::SIO_TYPE::SIO_WRITE});

    // ')'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_13, ERR_CODE, token.lineNumber);
    }

    // ';'
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_15, ERR_CODE, token.lineNumber);
    }

    // Load up the next token, so it can be processed.
    token = lexer->getNextToken();
    return true;
}