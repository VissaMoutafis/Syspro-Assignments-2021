PROGRAM := travelMonitor

OBJS += $(BUILD)/travelMonitor.o $(BUILD)/travelMonitor_exec.o 

ARGS := 

$(BIN)/$(PROGRAM) : $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(BIN)/$(PROGRAM) $(LIBS)
	@cp $(BIN)/$(PROGRAM) $(MAKE-DIR)

run :
	./$(PROGRAM) $(ARGS)

.PHONY : clean
clean :
	@$(RM) -f $(MAKE-DIR)/$(PROGRAM) $(BIN)/$(PROGRAM) $(BUILD)/travelMonitor.o $(BUILD)/travelMonitor_exec.o

$(BUILD)/%.o : ./%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o : $(SRC)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o : $(SRC)/modules/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o : $(SRC)/Utilities/%.c
	@$(CC) $(CFLAGS) -c $< -o $@