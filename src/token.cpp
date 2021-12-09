#include <algorithm>

#include <token.h>

std::ostream &FJP::operator<<(std::ostream &out, const FJP::Token &token) {
    // Prints out the token into the output stream in a JSON format.
    out << "{\n";
    out << "    \"typeId\": " << "\""     << token.tokenType                    << "\"" << ",\n";
    out << "    \"type\": " << "\""       << token_type_to_str(token.tokenType) << "\"" << ",\n";
    out << "    \"lineNumber\": " << "\"" << token.lineNumber                   << "\"" << ",\n";
    out << "    \"value\": " << "\""      << token.value                        << "\"" << "\n";
    out << "}";

    // Return the output stream.
    return out;
}

std::string FJP::token_type_to_str(FJP::TokenType tokenType) {
    // Tries to find a token by its corresponding string value (using the keywords list)
    auto match = std::find_if(keywords.begin(), keywords.end(), [&](const auto &keyword) {
       return keyword.second == tokenType;
    });

    // If such token has not been found, it has to be either an
    // identifier or a number because those are not listed out due to
    // their variability
    if (match != keywords.end())
        return match->first;

    switch (tokenType) {
        case IDENTIFIER:
            return "identifier";
        case NUMBER:
            return "number";
        default:
            return "unknown"; // It should never get here
    }
    // This is here just so the compiler does not complain about
    // a missing return value at the end of the method.
    return "unknown";
}