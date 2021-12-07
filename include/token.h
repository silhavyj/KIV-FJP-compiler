#pragma once

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <unordered_set>

namespace FJP {

    enum TokenType {
        IDENTIFIER,
        NUMBER,
        PLUS,
        MINUS,
        ASTERISK,
        SLASH,
        EQUALS,
        NOT_EQUALS,
        LESS,
        LESS_OR_EQUAL,
        GREATER,
        GREATER_OR_EQUAL,
        LEFT_PARENTHESIS,
        RIGHT_PARENTHESIS,
        LEFT_CURLY_BRACKET,
        RIGHT_CURLY_BRACKET,
        LEFT_SQUARED_BRACKET,
        RIGHT_SQUARED_BRACKET,
        LOGICAL_AND,
        LOGICAL_OR,
        COMMA,
        COLON,
        SEMICOLON,
        PERIOD,
        ASSIGN,
        INIT_SIGN,
        IF,
        FOR,
        WHILE,
        DO,
        REPEAT,
        UNTIL,
        CONST,
        INT,
        BOOL,
        INT_ARRAY,
        BOOL_ARRAY,
        TRUE,
        FALSE,
        FUNCTION,
        WRITE,
        READ,
        ELSE,
        CALL,
        EXCLAMATION_MARK,
        QUESTION_MARK,
        GOTO,
        INSTANCEOF,
        START,
        UNKNOWN,
        FOREACH,
        SWITCH,
        CASE,
        BREAK,
        HASH_MARK,
        END
    };

    struct Token {
        TokenType tokenType;
        std::string value;
        int lineNumber;
    };

    static std::vector<std::pair<std::string, TokenType>> keywords = {
        {"instanceof",TokenType::INSTANCEOF            },
        {"function",  TokenType::FUNCTION              },
        {"repeat",    TokenType::REPEAT                },
        {"until",     TokenType::UNTIL                 },
        {"const",     TokenType::CONST                 },
        {"while",     TokenType::WHILE                 },
        {"START",     TokenType::START                 },
        {"END",       TokenType::END                   },
        {"write",     TokenType::WRITE                 },
        {"foreach",   TokenType::FOREACH               },
        {"switch",    TokenType::SWITCH                },
        {"case",      TokenType::CASE                  },
        {"break",     TokenType::BREAK                 },
        {"else",      TokenType::ELSE                  },
        {"read",      TokenType::READ                  },
        {"goto",      TokenType::GOTO                  },
        {"bool",      TokenType::BOOL                  },
        {"call",      TokenType::CALL                  },
        {"true",      TokenType::TRUE                  },
        {"false",     TokenType::FALSE                 },
        {"for",       TokenType::FOR                   },
        {"int",       TokenType::INT                   },
        {"int[]",     TokenType::INT_ARRAY             },
        {"bool[]",    TokenType::BOOL_ARRAY            },
        {"if",        TokenType::IF                    },
        {"do",        TokenType::DO                    },
        {":=",        TokenType::ASSIGN                },
        {"<=",        TokenType::LESS_OR_EQUAL         },
        {"!=",        TokenType::NOT_EQUALS            },
        {">=",        TokenType::GREATER_OR_EQUAL      },
        {"==",        TokenType::EQUALS                },
        {"&&",        TokenType::LOGICAL_AND           },
        {"||",        TokenType::LOGICAL_OR            },
        {"=",         TokenType::INIT_SIGN             },
        {"(",         TokenType::LEFT_PARENTHESIS      },
        {")",         TokenType::RIGHT_PARENTHESIS     },
        {"{",         TokenType::LEFT_CURLY_BRACKET    },
        {"}",         TokenType::RIGHT_CURLY_BRACKET   },
        {"[",         TokenType::LEFT_SQUARED_BRACKET  },
        {"]",         TokenType::RIGHT_SQUARED_BRACKET },
        {"*",         TokenType::ASTERISK              },
        {"/",         TokenType::SLASH                 },
        {"<",         TokenType::LESS                  },
        {">",         TokenType::GREATER               },
        {"+",         TokenType::PLUS                  },
        {"-",         TokenType::MINUS                 },
        {";",         TokenType::SEMICOLON             },
        {":",         TokenType::COLON                 },
        {",",         TokenType::COMMA                 },
        {".",         TokenType::PERIOD                },
        {"!",         TokenType::EXCLAMATION_MARK      },
        {"?",         TokenType::QUESTION_MARK         },
        {"#",         TokenType::HASH_MARK             }
    };

    std::ostream &operator<<(std::ostream &out, const Token &token);
    std::string token_type_to_str(TokenType tokenType);
}