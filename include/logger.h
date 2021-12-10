#pragma once

#include <string>
#include <cstdint>

namespace FJP {

    #define DEBUG_LEVEL_ON ///< turn the debug level on
    #define COLORED_OUTPUT ///< make the output colorful

    // Some helper macros to make things easier when debugging

    #define LOG_ERR(msg)   (FJP::Logger::getInstance()->log(__FILE__, __LINE__, FJP::Logger::LogType::ERROR, (msg)))
    #define LOG_DEBUG(msg) (FJP::Logger::getInstance()->log(__FILE__, __LINE__, FJP::Logger::LogType::DEBUG, (msg)))
    #define LOG_INFO(msg)  (FJP::Logger::getInstance()->log(__FILE__, __LINE__, FJP::Logger::LogType::INFO,  (msg)))

    /// This class represents the logger of the application.
    /// It is mainly used for debugging purposes.
    class Logger {
    public:
        static constexpr const char *RESET  = "\033[0m";  ///< reset color code
        static constexpr const char *RED    = "\033[31m"; ///< red color code
        static constexpr const char *GREEN  = "\033[32m"; ///< green color code
        static constexpr const char *YELLOW = "\033[33m"; ///< yellow color code

        static constexpr char ERR_SYMBOL   = '-';         ///< ERROR indicator
        static constexpr char INFO_SYMBOL  = '+';         ///< SUCCESS indicator
        static constexpr char DEBUG_SYMBOL = '?';         ///< DEBUG indicator

        /// Enumeration of different log levels
        enum LogType {
            ERROR, ///< error level of debugging
            DEBUG, ///< debug level of debugging
            INFO   ///< info level of debugging
        };

    private:
        static Logger *instance; ///< The instance of the class

    private:
        /// Constructor - creates an instance of the class
        Logger();

        /// Delete copy constructor of the class
        Logger(Logger &) = delete;

        /// Deleted assign operator of the class
        void operator=(Logger const &) = delete;

        /// Creates a log.
        /// \param filename the name of the file the log came from
        /// \param lineNumber the number of the line in the file the log came from
        /// \param logSymbol log symbol (indicator - error/debug/info)
        /// \param color color in which the log will be printed out
        /// \param message the log message itself
        void log(const std::string &filename, uint32_t lineNumber, char logSymbol, const std::string &color, const char *message);

    public:
        /// Return the instance of the class
        /// \return the instance of the class
        static Logger *getInstance();

        /// Creates a log.
        /// \param filename the name of the file the log came from
        /// \param lineNumber the number of the line in the file the log came from
        /// \param logType log symbol (indicator - error/debug/info)
        /// \param message the log message itself
        void log(const std::string &filename, uint32_t lineNumber, LogType logType, const char *message);
    };
}