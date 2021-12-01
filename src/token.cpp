#include <algorithm>

#include "token.h"

std::ostream &FJP::operator<<(std::ostream &out, const FJP::Token &token) {
    out << "{\n";
    out << "    \"typeId\": " << "\""     << token.tokenType                    << "\"" << ",\n";
    out << "    \"type\": " << "\""       << token_type_to_str(token.tokenType) << "\"" << ",\n";
    out << "    \"lineNumber\": " << "\"" << token.lineNumber                   << "\"" << ",\n";
    out << "    \"value\": " << "\""      << token.value                        << "\"" << "\n";
    out << "}";
    return out;
}

std::string FJP::token_type_to_str(FJP::TokenType tokenType) {
    auto match = std::find_if(keywords.begin(), keywords.end(), [&](const auto &keyword) {
       return keyword.second == tokenType;
    });

    if (match != keywords.end())
        return match->first;

    switch (tokenType) {
        case IDENTIFIER:
            return "identifier";
        case NUMBER:
            return "number";
        default:
            return "unknown";
    }
    return "unknown";
}