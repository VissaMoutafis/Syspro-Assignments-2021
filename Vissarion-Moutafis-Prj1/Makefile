PROGRAM = vaccineMonitor

CC = gcc

INCLUDE = ./include

SRC  = ./src

BIN = ./bin

BUILD = ./build

OBJS = 

LIBRARIES = -lm

CFLAGS =  -Wall -Werror -I $(INCLUDE)

TESTCASE = 

ARGS = -c inputFile -b 100000

DEBUG_MSG = OFF

include $(SRC)/src.inc

ifeq ($(DEBUG), 1) 
	CFLAGS += -D DEBUG -g3
	DEBUG_MSG = ON
endif

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

all: clean $(BUILD)/$(PROGRAM)

clean:
	@echo Cleaning Binaries...
	@rm -rf $(BUILD)/$(PROGRAM) $(OBJS) ./$(PROGRAM)
	@echo Done.

print-msg:
	@echo Building $(PROGRAM). DEBUG MODE: $(DEBUG_MSG)