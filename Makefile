TARGET = fjp
CCX    = g++
FLAGS  = -Wall -O2 -std=c++17 -pedantic-errors -Wextra -Werror
SRC    = src
BIN    = bin
SOURCE = $(wildcard $(SRC)/*.cpp) 
OBJECT = $(patsubst %,$(BIN)/%, $(notdir $(SOURCE:.cpp=.o)))

all: $(TARGET)

$(TARGET) : $(OBJECT)
	$(CCX) $(FLAGS) -o $@ $^

$(BIN)/%.o : $(SRC)/%.cpp
	@mkdir -p $(BIN)
	$(CCX) $(FLAGS) -c $< -o $@

.PHONY clean:
clean:
	rm -rf $(BIN) $(TARGET) || true
	rm stacktrace.txt || true
	rm tokens.json || true
	rm code.pl0-asm || true
