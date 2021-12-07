#include <iostream>

#include <logger.h>

FJP::Logger *FJP::Logger::instance = NULL;

FJP::Logger::Logger() {
}

FJP::Logger *FJP::Logger::getInstance() {
    if (instance == NULL)
        instance = new Logger;
    return instance;
}

void FJP::Logger::log(const std::string &filename, uint32_t lineNumber, LogType logType, const char *message) {
    switch (logType) {
        case LogType::ERROR:
            log(filename, lineNumber, ERR_SYMBOL, RED, message);
            break;
        case LogType::DEBUG:
            #ifdef DEBUG_LEVEL_ON
                log(filename, lineNumber, DEBUG_SYMBOL, YELLOW, message);
            #endif
            break;
        case LogType::INFO:
            log(filename, lineNumber, INFO_SYMBOL, GREEN, message);
            break;
    }
}

void FJP::Logger::log(const std::string &filename, uint32_t lineNumber, char logSymbol, const std::string &color, const char *message) {
    #ifndef COLORED_OUTPUT
     (void)color;
    #endif

    #ifndef DEBUG_LEVEL_ON
        (void)filename;
        (void)lineNumber;
    #endif

    std::cout
    #ifdef COLORED_OUTPUT
        << color
    #endif
       << "["  << logSymbol << "]"
    #ifdef COLORED_OUTPUT
       << RESET
    #endif
    #ifdef DEBUG_LEVEL_ON
        << "[" << filename << " | #" << lineNumber << "]"
    #endif
        << " " << message << "\n";
}