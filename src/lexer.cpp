#include <algorithm>
#include <fstream>
#include <sstream>

#include <lexer.h>
#include <errors.h>
#include <logger.h>

// Helper function used to validate whether
// a character could be a part of an identifier or not.
static int is_part_of_identifier(int c);

// Helper function used to validate whether
// a character could be a start of an identifier or not.
static int is_start_of_identifier(int c);

FJP::Lexer *FJP::Lexer::instance = nullptr;

FJP::Lexer* FJP::Lexer::getInstance() {
    if (instance == nullptr) {
        instance = new Lexer;
    }
    return instance;
}

FJP::Lexer::Lexer() : currentCharIndex(0), currentLineNumber(0) {
    // Sort all the program keywords by their length. This is done
    // because we want to parse longer keywords first. For example,
    // we don't want to parse ':=' as ':' and '=' but we want to
    // parse it as ':='.
    sort(keywords.begin(), keywords.end(), [&](const auto &keyword_A, const auto &keyword_B) {
        return keyword_A.first.length() > keyword_B.first.length();
    });

    // Create a list of all alphabetic keywords such as 'goto', 'for', etc.
    for (const auto &keyword : keywords) {
        if (std::all_of(std::begin(keyword.first), std::end(keyword.first), [](char c) { return std::isalpha(c); })) {
            alphabetic_keywords.insert(keyword.first);
        }
    }
}

void FJP::Lexer::init(std::string filename, bool debug) {
    // Initialize variables.
    currentLineNumber = 0;
    currentCharIndex = 0;
    tokens.clear();

    // Terminate the application if the input file cannot be open.
    std::ifstream file(filename);
    if (file.is_open() == false) {
        FJP::exitProgramWithError(FJP::IOErrors::ERROR_00, ERR_CODE);
    }

    // Read the content of the input file (source code).
    fileContent = std::string((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));

    // Parse the input file and sets the token iterator
    // to the first token, so it can be consumed by the parser.
    processAllTokens(debug);
    currentTokenIt = tokens.begin();
}

FJP::Token FJP::Lexer::getNextToken() {
    // If the parse requests another token while it's
    // already an end of file, it's an error
    if (currentTokenIt == tokens.end()) {
        FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_00, ERR_CODE);
    }

    // Return the current token to the parser move on to the next token.
    Token token = *currentTokenIt;
    ++currentTokenIt;
    return token;
}

void FJP::Lexer::returnToPreviousToken() {
    // We're already at the very for token.
    if (currentTokenIt == tokens.begin()) {
        FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_05, ERR_CODE);
    }

    // Move back one step within the stream of tokens.
    --currentTokenIt;
}

void FJP::Lexer::processAllTokens(bool debug) {
    Token token;

    std::stringstream ss;
    std::ofstream outputJSONFile;

    // If the debug is on, create the output file (tokens.json)
    if (debug == true) {
        outputJSONFile = std::ofstream(OUTPUT_FILE);

        // If there's an error while opening the output file, terminate the application.
        if (outputJSONFile.is_open() == false) {
            FJP::exitProgramWithError(FJP::IOErrors::ERROR_01, ERR_CODE);
        }
    }

    // Parse the entire input file token by token.
    ss << "[";
    if (fileContent.size() > 0) {
        while (isEndOfFile() == false) {
            token = parseNextToken();
            tokens.push_back(token);
            ss << token << ",\n";
        }
        ss.seekp(-2, std::ios_base::end);
    }
    ss << "]";

    // If the debug flag is on, store all tokens into the output file.
    // The format of the output file is JSON.
    if (debug == true) {
        outputJSONFile << ss.str();
        outputJSONFile.close();
    }
}

bool FJP::Lexer::isEndOfFile() const {
    return currentCharIndex >= static_cast<long>(fileContent.length());
}

