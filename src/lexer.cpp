#include <algorithm>
#include <fstream>
#include <sstream>

#include <lexer.h>
#include <errors.h>
#include <logger.h>

static int is_part_of_identifier(int c);
static int is_start_of_identifier(int c);

FJP::Lexer *FJP::Lexer::instance = nullptr;

FJP::Lexer* FJP::Lexer::getInstance() {
    if (instance == nullptr) {
        instance = new Lexer;
    }
    return instance;
}

FJP::Lexer::Lexer() : currentCharIndex(0), currentLineNumber(0) {
    sort(keywords.begin(), keywords.end(), [&](const auto &keyword_A, const auto &keyword_B) {
        return keyword_A.first.length() > keyword_B.first.length();
    });
    for (const auto &keyword : keywords) {
        if (std::all_of(std::begin(keyword.first), std::end(keyword.first), [](char c) { return std::isalpha(c); })) {
            alphabetic_keywords.insert(keyword.first);
        }
    }
}

void FJP::Lexer::init(std::string filename, bool debug) {
    currentLineNumber = 0;
    currentCharIndex = 0;
    tokens.clear();

    std::ifstream file(filename);
    if (file.is_open() == false) {
        FJP::exitProgramWithError(FJP::IOErrors::ERROR_00, ERR_CODE);
    }

    fileContent = std::string((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
    processAllTokens(debug);
    currentTokenIt = tokens.begin();
}

FJP::Token FJP::Lexer::getNextToken() {
    if (currentTokenIt == tokens.end()) {
        FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_00, ERR_CODE);
    }
    Token token = *currentTokenIt;
    ++currentTokenIt;
    return token;
}

void FJP::Lexer::returnToPreviousToken() {
    if (currentTokenIt == tokens.begin()) {
        FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_05, ERR_CODE);
    }
    --currentTokenIt;
}

void FJP::Lexer::processAllTokens(bool debug) {
    Token token;
    std::stringstream ss;
    std::ofstream outputJSONFile;

    if (debug == true) {
        outputJSONFile = std::ofstream(OUTPUT_FILE);
        if (outputJSONFile.is_open() == false) {
            FJP::exitProgramWithError(FJP::IOErrors::ERROR_01, ERR_CODE);
        }
    }

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
    skipComments();

    for (auto &keyword : keywords) {
        if (currentCharIndex + keyword.first.length() > fileContent.length())
            continue;

        tokenValue = fileContent.substr(currentCharIndex, keyword.first.length());
        if (tokenValue == keyword.first) {
            currentCharIndex += tokenValue.length();

            if (alphabetic_keywords.count(tokenValue)) {
                if (currentCharIndex < static_cast<long>(fileContent.length()) && is_part_of_identifier(fileContent[currentCharIndex])) {
                    currentCharIndex -= tokenValue.length();
                    continue;
                }
            }
            skipWhiteCharacters();
            return {
                    keyword.second,
                    tokenValue,
                    currentLineNumber
            };
        }
    }
    if (isdigit(fileContent[currentCharIndex])) {
        std::string number = getValue(&isdigit);

        if (atoi(number.c_str()) < 0) {
            FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_02, ERR_CODE, currentLineNumber);
        }
        skipWhiteCharacters();
        return {
                NUMBER,
                number,
                currentLineNumber

        };
    }
    if (is_start_of_identifier(fileContent[currentCharIndex])) {
        std::string identifier = getValue(&is_part_of_identifier);

        if (identifier.length() > MAX_IDENTIFIER_LEN) {
            FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_03, ERR_CODE, currentLineNumber);
        }
        skipWhiteCharacters();
        return {
                IDENTIFIER,
                identifier,
                currentLineNumber
        };
    }
    FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_04, ERR_CODE, currentLineNumber);
    return {};
}

std::string FJP::Lexer::getValue(int (validation_fce)(int)) {
    std::string value = "";
    while (!isEndOfFile() && validation_fce(fileContent[currentCharIndex])) {
        value += fileContent[currentCharIndex];
        currentCharIndex++;
    }
    return value;
}

static int is_part_of_identifier(int c) {
    return isdigit(c) || isalpha(c) || c == '_';
}

static int is_start_of_identifier(int c) {
    return isalpha(c) || c == '_';
}

void FJP::Lexer::skipWhiteCharacters() {
    while (isspace(fileContent[currentCharIndex])) {
        if (fileContent[currentCharIndex] == '\n') {
            currentLineNumber++;
        }
        currentCharIndex++;
    }
}

void FJP::Lexer::skipComments() {
    skipWhiteCharacters();

    std::string commentStart(COMMENT_START);
    std::string commentEnd(COMMENT_END);

    if (currentCharIndex + commentStart.length() >= fileContent.length())
        return;

    std::string characterPair = fileContent.substr(currentCharIndex, commentStart.length());
    if (characterPair != commentStart)
        return;

    int openComments = 1;
    currentCharIndex += commentStart.length();

    while (openComments > 0) {
        skipWhiteCharacters();

        if (currentCharIndex + commentStart.length() > fileContent.length()) {
            FJP::exitProgramWithError(FJP::CompilationErrors::ERROR_01, ERR_CODE);
        }

        characterPair = fileContent.substr(currentCharIndex, commentStart.length());
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
    skipWhiteCharacters();
}