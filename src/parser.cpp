#include <cassert>
#include <fstream>
#include <iomanip>

#include "logger.h"
#include "isa.h"
#include "parser.h"
#include "errors.h"

FJP::Parser *FJP::Parser::instance = nullptr;

FJP::Parser* FJP::Parser::getInstance() {
    if (instance == NULL) {
        instance = new Parser;
    }
    return instance;
}

FJP::Parser::Parser() : lexer(nullptr), arSize(0) {
}

FJP::GeneratedCode FJP::Parser::parse(FJP::Lexer *lexer, bool debug) {
    assert(lexer != nullptr);

    this->lexer = lexer;
    generatedCode = FJP::GeneratedCode();

    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::START) {
        FJP::exitProgramWithError("missing START symbol", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    processBlock();
    if (token.tokenType != FJP::TokenType::END) {
        FJP::exitProgramWithError("missing END symbol", ERR_CODE, token.lineNumber);
    }

    if (debug == true) {
        storeCodeInstructionsIntoFile();
    }
    return generatedCode;
}

void FJP::Parser::storeCodeInstructionsIntoFile() {
    std::ofstream file = std::ofstream (std::string(OUTPUT_FILE));

    if (file.is_open() == false) {
        FJP::exitProgramWithError(FJP::IOErrors::ERROR_01, ERR_CODE);
        return;
    }
    for (int i = 0; i < generatedCode.getSize(); i++) {
        file << "[#" << std::setw(ADDRESS_LEN) << std::setfill('0') << i << "] " << generatedCode[i];
        if (i < generatedCode.getSize() - 1) {
            file << "\n";
        }
    }
    file.close();
}

void FJP::Parser::processBlock() {
    int frameVariableCount = FRAME_INIT_VAR_COUNT;
    arSize = FRAME_INIT_VAR_COUNT;

    symbolTable.createFrame();
    generatedCode.addInstruction({FJP::OP_CODE::INC, 0, 0});
    int incInstructionAddress = generatedCode.getSize() - 1;

    processConst();
    processVariable(frameVariableCount);
    int jmpAddress = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});

    processFunction();
    generatedCode[jmpAddress].m = generatedCode.getSize();
    generatedCode[incInstructionAddress].m = frameVariableCount;

    processStatement();


    arSize -= FRAME_INIT_VAR_COUNT;
    symbolTable.destroyFrame();
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_RET});
}

void FJP::Parser::processConst() {
    while (token.tokenType == FJP::TokenType::CONST) {
        token = lexer->getNextToken();

        int value;
        std::string identifier;
        FJP::TokenType dataType = token.tokenType;

        if (!(dataType == FJP::TokenType::INT || dataType == FJP::TokenType::BOOL)) {
            FJP::exitProgramWithError("invalid datatype", ERR_CODE, token.lineNumber);
        }

        while (true) {
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::IDENTIFIER) {
                FJP::exitProgramWithError("missing identifier", ERR_CODE, token.lineNumber);
                return;
            }
            identifier = token.value;
            if (symbolTable.existsSymbol(identifier) == true) {
                FJP::exitProgramWithError("token name is already taken", ERR_CODE, token.lineNumber);
                return;
            }

            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::INIT_SIGN) {
                FJP::exitProgramWithError("missing '='", ERR_CODE, token.lineNumber);
                return;
            }
            token = lexer->getNextToken();

            switch (dataType) {
                case FJP::TokenType::INT:
                    if (token.tokenType != FJP::TokenType::NUMBER) {
                        FJP::exitProgramWithError("a number expected", ERR_CODE, token.lineNumber);
                    }
                    value = atoi(token.value.c_str());
                    symbolTable.addSymbol({FJP::SymbolType::SYMBOL_CONST, identifier, value, 0, 0, 0});
                    break;
                case FJP::TokenType::BOOL:
                    if (!(token.tokenType == FJP::TokenType::TRUE || token.tokenType == FJP::TokenType::FALSE)) {
                        FJP::exitProgramWithError("a true/false value expected", ERR_CODE, token.lineNumber);
                    }
                    symbolTable.addSymbol({FJP::SymbolType::SYMBOL_CONST, identifier, token.tokenType == FJP::TokenType::TRUE, 0, 0, 0});
                    break;
                default:
                    break;
            }
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::COMMA)
                break;
        }
        if (token.tokenType != FJP::TokenType::SEMICOLON) {
            FJP::exitProgramWithError("missing ';'", ERR_CODE, token.lineNumber);
        }
        token = lexer->getNextToken();
    }
}