FJP::Token FJP::Lexer::parseNextToken() {
    std::string tokenValue;

    // Skip all comments if there are any.
    skipComments();

    // Go through all the keywords in descending order (by their length)
    for (auto &keyword : keywords) {
        // Check if the current keyword would fit into the file (it doesn't exceed the file length)
        if (currentCharIndex + keyword.first.length() > fileContent.length())
            continue;

        // Check if it's indeed the keyword.
        tokenValue = fileContent.substr(currentCharIndex, keyword.first.length());
        if (tokenValue == keyword.first) {

            // Move on by the keyword length in the input file.
            currentCharIndex += tokenValue.length();

            // Check if the keyword is not just a prefix of a custom
            // identifier such as a variable or an array.
            if (alphabetic_keywords.count(tokenValue)) {
                if (currentCharIndex < static_cast<long>(fileContent.length()) && is_part_of_identifier(fileContent[currentCharIndex])) {
                    currentCharIndex -= tokenValue.length();
                    continue;
                }
            }

            // If a keyword has been found, skip all the white spaces
            // and return the token.
            skipWhiteCharacters();
            return {
                    keyword.second,   // type of the token
                    tokenValue,       // value of the token
                    currentLineNumber // number of the line the token is on
            };
        }
    }

    // If it's not a keyword, check if it's a number
    if (isdigit(fileContent[currentCharIndex])) {
        // If it is a digit, get all the following digits as well
        // until you find a non-digit character.
        std::string number = getValue(&isdigit);

        // Check if the digit is not too long (it fits into the datatype).
        if (atoi(number.c_str()) < 0) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_02, ERR_CODE, currentLineNumber);
        }

        // Skip all the white spaces and return the token.
        skipWhiteCharacters();
        return {
                NUMBER,           // type of the token
                number,           // value of the token
                currentLineNumber // number of the line the token is on

        };
    }

    // It's not a keyword nor a number, it has to be an identifier.
    if (is_start_of_identifier(fileContent[currentCharIndex])) {
        // Consume all characters that belong to the identifier.
        std::string identifier = getValue(&is_part_of_identifier);

        // Check if the identifier doesn't exceed the maximum length.
        if (identifier.length() > MAX_IDENTIFIER_LEN) {
            FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_03, ERR_CODE, currentLineNumber);
        }

        // Skip all the white spaces and return the token.
        skipWhiteCharacters();
        return {
                IDENTIFIER,       // type of the token
                identifier,       // value of the token
                currentLineNumber // number of the line the token is on
        };
    }

    // If the program gets to this point, it's an unknown character.
    FJP::exitProgramWithError(__FUNCTION__, FJP::CompilationErrors::ERROR_04, ERR_CODE, currentLineNumber);

    // Just so the compiler doesn't complain about a missing return value.
    return {};
}

std::string FJP::Lexer::getValue(int (validation_fce)(int)) {
    std::string value = "";

    // Keep consuming the characters as long as the validation
    // function is satisfied.
    while (!isEndOfFile() && validation_fce(fileContent[currentCharIndex])) {
        value += fileContent[currentCharIndex];
        currentCharIndex++;
    }
    return value;
}

static int is_part_of_identifier(int c) {
    // The "body" of an identifier (not the first character)
    // can consist of a digit, alphabetic character or an underscore.
    return isdigit(c) || isalpha(c) || c == '_';
}

static int is_start_of_identifier(int c) {
    // An identifier can only start with an alphabetic character or an underscore.
    return isalpha(c) || c == '_';
}

void FJP::Lexer::skipWhiteCharacters() {
    // Skip all white spaces.
    while (isspace(fileContent[currentCharIndex])) {
        // Keep counting the lines as you skip the white spaces.
        if (fileContent[currentCharIndex] == '\n') {
            currentLineNumber++;
        }
        currentCharIndex++;
    }
}

void FJP::Lexer::skipComments() {
    // First off, skip all possible white characters.
    skipWhiteCharacters();

    std::string commentStart(COMMENT_START);
    std::string commentEnd(COMMENT_END);

    // Make sure we're not at the end of the file (that there's still enough room for a comment).
    if (currentCharIndex + commentStart.length() >= fileContent.length())
        return;

    // Check if the next two characters make up the start sequence of a comment.
    std::string characterPair = fileContent.substr(currentCharIndex, commentStart.length());
    if (characterPair != commentStart)
        return;

    // Keep counting open comments.
    int openComments = 1;

    currentCharIndex += commentStart.length();
    while (openComments > 0) {
        // Skip all white spaces.
        skipWhiteCharacters();

        // If we reach the end of the file and there's still an unclosed comment, it's an error.
        if (currentCharIndex + commentStart.length() > fileContent.length()) {
            FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_01, ERR_CODE);
        }

        // Parse the next two characters.
        characterPair = fileContent.substr(currentCharIndex, commentStart.length());

        // Check if it's an opening of another comment,
        // closing sequence, or neither.
        if (characterPair == commentStart) {
            openComments++;
            currentCharIndex += commentStart.length();
        } else if (characterPair == commentEnd) {
            openComments--;
            currentCharIndex += commentStart.length();
        } else {
            currentCharIndex++;
        }
    }
    // Skip all white spaces.
    skipWhiteCharacters();
}