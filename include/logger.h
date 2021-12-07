#pragma once

#include <string>
#include <cstdint>

namespace FJP {

    #define DEBUG_LEVEL_ON
    #define COLORED_OUTPUT

    #define LOG_ERR(msg)   (FJP::Logger::getInstance()->log(__FILE__, __LINE__, FJP::Logger::LogType::ERROR, (msg)))
    #define LOG_DEBUG(msg) (FJP::Logger::getInstance()->log(__FILE__, __LINE__, FJP::Logger::LogType::DEBUG, (msg)))
    #define LOG_INFO(msg)  (FJP::Logger::getInstance()->log(__FILE__, __LINE__, FJP::Logger::LogType::INFO,  (msg)))

    class Logger {
    public:
        static constexpr const char *RESET  = "\033[0m";
        static constexpr const char *RED    = "\033[31m";
        static constexpr const char *GREEN  = "\033[32m";
        static constexpr const char *YELLOW = "\033[33m";

        static constexpr char ERR_SYMBOL   = '-';
        static constexpr char INFO_SYMBOL  = '+';
        static constexpr char DEBUG_SYMBOL = '?';

        enum LogType {
            ERROR,
            DEBUG,
            INFO
        };

    private:
        static Logger *instance;

    private:
        Logger();
        Logger(Logger &) = delete;
        void operator=(Logger const &) = delete;
        void log(const std::string &filename, uint32_t lineNumber, char logSymbol, const std::string &color, const char *message);

    public:
        static Logger *getInstance();
        void log(const std::string &filename, uint32_t lineNumber, LogType logType, const char *message);
    };
}