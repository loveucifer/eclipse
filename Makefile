CXX      := c++
CXXFLAGS := -std=c++17 -Wall -Wextra -g
LDFLAGS  :=

SDL_CFLAGS   := $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer)
SDL_LIBS     := $(shell pkg-config --libs   sdl2 SDL2_image SDL2_ttf SDL2_mixer)
LUA_CFLAGS   := -I/opt/homebrew/opt/lua@5.4/include/lua
LUA_LIBS     := -L/opt/homebrew/opt/lua@5.4/lib -llua

CXXFLAGS += $(SDL_CFLAGS) $(LUA_CFLAGS)
LDFLAGS  += $(SDL_LIBS)   $(LUA_LIBS)

SRC := main.cpp
OBJ := main.o
BIN := main

.PHONY: all build run clean

all build: $(BIN)

$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CXX) $^ $(LDFLAGS) -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJ) $(BIN)
