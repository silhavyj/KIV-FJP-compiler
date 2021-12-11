#pragma once

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <unordered_set>

namespace FJP {

    /// List of all tokens available in the application.
    /// A stream of these tokens is used as an input for the parser.
    enum TokenType {
        IDENTIFIER,            // _[a-zA-Z]*
        NUMBER,                // [0-9]+
        PLUS,                  // '+'
        MINUS,                 // '-'
        ASTERISK,              // '*'
        SLASH,                 // '/'
        EQUALS,                // '=='
        CONST_INIT,            // '='
        NOT_EQUALS,            // '!='
        LESS,                  // '<'
        LESS_OR_EQUAL,         // '<='
        GREATER,               // '>'
        GREATER_OR_EQUAL,      // '>='
        LEFT_PARENTHESIS,      // '('
        RIGHT_PARENTHESIS,     // ')'
        LEFT_CURLY_BRACKET,    // '{'
        RIGHT_CURLY_BRACKET,   // '}'
        LEFT_SQUARED_BRACKET,  // '['
        RIGHT_SQUARED_BRACKET, // ']'
        LOGICAL_AND,           // '&&'
        LOGICAL_OR,            // '||'
        COMMA,                 // ','
        COLON,                 // ':'
        SEMICOLON,             // ';'
        PERIOD,                // '.'
        ASSIGN,                // ':='
        IF,                    // 'if'
        FOR,                   // 'for'
        WHILE,                 // 'while'
        DO,                    // 'do'
        REPEAT,                // 'repeat'
        UNTIL,                 // 'until'
        CONST,                 // 'const'
        INT,                   // 'int'
        BOOL,                  // 'bool'
        INT_ARRAY,             // 'int[]'
        BOOL_ARRAY,            // 'bool[]'
        TRUE,                  // 'true'
        FALSE,                 // 'false'
        FUNCTION,              // 'function'
        WRITE,                 // 'write'
        READ,                  // 'read'
        ELSE,                  // 'else'
        CALL,                  // 'call'
        EXCLAMATION_MARK,      // '!'
        QUESTION_MARK,         // '?'
        GOTO,                  // 'goto'
        INSTANCEOF,            // 'instanceof'
        START,                 // 'START'
        FOREACH,               // 'foreach'
        SWITCH,                // 'switch'
        CASE,                  // 'case'
        BREAK,                 // 'break'
        HASH_MARK,             // '#'
        END,                   // 'END'
        UNKNOWN
    };

    /// Definition of a token which is being passed from the lexer to the parser.
    struct Token {
        TokenType tokenType; ///< type of the token
        std::string value;   ///< value of the token e.g. a number
        int lineNumber;      ///< number of the line in the source code
    };

    /// List of literal string values mapped onto their corresponding token values.
    /// This std::vector will be sorted out upon the lexer instantiation. The reason
    /// for sorting these values is to prevent mistreating tokens which happen
    /// to be a prefix of another token. For example, ':' and ':='. Therefore,
    /// the list will be sorted in a descending order by the token string value.
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
        {"=",         TokenType::CONST_INIT            },
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

    /// Prints out the token passed in as a parameter in a JSON format.
    /// \param out output stream
    /// \param token token to be printed to the stream
    /// \return modified output stream (extended by the token)
    std::ostream &operator<<(std::ostream &out, const Token &token);

    /// Converts a token type into a string value. This function
    /// is used when printing tokens into the output stream.
    /// \param tokenType type of a token
    /// \return token type converted into a string value
    std::string token_type_to_str(TokenType tokenType);
}