void FJP::Parser::processVariable(int &frameVariableCount) {
    FJP::TokenType dataType = token.tokenType;

    if (!(dataType == FJP::TokenType::INT || dataType == FJP::TokenType::BOOL)) {
        return;
    }

    int arrayAddress;
    int arraySize = 0;
    int arrayDepth;
    int number;
    std::string identifier;
    FJP::Symbol symbol;

    while (true) {
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::IDENTIFIER) {
            FJP::exitProgramWithError("missing identifier", ERR_CODE, token.lineNumber);
        }
        identifier = token.value;
        if (symbolTable.existsSymbol(identifier) ==  true) {
            FJP::exitProgramWithError("token name is already taken", ERR_CODE, token.lineNumber);
        }

        switch (dataType) {
            case FJP::TokenType::INT:
                symbolTable.addSymbol({FJP::SymbolType::SYMBOL_INT, identifier, DEFAULT_INT_VALUE, symbolTable.getDepthLevel(), arSize, 0 });
                break;
            case FJP::TokenType::BOOL:
                symbolTable.addSymbol({FJP::SymbolType::SYMBOL_BOOL, identifier, DEFAULT_BOOL_VALUE, symbolTable.getDepthLevel(), arSize, 0 });
                break;
            default:
                break;
        }
        arSize++;
        frameVariableCount++;
        arrayDepth = symbolTable.getDepthLevel();

        token = lexer->getNextToken();
        if (token.tokenType == FJP::TokenType::LEFT_SQUARED_BRACKET) {
            arrayAddress = arSize - 1;
            token = lexer->getNextToken();

            switch (token.tokenType) {
                case FJP::TokenType::NUMBER:
                    arraySize = atoi(token.value.c_str());
                    if (arraySize <= 0) {
                        FJP::exitProgramWithError("size has to be >= 1", ERR_CODE, token.lineNumber);
                    }
                    arSize += (arraySize - 1);
                    frameVariableCount += (arraySize - 1);
                    symbolTable.makeArray(identifier, arraySize);
                    break;
                case FJP::TokenType::IDENTIFIER:
                    symbol = symbolTable.findSymbol(token.value);
                    if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
                        FJP::exitProgramWithError("symbol not found", ERR_CODE, token.lineNumber);
                    }
                    if (symbol.symbolType != FJP::SymbolType::SYMBOL_CONST) {
                        FJP::exitProgramWithError("an array can be initialized only with a const value", ERR_CODE, token.lineNumber);
                    }
                    if (symbol.value < 1) {
                        FJP::exitProgramWithError("size has to be >= 1", ERR_CODE, token.lineNumber);
                    }
                    arSize += (symbol.value - 1);
                    frameVariableCount += (symbol.value - 1);
                    symbolTable.makeArray(identifier, symbol.value);
                    arraySize = symbol.value;
                    break;
                default:
                    FJP::exitProgramWithError("invalid array size", ERR_CODE, token.lineNumber);
                    break;
            }

            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                FJP::exitProgramWithError("missing ]", ERR_CODE, token.lineNumber);
            }
            token = lexer->getNextToken();

            if (token.tokenType == FJP::TokenType::INIT_SIGN) {
                token = lexer->getNextToken();
                if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
                    FJP::exitProgramWithError("invalid initialization - missing {", ERR_CODE, token.lineNumber);
                }
                for (int i = 0; i < arraySize; i++) {
                    token = lexer->getNextToken();

                    switch (dataType) {
                        case FJP::TokenType::INT:
                            if (token.tokenType != FJP::TokenType::NUMBER) {
                                FJP::exitProgramWithError("invalid initialization - number expected", ERR_CODE, token.lineNumber);
                            }
                            number = atoi(token.value.c_str());
                            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, number});
                            generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - arrayDepth, arrayAddress + i});
                            break;
                        case FJP::TokenType::BOOL:
                            if (!(token.tokenType == FJP::TokenType::TRUE || token.tokenType == FJP::TokenType::FALSE)) {
                                FJP::exitProgramWithError("invalid initialization - bool value expected", ERR_CODE, token.lineNumber);
                            }
                            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, token.tokenType == FJP::TokenType::TRUE});
                            generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - arrayDepth, arrayAddress + i});
                            break;
                        default:
                            break;
                    }
                    token = lexer->getNextToken();
                    if (token.tokenType != FJP::TokenType::COMMA && i < arraySize - 1) {
                        FJP::exitProgramWithError("array has to be fully initialized", ERR_CODE, token.lineNumber);
                    }
                }
                if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
                    FJP::exitProgramWithError("invalid initialization - missing }", ERR_CODE, token.lineNumber);
                }
                token = lexer->getNextToken();
            }
        }
        if (token.tokenType != FJP::TokenType::COMMA) {
            break;
        }
    }
    if (token.tokenType !=  FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError("missing ';'", ERR_CODE, token.lineNumber);
        return;
    }
    token = lexer->getNextToken();
    processVariable(frameVariableCount);
}

