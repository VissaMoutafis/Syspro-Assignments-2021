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

include $(SRC)/src.inc

ifeq ($(DEBUG), 1) 
	CFLAGS += -D DEBUG -g3
endif

$(PROGRAM): $(BUILD)/$(PROGRAM)

$(BUILD)/$(PROGRAM) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/$(PROGRAM) $(LIBRARIES)
	cp $(BUILD)/$(PROGRAM) ./$(PROGRAM)

$(BIN)/%.o : $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run : $(BUILD)/$(PROGRAM)
	./$(BUILD)/$(PROGRAM) $(ARGS)

all: clean $(BUILD)/$(PROGRAM)

clean:
	rm -rf $(BUILD)/$(PROGRAM) $(OBJS) ./$(PROGRAM)