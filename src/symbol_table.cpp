#include <cassert>

#include <symbol_table.h>

bool FJP::Symbol::operator==(const FJP::Symbol &other) const {
    // Check if the names are the same.
    return this->name == other.name;
}

size_t FJP::SymbolHash::operator()(const FJP::Symbol &symbol) const {
    // Calculates the hash value of the symbol. The hash value
    // is calculated by the symbol's name, so we can keep track
    // of all unique symbols.
    return std::hash<std::string>{}(symbol.name);
}

void FJP::SymbolTable::createFrame() {
    frames.push_back({});
}

void FJP::SymbolTable::destroyFrame() {
    frames.pop_back();
}

bool FJP::SymbolTable::existsSymbol(const std::string &name) const {
    // Find the symbol in the symbol table.
    FJP::Symbol symbol = findSymbol(name);

    // Check if the symbol has been found successfully.
    return symbol.symbolType != FJP::SymbolType::SYMBOL_NOT_FOUND;
}

int FJP::SymbolTable::getDepthLevel() const {
    // Returns the number of frames (depth) currently held in the symbol table.
    return std::max(0, static_cast<int>(frames.size()) - 1);
}

FJP::Symbol FJP::SymbolTable::findSymbol(const std::string &name) const {
    // Default symbol that will be returned if the symbol has not been found - symbol type = SYMBOL_NOT_FOUND.
    // We also use this symbol to find the searched symbol - the name is set to the searched one.
    FJP::Symbol symbol = { FJP::SymbolType::SYMBOL_NOT_FOUND, name, 0, 0, 0, 0};

    // Go through all the frames (from the last one back to the first one) and for each
    // frame, check if it holds the searched symbol. Once the symbol is found, return it.
    for (auto iter = frames.rbegin(); iter != frames.rend(); ++iter) {
        auto symbolIt = iter->symbols.find(symbol);
        if (symbolIt != iter->symbols.end()) {
            return *symbolIt;
        }
    }
    // Symbol has not been found.
    return symbol;
}

void FJP::SymbolTable::addSymbol(FJP::Symbol symbol) {
    assert(frames.empty() == false);
    frames.rbegin()->symbols.insert(symbol);
}

void FJP::SymbolTable::makeArray(const std::string &name, int size) {
    FJP::Symbol symbol = { FJP::SymbolType::SYMBOL_NOT_FOUND, name, 0, 0, 0, 0};

    // Go through all the frames in a reverse order.
    for (auto iter = frames.rbegin(); iter != frames.rend(); ++iter) {
        // Check if the current frame holds the symbol we're searching for.
        auto symbolIt = iter->symbols.find(symbol);
        if (symbolIt != iter->symbols.end()) {
            // Copy the symbol into a temporary one and set its new size
            // which happens to be the number of elements in the array.
            FJP::Symbol updatedSymbol = *symbolIt;
            updatedSymbol.size = size;

            // Change the symbol type from int to int[]
            // or from bool to bool[].
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
            // Remove the original symbol from the frame and replace it
            // with the updated one.
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