void FJP::Parser::processFunction() {
    std::string identifier;
    while (token.tokenType == FJP::TokenType::FUNCTION) {
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::IDENTIFIER) {
            FJP::exitProgramWithError("missing identifier", ERR_CODE, token.lineNumber);
        }
        identifier = token.value;
        if (symbolTable.existsSymbol(identifier) ==  true) {
            FJP::exitProgramWithError("token name is already taken", ERR_CODE, token.lineNumber);
        }
        symbolTable.addSymbol({FJP::SymbolType::SYMBOL_FUNCTION, token.value, generatedCode.getSize(), symbolTable.getDepthLevel(), arSize, 0});

        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
            FJP::exitProgramWithError("missing (", ERR_CODE, token.lineNumber);
        }
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
            FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
        }
        token = lexer->getNextToken();
        if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
            FJP::exitProgramWithError("missing {", ERR_CODE, token.lineNumber);
        }
        token = lexer->getNextToken();
        processBlock();

        if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
            FJP::exitProgramWithError("missing }", ERR_CODE, token.lineNumber);
        }
        token = lexer->getNextToken();
    }
}

void FJP::Parser::processStatement() {
    processNop();
    processAssignment();
    processCall();
    processScope();
    processIf();
    processWhile();
    processDoWhile();
    processFor();
    processRepeatUntil();
    processForeach();
    processGoto();
    processRead();
    processWrite();
}

void FJP::Parser::processAssignment(bool expectSemicolon) {
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        return;
    }

    FJP::Symbol variable = symbolTable.findSymbol(token.value);
    if (variable.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        processLabel(token.value);
        return;
    }

    switch (variable.symbolType) {
        case FJP::SymbolType::SYMBOL_INT:
        case FJP::SymbolType::SYMBOL_BOOL:
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::ASSIGN) {
                FJP::exitProgramWithError(":= expected", ERR_CODE, token.lineNumber);
            }
            token = lexer->getNextToken();
            processExpression();

            if (variable.symbolType == FJP::SymbolType::SYMBOL_BOOL) {
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
            }
            generatedCode.addInstruction({ FJP::OP_CODE::STO, symbolTable.getDepthLevel() - variable.level, variable.address });
            break;
        case FJP::SymbolType::SYMBOL_INT_ARRAY:
        case FJP::SymbolType::SYMBOL_BOOL_ARRAY:
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::LEFT_SQUARED_BRACKET) {
                FJP::exitProgramWithError("[ expected", ERR_CODE, token.lineNumber);
            }
            token = lexer->getNextToken();
            processExpression();
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, variable.address });
            generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS });
            if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                FJP::exitProgramWithError("] expected", ERR_CODE, token.lineNumber);
            }
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::ASSIGN) {
                FJP::exitProgramWithError(":= expected", ERR_CODE, token.lineNumber);
            }
            token = lexer->getNextToken();
            processExpression();
            if (variable.symbolType == FJP::SymbolType::SYMBOL_BOOL_ARRAY) {
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
            }
            generatedCode.addInstruction({FJP::OP_CODE::STA, symbolTable.getDepthLevel() - variable.level, 0});
            break;
        default:
            FJP::exitProgramWithError("variable type expected", ERR_CODE, token.lineNumber);
            break;
    }

    if (expectSemicolon == true) {
        if (token.tokenType != FJP::TokenType::SEMICOLON) {
            FJP::exitProgramWithError("missing ;", ERR_CODE, token.lineNumber);
        }
        token = lexer->getNextToken();
    }
}

