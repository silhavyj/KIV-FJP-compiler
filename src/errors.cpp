#include <iostream>

#include <errors.h>

void FJP::exitProgramWithError(const char *errMsg, int errCode) {
    std::cout << errMsg << std::endl;

    // Exit the program.
    exit(errCode);
}

void FJP::exitProgramWithError(const char *methodName, const char *errMsg, int errCode, int lineNumber) {
    // [<method name>][#<line>] <error message>
    std::cout << "[" << methodName << "][#" << lineNumber <<  "] " << errMsg << std::endl;

    // Exit the program.
    exit(errCode);
}