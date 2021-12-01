#include <cassert>

#include "symbol_table.h"

bool FJP::Symbol::operator==(const FJP::Symbol &other) const {
    return this->name == other.name;
}

size_t FJP::SymbolHash::operator()(const FJP::Symbol &symbol) const {
    return std::hash<std::string>{}(symbol.name);
}

void FJP::SymbolTable::createFrame() {
    frames.push_back({});
}

void FJP::SymbolTable::destroyFrame() {
    frames.pop_back();
}

bool FJP::SymbolTable::existsSymbol(const std::string &name) const {
    FJP::Symbol symbol = findSymbol(name);
    return symbol.symbolType != FJP::SymbolType::SYMBOL_NOT_FOUND;
}

int FJP::SymbolTable::getDepthLevel() const {
    return std::max(0, static_cast<int>(frames.size()) - 1);
}

FJP::Symbol FJP::SymbolTable::findSymbol(const std::string &name) const {
    FJP::Symbol symbol = { FJP::SymbolType::SYMBOL_NOT_FOUND, name, 0, 0, 0, 0};
    for (auto iter = frames.rbegin(); iter != frames.rend(); ++iter) {
        auto symbolIt = iter->symbols.find(symbol);
        if (symbolIt != iter->symbols.end()) {
            return *symbolIt;
        }
    }
    return symbol;
}

void FJP::SymbolTable::addSymbol(FJP::Symbol symbol) {
    assert(frames.empty() == false);
    frames.rbegin()->symbols.insert(symbol);
}

void FJP::SymbolTable::makeArray(const std::string &name, int size) {
    FJP::Symbol symbol = { FJP::SymbolType::SYMBOL_NOT_FOUND, name, 0, 0, 0, 0};
    for (auto iter = frames.rbegin(); iter != frames.rend(); ++iter) {
        auto symbolIt = iter->symbols.find(symbol);
        if (symbolIt != iter->symbols.end()) {
            FJP::Symbol updatedSymbol = *symbolIt;
            updatedSymbol.size = size;

            switch (updatedSymbol.symbolType) {
                case FJP::SymbolType::SYMBOL_INT:
                    updatedSymbol.symbolType = FJP::SymbolType::SYMBOL_INT_ARRAY;
                    break;
                case FJP::SymbolType::SYMBOL_BOOL:
                    updatedSymbol.symbolType = FJP::SymbolType::SYMBOL_BOOL_ARRAY;
                    break;
                default:
                    break;
            }
            iter->symbols.erase(*symbolIt);
            iter->symbols.insert(updatedSymbol);
            return;
        }
    }
}

std::list<FJP::Frame> &FJP::SymbolTable::getFrames() {
    return frames;
}

std::ostream &FJP::operator<<(std::ostream &out, FJP::SymbolTable &symbolTable) {
    out << "============\n";
    auto frames = symbolTable.getFrames();
    for (const auto& frame : frames)
        out << frame;
    out << "============\n";
    return out;
}

std::ostream &FJP::operator<<(std::ostream &out, const FJP::Frame &frame) {
    out << "------------\n";
    for (const auto &symbol : frame.symbols)
        out << symbol << '\n';
    out << "------------\n";
    return out;
}

std::ostream &FJP::operator<<(std::ostream &out, const FJP::Symbol &symbol) {
    out << "type: "     << symbol.symbolType
        << " | name: "    << symbol.name
        << " | value: "   << symbol.value
        << " | level: "   << symbol.level
        << " | address: " << symbol.address;
    return out;
}

std::ostream &FJP::operator<<(std::ostream &out, const FJP::SymbolType &symbolType) {
    switch (symbolType) {
        case FJP::SymbolType::SYMBOL_CONST:
            out << "const";
            break;
        case FJP::SymbolType::SYMBOL_INT:
            out << "int";
            break;
        case FJP::SymbolType::SYMBOL_BOOL:
            out << "bool";
            break;
        case FJP::SymbolType::SYMBOL_FUNCTION:
            out << "function";
            break;
        case FJP::SymbolType::SYMBOL_NOT_FOUND:
            out << "not-found";
            break;
        default:
            out << "unknown";
    }
    return out;
}