void FJP::Parser::processLabel(const std::string label) {
    symbolTable.addSymbol({FJP::SymbolType::SYMBOL_LABEL, label, 0, 0, generatedCode.getSize(), 0});
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::COLON) {
        FJP::exitProgramWithError(": expected", ERR_CODE, token.lineNumber);
    }
    auto itr = undefinedLabels.find(label);
    if (itr != undefinedLabels.end()) {
        for (auto index : itr->second) {
            generatedCode[index].m = generatedCode.getSize();
        }
        undefinedLabels.erase(label);
    }
    token = lexer->getNextToken();
}

void FJP::Parser::processCall() {
    if (token.tokenType != FJP::TokenType::CALL) {
        return;
    }

    token = lexer->getNextToken();
    Symbol function = symbolTable.findSymbol(token.value);

    if (function.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError("identifier not found", ERR_CODE, token.lineNumber);
    }

    if (function.symbolType != FJP::SymbolType::SYMBOL_FUNCTION) {
        FJP::exitProgramWithError("function name expected", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("( expected", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError(") expected", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError("; expected", ERR_CODE, token.lineNumber);
    }

    generatedCode.addInstruction({FJP::OP_CODE::CAL, symbolTable.getDepthLevel() - function.level, function.value});
    token = lexer->getNextToken();
}

void FJP::Parser::processScope() {
    if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
        return;
    }
    token = lexer->getNextToken();
    do {
        processStatement();
    } while (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET);

    token = lexer->getNextToken();
}

void FJP::Parser::processIf() {
    if (token.tokenType != TokenType::IF) {
        return;
    }

    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("missing (", ERR_CODE, token.lineNumber);
    }

    token = lexer->getNextToken();
    processCondition();

    int currentInstruction1 = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0});

    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
    }

    token = lexer->getNextToken();
    processStatement();

    if (token.tokenType == FJP::TokenType::ELSE) {
        int currentInstruction2 = generatedCode.getSize();
        generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});
        generatedCode[currentInstruction1].m = generatedCode.getSize();

        token = lexer->getNextToken();
        processStatement();
        generatedCode[currentInstruction2].m = generatedCode.getSize();
    } else {
        generatedCode[currentInstruction1].m = generatedCode.getSize();
    }
}

void FJP::Parser::processNop() {
    if (token.tokenType == FJP::TokenType::SEMICOLON) {
        token = lexer->getNextToken();
    }
}

void FJP::Parser::processWhile() {
    if (token.tokenType != FJP::TokenType::WHILE) {
        return;
    }

    int startWhileLoop = generatedCode.getSize();

    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("missing (", ERR_CODE, token.lineNumber);
    }

    token = lexer->getNextToken();
    processCondition();
    int endOfWhileCondition = generatedCode.getSize();

    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
    }

    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0 });
    token = lexer->getNextToken();
    processStatement();

    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, startWhileLoop});
    generatedCode[endOfWhileCondition].m = generatedCode.getSize();
}

