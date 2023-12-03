BIN_DIR := bin
BUILD_DIR := build
SERVER_BIN := $(BIN_DIR)/server
CLIENT_BIN := $(BIN_DIR)/client

SERVER_BUILD := $(BUILD_DIR)/server
CLIENT_BUILD := $(BUILD_DIR)/client

.PHONY: all
all: $(SERVER_BIN) $(CLIENT_BIN)

CC := gcc
CFLAGS := -MMD -Wall -g


SERVER_SOURCE_DIR := server
SERVER_SOURCES := $(shell find $(SERVER_SOURCE_DIR) -type f -name "*.c")
SERVER_OBJECTS := $(patsubst $(SERVER_SOURCE_DIR)/%.c,$(SERVER_BUILD)/%.o,$(SERVER_SOURCES))

$(SERVER_BIN): $(SERVER_OBJECTS) | $(BIN_DIR)
	$(CC) $^ -o $@

$(SERVER_OBJECTS): $(SERVER_BUILD)/%.o: $(SERVER_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

CLIENT_SOURCE_DIR := server
CLIENT_SOURCES := $(shell find $(CLIENT_SOURCE_DIR) -type f -name "*.c")
CLIENT_OBJECTS := $(patsubst $(CLIENT_SOURCE_DIR)/%.c,$(CLIENT_BUILD)/%.o,$(CLIENT_SOURCES))

$(CLIENT_BIN): $(CLIENT_OBJECTS) | $(BIN_DIR)
	$(CC) $^ -o $@

$(CLIENT_OBJECTS): $(CLIENT_BUILD)/%.o: $(CLIENT_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


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
	@if tmux has-session -t $(TMUX_SESSION) 2>/dev/null; then \
		tmux kill-session -t $(TMUX_SESSION); \
	fi
	tmux new-session -d -s $(TMUX_SESSION) \; \
  	split-window -h \; \
  	split-window -v \; \
  	send-keys -t $(TMUX_SESSION):0.0 $(SERVER_BIN) C-m \; \
  	send-keys -t $(TMUX_SESSION):0.1 $(CLIENT_BIN) C-m \; \
  	send-keys -t $(TMUX_SESSION):0.2 $(CLIENT_BIN) C-m
	tmux attach-session -t $(TMUX_SESSION)
