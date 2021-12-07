#pragma once

#include <list>
#include <string>
#include <unordered_set>
#include <iostream>

namespace FJP {

    enum SymbolType {
        SYMBOL_CONST = 1,
        SYMBOL_INT,
        SYMBOL_BOOL,
        SYMBOL_INT_ARRAY,
        SYMBOL_BOOL_ARRAY,
        SYMBOL_FUNCTION,
        SYMBOL_NOT_FOUND,
        SYMBOL_LABEL
    };

    struct Symbol {
        FJP::SymbolType symbolType;
        std::string name;
        int value;
        int level;
        int address;
        int size;

        bool operator==(const FJP::Symbol &other) const;
    };

    struct SymbolHash {
        size_t operator()(const FJP::Symbol &symbol) const;
    };

    struct Frame {
        std::unordered_set<FJP::Symbol, FJP::SymbolHash> symbols;
    };

    class SymbolTable {
    private:
        std::list<FJP::Frame> frames;

    public:
        void createFrame();
        void destroyFrame();
        bool existsSymbol(const std::string &name) const;
        FJP::Symbol findSymbol(const std::string &name) const;
        void addSymbol(FJP::Symbol symbol);
        std::list<FJP::Frame> &getFrames();
        int getDepthLevel() const;
        void makeArray(const std::string &name, int size);
    };

    std::ostream &operator<<(std::ostream &out, FJP::SymbolTable &symbolTable);
    std::ostream &operator<<(std::ostream &out, const FJP::Frame &frame);
    std::ostream &operator<<(std::ostream &out, const FJP::Symbol &symbol);
    std::ostream &operator<<(std::ostream &out, const FJP::SymbolType &symbolType);
}