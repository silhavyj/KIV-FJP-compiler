#pragma once

namespace FJP {

    void exitProgramWithError(const char *errMsg, int errCode);
    void exitProgramWithError(const char *methodName, const char *errMsg, int errCode, int lineNumber);

    namespace IOErrors {
        static constexpr const char *ERROR_00 = "input file not found";
        static constexpr const char *ERROR_01 = "could not open output file";
    }

    namespace CompilationErrors {
        static constexpr const char *ERROR_00 = "program is incomplete";
        static constexpr const char *ERROR_01 = "end of file due to an unclosed comment";
        static constexpr const char *ERROR_02 = "number is too long";
        static constexpr const char *ERROR_03 = "identifier is too long";
        static constexpr const char *ERROR_04 = "unknown character";
        static constexpr const char *ERROR_05 = "no token to return to";
    }

    namespace RuntimeErrors {
        static constexpr const char *ERROR_00 = "stack overflow error";
        static constexpr const char *ERROR_01 = "arithmetic exception: division by zero error";
        static constexpr const char *ERROR_02 = "arithmetic exception: integer overflow";
    }
}