PROGRAM := travelMonitor

CUR_OBJS = $(BUILD)/travelMonitor.o \
		$(BUILD)/travelMonitor_exec.o \
		$(BUILD)/travelMonitorIPC.o \
		$(BUILD)/travelMonitorMessageHandlers.o \
		$(BUILD)/travelMonitor_setup.o\
		$(BUILD)/travelMonitor_sigactions.o

ARGS := 

$(BIN)/$(PROGRAM) : $(OBJS) $(CUR_OBJS)
	@$(CC) $(CFLAGS) $(OBJS) $(CUR_OBJS) -o $(BIN)/$(PROGRAM) $(LIBS)
	@cp $(BIN)/$(PROGRAM) $(MAKE-DIR)

run :
	./$(PROGRAM) $(ARGS)

.PHONY : clean
clean :
	@$(RM) -f $(MAKE-DIR)/$(PROGRAM) $(BIN)/$(PROGRAM) $(CUR_OBJS)

$(BUILD)/%.o : ./%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o : $(SRC)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o : $(SRC)/modules/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o : $(SRC)/Utilities/%.c
	@$(CC) $(CFLAGS) -c $< -o $@