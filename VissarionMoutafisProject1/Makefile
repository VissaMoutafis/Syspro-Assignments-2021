PROGRAM = vaccineMonitor

CC = gcc

INCLUDE = ./include

SRC  = ./src

BIN = ./bin

BUILD = ./build

OBJS = 

LIBRARIES = -lm

CFLAGS = -Wall -Werror -I $(INCLUDE)

TESTCASE = 

ARGS = -c inputFile -b 100000

DEBUG_MSG = OFF

include $(SRC)/src.inc

ifeq ($(DEBUG), 1) 
	CFLAGS += -D DEBUG -g3
	DEBUG_MSG = ON
endif

all: $(BUILD)/$(PROGRAM)

$(PROGRAM): $(BUILD)/$(PROGRAM)

$(BUILD)/$(PROGRAM) : print-msg $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/$(PROGRAM) $(LIBRARIES)
	@cp $(BUILD)/$(PROGRAM) ./$(PROGRAM)
	@echo Done.

$(BIN)/%.o : $(SRC)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN)/%.o : $(SRC)/modules/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

run : $(BUILD)/$(PROGRAM)
	./$(BUILD)/$(PROGRAM) $(ARGS)

re-run: clean $(BUILD)/$(PROGRAM)

valgrind-run : $(BUILD)/$(PROGRAM)
	valgrind --leak-check=full --show-leak-kinds=all ./$(BUILD)/$(PROGRAM) $(ARGS)

clean:
	@echo Cleaning Binaries...
	@rm -rf $(BUILD)/$(PROGRAM) $(OBJS) ./$(PROGRAM)
	@echo Done.

print-msg:
	@echo "DEBUG MODE: $(DEBUG_MSG)\nBuilding $(PROGRAM)..."