void FJP::Parser::processDoWhile() {
    if (token.tokenType != FJP::TokenType::DO) {
        return;
    }
    int doWhileStart = generatedCode.getSize();
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
        FJP::exitProgramWithError("expected {", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    processStatement();

    if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
        FJP::exitProgramWithError("expected }", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::WHILE) {
        FJP::exitProgramWithError("missing while (do-while)", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    processCondition();

    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_EQ});
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, doWhileStart });

    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError("missing ;", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
}

void FJP::Parser::processRepeatUntil() {
    if (token.tokenType != FJP::TokenType::REPEAT) {
        return;
    }
    int repeatUntilStart = generatedCode.getSize();
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_CURLY_BRACKET) {
        FJP::exitProgramWithError("expected {", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    processStatement();

    if (token.tokenType != FJP::TokenType::RIGHT_CURLY_BRACKET) {
        FJP::exitProgramWithError("expected }", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::UNTIL) {
        FJP::exitProgramWithError("missing while (repeat-until)", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    processCondition();

    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_EQ});
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, repeatUntilStart });

    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError("missing ;", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
}

void FJP::Parser::processForeach() {
    if (token.tokenType != FJP::TokenType::FOREACH) {
        return;
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("missing (", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError("missing identifier", ERR_CODE, token.lineNumber);
    }
    FJP::Symbol iterVariable = symbolTable.findSymbol(token.value);
    if (iterVariable.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError("identifier not found", ERR_CODE, token.lineNumber);
    }
    switch (iterVariable.symbolType) {
        case FJP::SymbolType::SYMBOL_INT:
        case FJP::SymbolType::SYMBOL_BOOL:
            break;
        default:
            FJP::exitProgramWithError("invalid iterator type", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::COLON) {
        FJP::exitProgramWithError("foreach - missing :", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError("missing identifier", ERR_CODE, token.lineNumber);
    }
    FJP::Symbol dataArray = symbolTable.findSymbol(token.value);
    if (dataArray.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError("array not found", ERR_CODE, token.lineNumber);
    }

    switch (iterVariable.symbolType) {
        case FJP::SymbolType::SYMBOL_INT:
            if (dataArray.symbolType != FJP::SymbolType::SYMBOL_INT_ARRAY) {
                FJP::exitProgramWithError("the iterator and the array have to be the same type", ERR_CODE, token.lineNumber);
            }
            break;
        case FJP::SymbolType::SYMBOL_BOOL:
            if (dataArray.symbolType != FJP::SymbolType::SYMBOL_BOOL_ARRAY) {
                FJP::exitProgramWithError("the iterator and the array have to be the same type", ERR_CODE, token.lineNumber);
            }
            break;
        default:
            break;
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();

    int indexAddress = arSize;
    generatedCode.addInstruction({FJP::OP_CODE::INC, 0, 1});

    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
    generatedCode.addInstruction({FJP::OP_CODE::STO, 0, indexAddress});

    int startForeachBody = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::LOD, 0, indexAddress});
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, dataArray.size});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ});

    int exitForeachAddress = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0});

    generatedCode.addInstruction({FJP::OP_CODE::LOD, 0, indexAddress});
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, dataArray.address});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
    generatedCode.addInstruction({FJP::OP_CODE::LDA, symbolTable.getDepthLevel() - dataArray.level, 0});
    generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - iterVariable.level, iterVariable.address});

    generatedCode.addInstruction({FJP::OP_CODE::LOD, 0, indexAddress});
    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 1});
    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
    generatedCode.addInstruction({FJP::OP_CODE::STO, 0, indexAddress});

    processStatement();
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, startForeachBody});

    generatedCode[exitForeachAddress].m = generatedCode.getSize();
    generatedCode.addInstruction({FJP::OP_CODE::DEC, 0, 1});
}

void FJP::Parser::processFor() {
    if (token.tokenType != FJP::TokenType::FOR) {
        return;
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("missing (", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    processAssignment();

    int startCondition = generatedCode.getSize();
    processCondition();
    generatedCode.addInstruction({FJP::OP_CODE::JPC, 0, 0 });
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, 0});
    int startUpdatePart = generatedCode.getSize();

    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError("missing ;", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();

    processAssignment(false);
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, startCondition});
    int endUpdatePart = generatedCode.getSize();

    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    processStatement();
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, startUpdatePart});

    generatedCode[startUpdatePart - 2].m = generatedCode.getSize();
    generatedCode[startUpdatePart - 1].m = endUpdatePart;
}

