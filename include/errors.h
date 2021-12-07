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

        static constexpr const char *ERROR_06 = "array can be initialized only with a constant value";
        static constexpr const char *ERROR_07 = "number expected";
        static constexpr const char *ERROR_08 = "array has to be fully initialized";
        static constexpr const char *ERROR_09 = "array not found";
        static constexpr const char *ERROR_10 = "true/false value expected";
        static constexpr const char *ERROR_11 = "missing ':'";
        static constexpr const char *ERROR_12 = "missing '('";
        static constexpr const char *ERROR_13 = "missing ')'";
        static constexpr const char *ERROR_14 = "missing ':='";
        static constexpr const char *ERROR_15 = "missing ';'";
        static constexpr const char *ERROR_16 = "missing ']'";
        static constexpr const char *ERROR_17 = "missing '['";
        static constexpr const char *ERROR_18 = "missing '{'";
        static constexpr const char *ERROR_19 = "missing '}'";
        static constexpr const char *ERROR_21 = "missing '='";
        static constexpr const char *ERROR_22 = "expected variable, number, (, or missing ;";
        static constexpr const char *ERROR_23 = "name of a function expected";
        static constexpr const char *ERROR_24 = "identifier expected";
        static constexpr const char *ERROR_26 = "invalid array size provided";
        static constexpr const char *ERROR_28 = "unknown condition operator";
        static constexpr const char *ERROR_29 = "invalid datatype";
        static constexpr const char *ERROR_30 = "invalid identifier to print out";
        static constexpr const char *ERROR_31 = "invalid instanceof type";
        static constexpr const char *ERROR_32 = "invalid iterator type";
        static constexpr const char *ERROR_33 = "label expected";
        static constexpr const char *ERROR_34 = "literal expected";
        static constexpr const char *ERROR_35 = "literals must match the symbol type";
        static constexpr const char *ERROR_36 = "missing END symbol";
        static constexpr const char *ERROR_37 = "missing START symbol";
        static constexpr const char *ERROR_38 = "missing while (do-while)";
        static constexpr const char *ERROR_39 = "missing until (repeat-until)";
        static constexpr const char *ERROR_40 = "array size must be >= 1";
        static constexpr const char *ERROR_41 = "symbol does not refer to a variable";
        static constexpr const char *ERROR_42 = "the iterator and the array have to be the same type";
        static constexpr const char *ERROR_43 = "name is already taken";
        static constexpr const char *ERROR_44 = "missing identifier";
        static constexpr const char *ERROR_45 = "symbol not found";
    }

    namespace RuntimeErrors {
        static constexpr const char *ERROR_00 = "stack overflow error";
        static constexpr const char *ERROR_01 = "arithmetic exception: division by zero error";
        static constexpr const char *ERROR_02 = "arithmetic exception: integer overflow";
    }
}