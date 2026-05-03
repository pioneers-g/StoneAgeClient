# Stone Age Client Makefile
# Build configuration for Windows using MinGW GCC

CC = gcc
CFLAGS = -Wall -Wextra -O2 -DWIN32_LEAN_AND_MEAN -D_USE_MATH_DEFINES
LDFLAGS = -mwindows -lddraw -ldsound -ldinput -ldxguid -lwinmm -lws2_32 -limm32
INCLUDES = -I./include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files - include subdirectories
SRCS = $(wildcard $(SRC_DIR)/*.c)
SRCS += $(wildcard $(SRC_DIR)/battle/*.c)
SRCS += $(wildcard $(SRC_DIR)/character/*.c)
SRCS += $(wildcard $(SRC_DIR)/directx/*.c)
SRCS += $(wildcard $(SRC_DIR)/login/*.c)
SRCS += $(wildcard $(SRC_DIR)/map/*.c)
SRCS += $(wildcard $(SRC_DIR)/network/*.c)
SRCS += $(wildcard $(SRC_DIR)/pet/*.c)
SRCS += $(wildcard $(SRC_DIR)/protocol/*.c)
SRCS += $(wildcard $(SRC_DIR)/sound/*.c)
SRCS += $(wildcard $(SRC_DIR)/text_protocol/*.c)
SRCS += $(wildcard $(SRC_DIR)/ui/*.c)
SRCS += $(wildcard $(SRC_DIR)/shop/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/stoneage.exe

# Default target
all: dirs $(TARGET)

# Create directories
dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)/battle $(OBJ_DIR)/character $(OBJ_DIR)/directx $(OBJ_DIR)/login
	@mkdir -p $(OBJ_DIR)/map $(OBJ_DIR)/network $(OBJ_DIR)/pet $(OBJ_DIR)/protocol
	@mkdir -p $(OBJ_DIR)/sound $(OBJ_DIR)/text_protocol $(OBJ_DIR)/ui $(OBJ_DIR)/shop

# Link
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile root level
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Compile subdirectories
$(OBJ_DIR)/battle/%.o: $(SRC_DIR)/battle/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/character/%.o: $(SRC_DIR)/character/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/directx/%.o: $(SRC_DIR)/directx/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/login/%.o: $(SRC_DIR)/login/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/map/%.o: $(SRC_DIR)/map/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/network/%.o: $(SRC_DIR)/network/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/pet/%.o: $(SRC_DIR)/pet/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/protocol/%.o: $(SRC_DIR)/protocol/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/sound/%.o: $(SRC_DIR)/sound/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/text_protocol/%.o: $(SRC_DIR)/text_protocol/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/ui/%.o: $(SRC_DIR)/ui/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/shop/%.o: $(SRC_DIR)/shop/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

# Release build
release: CFLAGS += -DNDEBUG -Os
release: clean all

# Run
run: all
	./$(TARGET)

.PHONY: all dirs clean debug release run
