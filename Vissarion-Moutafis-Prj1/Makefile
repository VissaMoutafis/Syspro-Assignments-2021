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

ARGS =

include $(SRC)/src.inc

ifeq ($(DEBUG), 1) 
	CFLAGS += -D DEBUG -g3
endif

$(BUILD)/$(PROGRAM) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/$(PROGRAM) $(LIBRARIES)
	cp $(BUILD)/$(PROGRAM) ./$(PROGRAM)

$(BIN)/%.o : $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)/$(PROGRAM) $(OBJS) ./$(PROGRAM)

clean-database: 
	rm -rf $(DATABASE-FILES)/*

run : $(BUILD)/$(PROGRAM)
	./$(BUILD)/$(PROGRAM) -c inputFile -b 100000

testcases: testFile.sh
	./testFile.sh viruses.csv countries.csv 100 0