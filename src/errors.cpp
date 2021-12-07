#include <iostream>

#include <errors.h>

void FJP::exitProgramWithError(const char *errMsg, int errCode) {
    std::cout << errMsg << std::endl;
    exit(errCode);
}

void FJP::exitProgramWithError(const char *methodName, const char *errMsg, int errCode, int lineNumber) {
    std::cout << "[" << methodName << "][#" << lineNumber <<  "] " << errMsg << std::endl;
    exit(errCode);
}