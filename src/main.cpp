#include <iostream>

#include "../lib/cxxopts.hpp"

#include "vm.h"
#include "errors.h"
#include "lexer.h"
#include "parser.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    cxxopts::ParseResult arg;
    cxxopts::Options options("./fjp <input>", "FJP compiler");

    options.add_options()
            ("d,debug", "generates the following files: tokens.json, code.pl0-asm, stacktrace.txt", cxxopts::value<bool>()->default_value("false"))
            ("r,run", "executes the program", cxxopts::value<bool>()->default_value("false"))
            ("h,help" , "print help")
            ;

    arg = options.parse(argc, argv);
    if (arg.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (argc < 2) {
        FJP::exitProgramWithError("\nERR: Input file is not specified!\n"
                                            "    Run './fjp --help'\n", 4);
    }

    bool debug = arg["debug"].as<bool>();
    auto parser = FJP::Parser::getInstance();
    auto lexer = FJP::Lexer::getInstance();
    auto vm = FJP::VirtualMachine::getInstance();

    lexer->init(argv[1], debug);
    auto program = parser->parse(lexer, debug);

    if (arg["run"].as<bool>()) {
        vm->execute(program, debug);
    }

    return 0;
}