void FJP::Parser::processGoto() {
    if (token.tokenType != FJP::TokenType::GOTO) {
        return;
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError("identifier expected", ERR_CODE, token.lineNumber);
    }
    FJP::Symbol symbol = symbolTable.findSymbol(token.value);

    if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        undefinedLabels[token.value].push_back(generatedCode.getSize());
    } else if (symbol.symbolType != FJP::SymbolType::SYMBOL_LABEL) {
        FJP::exitProgramWithError("label expected", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError("missing ;", ERR_CODE, token.lineNumber);
    }
    generatedCode.addInstruction({FJP::OP_CODE::JMP, 0, symbol.address});
    token = lexer->getNextToken();
}

void FJP::Parser::processCondition() {
    if (token.tokenType == FJP::TokenType::EXCLAMATION_MARK) {
        token = lexer->getNextToken();
        processExpression();

        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
        generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_EQ});
    } else {
        processExpression();
        FJP::OPRType instructionType = FJP::OPRType::OPR_RET;
        FJP::TokenType logicalOperation = FJP::TokenType::UNKNOWN;
        switch (token.tokenType) {
            case FJP::TokenType::EQUALS:
                instructionType = FJP::OPRType::OPR_EQ;
                break;
            case FJP::TokenType::NOT_EQUALS:
                instructionType = FJP::OPRType::OPR_NEQ;
                break;
            case FJP::TokenType::LESS:
                instructionType = FJP::OPRType::OPR_LESS;
                break;
            case FJP::TokenType::LESS_OR_EQUAL:
                instructionType = FJP::OPRType::OPR_LESS_EQ;
                break;
            case FJP::TokenType::GREATER:
                instructionType = FJP::OPRType::OPR_GRT;
                break;
            case FJP::TokenType::GREATER_OR_EQUAL:
                instructionType = FJP::OPRType::OPR_GRT_EQ;
                break;
            case FJP::TokenType::LOGICAL_AND:
            case FJP::TokenType::LOGICAL_OR:
                logicalOperation = token.tokenType;
                break;
            default:
                FJP::exitProgramWithError("invalid condition operator", ERR_CODE, token.lineNumber);
        }
        token = lexer->getNextToken();
        processExpression();

        switch (logicalOperation) {
            case FJP::TokenType::UNKNOWN:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, instructionType});
                break;
            case FJP::TokenType::LOGICAL_AND:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_MUL});
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0});
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ});
                break;
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

void FJP::Parser::processExpression() {
    FJP::TokenType currTokenType;

    if (token.tokenType == FJP::TokenType::PLUS || token.tokenType == FJP::TokenType::MINUS) {
        currTokenType = token.tokenType;
        token = lexer->getNextToken();
        processTerm();

        if (currTokenType == FJP::TokenType::MINUS) {
            generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_INVERT_VALUE});
        }
    } else {
        processTerm();
    }
    while (token.tokenType == FJP::TokenType::PLUS || token.tokenType == FJP::TokenType::MINUS) {
        currTokenType = token.tokenType;

        token = lexer->getNextToken();
        processTerm();

        switch (currTokenType) {
            case FJP::TokenType::PLUS:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
                break;
            case FJP::TokenType::MINUS:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_MINUS});
                break;
            default:
                break;
        }
    }
}

