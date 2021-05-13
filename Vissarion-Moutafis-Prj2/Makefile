# root directory
MAKE-DIR = $(PWD)

# basic variables for dir hierarchy
INCLUDE := $(MAKE-DIR)/include
SRC := $(MAKE-DIR)/src
BIN := $(MAKE-DIR)/bin
BUILD := $(MAKE-DIR)/build

# basic enviroment variables
CC := gcc
CFLAGS := -Wall -I $(INCLUDE)
LIBS := -lm
OBJS := 

# debug switch message
DEBUG_MSG = OFF

# the directories of the .mk files needed
# for the app components
MONITOR := $(SRC)/Monitor
TRAVEL_MONITOR := $(SRC)/TravelMonitor

# include the basic objective files
include $(SRC)/src.inc

# define some global variables
export MAKE-DIR SRC BUILD BIN CFLAGS CC LIBS OBJS

# check if debug mode is enabled
ifeq ($(DEBUG), 1) 
	CFLAGS += -D DEBUG -g3
	DEBUG_MSG = ON
endif


# program name
PROGRAM := travelMonitor
# args for <run>
ARGS := -m 3 -b 7 -s 100000 -i testcases/myinput_dir

.PHONY : all
all : print-msg 
	@echo "Building Monitor module..."
	@$(MAKE) -C $(MONITOR) -f monitor.mk 
	@echo "Done"
	@echo "Building Travel Monitor module..."	
	@$(MAKE) -C $(TRAVEL_MONITOR) -f travel_monitor.mk
	@echo Done.
		
.PHONY : monitor
monitor :
	@echo "Building Monitor module..."
	@$(MAKE) -C $(MONITOR) -f monitor.mk 
	@echo Done.

.PHONY : travel_monitor
travel_monitor :	
	@echo "Building Travel Monitor module..."	
	@$(MAKE) -C $(TRAVEL_MONITOR) -f travel_monitor.mk
	@echo Done!

.PHONY : run
run : all
	$(BIN)/$(PROGRAM) $(ARGS)

.PHONY : re-run
re-run: clean run

.PHONY : valgrind-run 
valgrind-run : $(BIN)/$(PROGRAM)
	valgrind \
	--leak-check=full \
	--show-leak-kinds=all \
	--trace-children=yes \
	$(BIN)/$(PROGRAM) $(ARGS)

.PHONY : clean
clean:
	@$(MAKE) -C $(MONITOR) -f monitor.mk clean
	@$(MAKE) -C $(TRAVEL_MONITOR) -f travel_monitor.mk clean
	@$(RM) -f $(OBJS)

.PHONY : clean-logs
clean-logs:
	@echo "Cleaning logs..."
	@$(RM) -rf logs
	@echo "Done!"

.PHONY : print-msg
print-msg:
	@echo "DEBUG MODE: $(DEBUG_MSG)"