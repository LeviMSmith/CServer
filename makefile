# Directories and common setup
BIN_DIR := bin
BUILD_DIR := build
SERVER_BIN := $(BIN_DIR)/server
CLIENT_BIN := $(BIN_DIR)/client
LIB_BIN := $(BUILD_DIR)/lib.a

SERVER_BUILD := $(BUILD_DIR)/server
CLIENT_BUILD := $(BUILD_DIR)/client

LIB_SOURCE_DIR := lib/src
LIB_INCLUDE := lib/include
LIB_BUILD := $(BUILD_DIR)/lib

.PHONY: all
all: $(SERVER_BIN) $(CLIENT_BIN)

CC := gcc
CFLAGS := -MMD -Wall -g -I$(LIB_INCLUDE)


# Utils lib that both server and client use
LIB_SOURCES := $(shell find $(LIB_SOURCE_DIR) -type f -name "*.c")
LIB_OBJECTS := $(patsubst $(LIB_SOURCE_DIR)/%.c,$(LIB_BUILD)/%.o,$(LIB_SOURCES))

$(LIB_BIN): $(LIB_OBJECTS) | $(BIN_DIR)
	ar rcs $@ $^

$(LIB_OBJECTS): $(LIB_BUILD)/%.o: $(LIB_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# The server program
SERVER_SOURCE_DIR := server
SERVER_SOURCES := $(shell find $(SERVER_SOURCE_DIR) -type f -name "*.c")
SERVER_OBJECTS := $(patsubst $(SERVER_SOURCE_DIR)/%.c,$(SERVER_BUILD)/%.o,$(SERVER_SOURCES))

$(SERVER_BIN): $(SERVER_OBJECTS) $(LIB_BIN) | $(BIN_DIR)
	$(CC) $^ -o $@

$(SERVER_OBJECTS): $(SERVER_BUILD)/%.o: $(SERVER_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# The client program
CLIENT_SOURCE_DIR := client
CLIENT_SOURCES := $(shell find $(CLIENT_SOURCE_DIR) -type f -name "*.c")
CLIENT_OBJECTS := $(patsubst $(CLIENT_SOURCE_DIR)/%.c,$(CLIENT_BUILD)/%.o,$(CLIENT_SOURCES))

$(CLIENT_BIN): $(CLIENT_OBJECTS) $(LIB_BIN) | $(BIN_DIR)
	$(CC) $(LFLAGS) $^ -o $@

$(CLIENT_OBJECTS): $(CLIENT_BUILD)/%.o: $(CLIENT_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# Utility targets
$(BIN_DIR):
	@mkdir -p $@

TMUX_SESSION := CServer

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)
	@rm -rf .cache
	@if tmux has-session -t $(TMUX_SESSION) 2>/dev/null; then \
		tmux kill-session -t $(TMUX_SESSION); \
	fi

.PHONY: run
run: $(SERVER_BIN) $(CLIENT_BIN)
	-tmux kill-session -t CServer 2>/dev/null || true
	tmux new-session -d -s $(TMUX_SESSION) \; \
		split-window -h \; \
		split-window -v
	tmux send-keys -t $(TMUX_SESSION):0.0 $(SERVER_BIN) C-m
	sleep 0.5
	tmux send-keys -t $(TMUX_SESSION):0.1 $(CLIENT_BIN) C-m
	tmux send-keys -t $(TMUX_SESSION):0.2 $(CLIENT_BIN) C-m
	tmux attach-session -t $(TMUX_SESSION)

