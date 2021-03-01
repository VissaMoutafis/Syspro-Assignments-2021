PROGRAM = main

CC = gcc

INCLUDE = ./include

SRC  = ./src

BIN = ./bin

BUILD = ./build

OBJS = 

LIBRARIES = -lm

CFLAGS = -g3 -Wall -Werror -I $(INCLUDE)

TESTCASE = 

ARGS =

include $(SRC)/src.inc

$(BUILD)/$(PROGRAM) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/$(PROGRAM) $(LIBRARIES)
	cp $(BUILD)/$(PROGRAM) ./$(PROGRAM)

$(BIN)/%.o : $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)/$(PROGRAM) $(OBJS)

clean-database: 
	rm -rf $(DATABASE-FILES)/*

run-noargs : $(BUILD)/$(PROGRAM)
	$(BUILD)/$(PROGRAM)