void FJP::Parser::processTerm() {
    FJP::TokenType currTokenType;
    processFactor();

    while (token.tokenType == FJP::TokenType::ASTERISK || token.tokenType == FJP::TokenType::SLASH) {
        currTokenType = token.tokenType;

        token = lexer->getNextToken();
        processFactor();

        switch (currTokenType) {
            case FJP::TokenType::ASTERISK:
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_MUL});
                break;
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
        case FJP::TokenType::IDENTIFIER:
            symbol = symbolTable.findSymbol(token.value);
            if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
                FJP::exitProgramWithError("identifier not found", ERR_CODE, token.lineNumber);
            }
            token = lexer->getNextToken();
            if (token.tokenType == FJP::TokenType::INSTANCEOF) {
                token = lexer->getNextToken();
                switch (token.tokenType) {
                    case FJP::TokenType::INT:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.symbolType == FJP::SYMBOL_INT});
                        break;
                    case FJP::TokenType::BOOL:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.symbolType == FJP::SYMBOL_BOOL});
                        break;
                    case FJP::TokenType::FUNCTION:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.symbolType == FJP::SYMBOL_FUNCTION});
                        break;
                    default:
                        FJP::exitProgramWithError("invalid instanceof type", ERR_CODE, token.lineNumber);
                        break;
                }
            } else {
                nextToken = false;
                switch (symbol.symbolType) {
                    case FJP::SymbolType::SYMBOL_INT:
                    case FJP::SymbolType::SYMBOL_BOOL:
                        generatedCode.addInstruction({FJP::OP_CODE::LOD, symbolTable.getDepthLevel() - symbol.level, symbol.address});
                        break;
                    case FJP::SymbolType::SYMBOL_CONST:
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.value});
                        break;
                    case FJP::SymbolType::SYMBOL_INT_ARRAY:
                    case FJP::SymbolType::SYMBOL_BOOL_ARRAY:
                        if (token.tokenType != FJP::TokenType::LEFT_SQUARED_BRACKET) {
                            FJP::exitProgramWithError("missing [", ERR_CODE, token.lineNumber);
                        }
                        token = lexer->getNextToken();
                        processExpression();
                        generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.address});
                        generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
                        generatedCode.addInstruction({FJP::OP_CODE::LDA, symbolTable.getDepthLevel() - symbol.level, 0});
                        if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                            FJP::exitProgramWithError("missing ]", ERR_CODE, token.lineNumber);
                        }
                        token = lexer->getNextToken();
                        break;
                    default:
                        FJP::exitProgramWithError("invalid identifier", ERR_CODE, token.lineNumber);
                        break;
                }
            }
            break;
        case FJP::TokenType::NUMBER:
            numberValue = atoi(token.value.c_str());
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, numberValue});
            break;
        case FJP::TokenType::TRUE:
        case FJP::TokenType::FALSE:
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, token.tokenType == FJP::TokenType::TRUE});
            break;
        case FJP::TokenType::LEFT_PARENTHESIS:
            token = lexer->getNextToken();
            processExpression();
            if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
                FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
            }
            break;
        default:
            FJP::exitProgramWithError("Expected variable, number, (, or missing ;", ERR_CODE, token.lineNumber);
            break;
    }
    if (nextToken == true) {
        token = lexer->getNextToken();
    }
}

void FJP::Parser::processRead() {
    if (token.tokenType != FJP::TokenType::READ) {
        return;
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("missing (", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::IDENTIFIER) {
        FJP::exitProgramWithError("identifier expected", ERR_CODE, token.lineNumber);
    }
    FJP::Symbol symbol = symbolTable.findSymbol(token.value);
    if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
        FJP::exitProgramWithError("symbol not found", ERR_CODE, token.lineNumber);
    }

    switch (symbol.symbolType) {
        case FJP::SymbolType::SYMBOL_INT:
            generatedCode.addInstruction({FJP::OP_CODE::SIO, 0, FJP::SIO_TYPE::SIO_READ});
            generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - symbol.level, symbol.address});
            break;
        case FJP::SymbolType::SYMBOL_BOOL:
            generatedCode.addInstruction({FJP::OP_CODE::SIO, 0, FJP::SIO_TYPE::SIO_READ});
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });
            generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
            generatedCode.addInstruction({FJP::OP_CODE::STO, symbolTable.getDepthLevel() - symbol.level, symbol.address});
            break;
        case FJP::SymbolType::SYMBOL_INT_ARRAY:
        case FJP::SymbolType::SYMBOL_BOOL_ARRAY:
            token = lexer->getNextToken();
            if (token.tokenType != FJP::TokenType::LEFT_SQUARED_BRACKET) {
                FJP::exitProgramWithError("missing [", ERR_CODE, token.lineNumber);
            }
            token = lexer->getNextToken();
            processExpression();
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.address });
            generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS });
            generatedCode.addInstruction({FJP::OP_CODE::SIO, 0, FJP::SIO_TYPE::SIO_READ});

            if (symbol.symbolType == FJP::SymbolType::SYMBOL_BOOL_ARRAY) {
                generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, 0 });
                generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_NEQ });
            }

            generatedCode.addInstruction({FJP::OP_CODE::STA, symbolTable.getDepthLevel() - symbol.level, 0});
            if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                FJP::exitProgramWithError("missing ]", ERR_CODE, token.lineNumber);
            }
            break;
        default:
            FJP::exitProgramWithError("symbol does not refer to a variable", ERR_CODE, token.lineNumber);
            break;
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError("missing ;", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
}

