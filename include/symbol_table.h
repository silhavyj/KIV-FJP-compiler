#pragma once

#include <list>
#include <string>
#include <unordered_set>
#include <iostream>

namespace FJP {

    /// Enumeration of all different symbol types that
    /// may occur in the source code (input file).
    enum SymbolType {
        SYMBOL_CONST = 1,  // 'const'
        SYMBOL_INT,        // 'int'
        SYMBOL_BOOL,       // 'bool'
        SYMBOL_INT_ARRAY,  // 'int[]'
        SYMBOL_BOOL_ARRAY, // 'bool[]'
        SYMBOL_FUNCTION,   // 'function'
        SYMBOL_LABEL,      // 'label:'

        SYMBOL_NOT_FOUND   // used as an indicator in case a symbol was not found
    };

    /// Definition of a symbol. The program keeps track of all symbols
    /// it has seen along with their information such as the address,
    /// level, size (in case of an array), name, etc.
    struct Symbol {
        FJP::SymbolType symbolType; ///< type of the symbol
        std::string name;           ///< name of the symbol such as 'arr', 'x', 'label', ...
        int value;                  ///< value of the symbol e.g. '5', 'true', ...
        int level;                  ///< level (depth) of the symbol
        int address;                ///< address of the symbol within the stack
        int size;                   ///< size of the symbol (used in case of an array - number of elements)

        /// Overloaded '==' operator. This method is used
        /// for comparing two different symbols. They're
        /// classified the same by their name.
        /// \param other instance of Symbol we're comparing to this instance.
        /// \return true/false depending on whether their names are the same or not.
        bool operator==(const FJP::Symbol &other) const;
    };

    /// Structure used for calculating the hash value of a symbol.
    /// This structured is passed into an std::unordered_set, so it
    /// knows to find particular symbols - taking advantage of its
    /// constant look-up times.
    struct SymbolHash {

        /// Calculates the hash value of the symbol passed as a parameter.
        /// \param symbol instance of Symbol of which we want to calculate the hash value
        /// \return symbol's hash value
        size_t operator()(const FJP::Symbol &symbol) const;
    };

    /// Definition of a "stack frame". A frame holds all symbols that
    /// were defined at that particular level. Whenever a function
    /// is called, a new frame is created.
    struct Frame {

        std::unordered_set<FJP::Symbol, FJP::SymbolHash> symbols; ///< set of symbols defined within the frame
    };

    /// Definition of a symbol table. The symbol table works as
    /// a stack of frames that are being created and destroyed
    /// whenever a function is called.
    class SymbolTable {
    private:
        std::list<FJP::Frame> frames; ///< list of frames that is being treated as a stack

    public:
        /// Creates a new frame. This method is called upon a function call.
        void createFrame();

        /// Destroys a frame that is on the top of the stack. This method is
        /// called when a function call terminates.
        void destroyFrame();

        /// Returns true/false depending on whether a symbol is found
        /// in the symbol table. It goes over all frames that the symbol
        /// table currently holds.
        /// \param name the name of the symbol we're looking for
        /// \return true/false depending on whether the symbol is found or not.
        bool existsSymbol(const std::string &name) const;

        /// Finds a symbol by its name within the symbol table. It takes into
        /// account all frames that are currently held in the symbol table.
        /// \param name the name of the symbol we're looking for
        /// \return instance of Symbol regardless of the symbol having been found or not.
        ///         If the symbol has not been found, its type is set to SYMBOL_NOT_FOUND
        FJP::Symbol findSymbol(const std::string &name) const;

        /// Adds a symbol into the frame on the top of the stack (current function/depth/level).
        /// \param symbol instance of Symbol that will be added into the current frame
        void addSymbol(FJP::Symbol symbol);

        /// Returns a list of all frames currently held in the symbol table.
        /// \return std::list of frames
        std::list<FJP::Frame> &getFrames();

        /// Returns the current depth/level. The depth depends on how many functions
        /// have been called so far (e.g. recursion). The current depth is determined
        /// by the size of the list of frames held in the symbol table.
        int getDepthLevel() const;

        /// Sets a symbol as an array. The symbol is given by its name,
        /// hence it needs to be found first.
        /// \param name the name of the symbol that is supposed to be already stored in the symbol table
        /// \param size the size of the newly created array
        void makeArray(const std::string &name, int size);
    };

    /// Prints out the contents of the symbol table
    /// \param out the output stream the symbol table will be printed out into.
    /// \param symbolTable the symbol table itself
    /// \return modified output stream (extended by the symbol table)
    std::ostream &operator<<(std::ostream &out, FJP::SymbolTable &symbolTable);

    /// Prints out the contents of a frame held in the symbol table.
    /// \param out the output stream the frame will be printed out into.
    /// \param frame the frame itself.
    /// \return modified output stream (extended by the frame)
    std::ostream &operator<<(std::ostream &out, const FJP::Frame &frame);

    /// Prints out a symbol.
    /// \param out the output stream the symbol will be printed out into.
    /// \param symbol the symbol itself
    /// \return modified output stream (extended by the symbol)
    std::ostream &operator<<(std::ostream &out, const FJP::Symbol &symbol);

    /// Prints out a symbol type
    /// \param out the output stream the symbol type will be printed out into.
    /// \param symbolType the symbol type itself
    /// \return modified output stream (extended by symbol type)
    std::ostream &operator<<(std::ostream &out, const FJP::SymbolType &symbolType);
}