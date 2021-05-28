PROGRAM := monitorServer

CFLAGS += -D MONITOR=1

CUR_OBJS += $(BUILD)/monitor.o \
		$(BUILD)/monitor_exec.o \
		$(BUILD)/monitorIPC.o \
		$(BUILD)/monitorMessageHandlers.o \
		$(BUILD)/monitor_threading.o

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