void FJP::Parser::processWrite() {
    if (token.tokenType != FJP::TokenType::WRITE) {
        return;
    }

    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::LEFT_PARENTHESIS) {
        FJP::exitProgramWithError("missing (", ERR_CODE, token.lineNumber);
    }

    int number;
    FJP::Symbol symbol;
    token = lexer->getNextToken();

    switch (token.tokenType) {
        case FJP::TokenType::IDENTIFIER:
            symbol = symbolTable.findSymbol(token.value);
            if (symbol.symbolType == FJP::SymbolType::SYMBOL_NOT_FOUND) {
                FJP::exitProgramWithError("symbol not found", ERR_CODE, token.lineNumber);
            }

            switch (symbol.symbolType) {
                case FJP::SymbolType::SYMBOL_INT:
                case FJP::SymbolType::SYMBOL_BOOL:
                    generatedCode.addInstruction({FJP::OP_CODE::LOD, symbolTable.getDepthLevel() - symbol.level, symbol.address});
                    break;
                case FJP::SymbolType::SYMBOL_INT_ARRAY:
                case FJP::SymbolType::SYMBOL_BOOL_ARRAY:
                    token = lexer->getNextToken();
                    if (token.tokenType != FJP::TokenType::LEFT_SQUARED_BRACKET) {
                        FJP::exitProgramWithError("missing [", ERR_CODE, token.lineNumber);
                    }
                    token = lexer->getNextToken();

                    processExpression();
                    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.address});
                    generatedCode.addInstruction({FJP::OP_CODE::OPR, 0, FJP::OPRType::OPR_PLUS});
                    generatedCode.addInstruction({FJP::OP_CODE::LDA, symbolTable.getDepthLevel() - symbol.level, 0});

                    if (token.tokenType != FJP::TokenType::RIGHT_SQUARED_BRACKET) {
                        FJP::exitProgramWithError("missing ]", ERR_CODE, token.lineNumber);
                    }
                    break;
                case FJP::SymbolType::SYMBOL_CONST:
                    generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, symbol.value});
                    break;
                default:
                    FJP::exitProgramWithError("symbol does not refer to a variable", ERR_CODE, token.lineNumber);
                    break;
            }
            break;
        case FJP::TokenType::NUMBER:
            number = atoi(token.value.c_str());
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, number});
            break;
        case FJP::TokenType::TRUE:
        case FJP::TokenType::FALSE:
            generatedCode.addInstruction({FJP::OP_CODE::LIT, 0, token.tokenType == FJP::TokenType::TRUE});
            break;
        default:
            FJP::exitProgramWithError("invalid identifier to print out", ERR_CODE, token.lineNumber);
            break;
    }
    generatedCode.addInstruction({FJP::OP_CODE::SIO, 0, FJP::SIO_TYPE::SIO_WRITE});

    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::RIGHT_PARENTHESIS) {
        FJP::exitProgramWithError("missing )", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
    if (token.tokenType != FJP::TokenType::SEMICOLON) {
        FJP::exitProgramWithError("missing ;", ERR_CODE, token.lineNumber);
    }
    token = lexer->getNextToken();
}