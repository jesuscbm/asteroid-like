LD_FLAGS=-lSDL3
INCLUDE_DIR=./src/
SRC_DIR=./src/
OBJ_DIR=.
CC=gcc
CFLAGS=-Wall -g

all: asteroid

asteroid: $(OBJ_DIR)/main.o $(OBJ_DIR)/game.o
	$(CC) $^ $(CFLAGS) $(LD_FLAGS) -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(INCLUDE_DIR)/config.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/game.o: $(SRC_DIR)/game.c $(INCLUDE_DIR)/game.h $(INCLUDE_DIR)/config.h
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o asteroid
