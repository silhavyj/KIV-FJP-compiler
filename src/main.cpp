#include <iostream>

#include <cxxopts.hpp>

#include <vm.h>
#include <ivm.h>
#include <errors.h>
#include <lexer.h>
#include <ilexer.h>
#include <parser.h>
#include <iparser.h>
#include <logger.h>

int main(int argc, char *argv[]) {
    // Create argument parser.
    cxxopts::ParseResult arg;
    cxxopts::Options options("./fjp <input>", "FJP compiler");

    // Add options as to with what flags the application could be run.
    options.add_options()
            ("d,debug", "generates the following files: tokens.json, code.pl0-asm, stacktrace.txt", cxxopts::value<bool>()->default_value("false"))
            ("r,run", "executes the program", cxxopts::value<bool>()->default_value("false"))
            ("h,help" , "print help")
            ;

    // Parse the arguments passed in from the terminal.
    arg = options.parse(argc, argv);

    // If the 'help' option is present, print out the help menu and terminate the application.
    if (arg.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // Make sure the user has provided the path to the input file.
    if (argc < 2) {
        FJP::exitProgramWithError("\nERR: Input file is not specified!\n"
                                            "    Run './fjp --help'\n", 4);
    }

    // Check if the user turned on debugging.
    bool debug = arg["debug"].as<bool>();

    // Create instances of all three logical parts that make up the whole application.
    FJP::IParser *parser = FJP::Parser::getInstance();
    FJP::ILexer *lexer = FJP::Lexer::getInstance();
    FJP::IVM *vm = FJP::VirtualMachine::getInstance();

    // Parse the input program and generate output code.
    lexer->init(argv[1], debug);
    auto program = parser->parse(lexer, debug);

    // If the user added the 'run' option, execute the program.
    if (arg["run"].as<bool>()) {
        vm->execute(program, debug);
    }
    return 0;
}