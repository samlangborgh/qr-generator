# Based off of the Makefile found here: https://stackoverflow.com/a/30602701

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .

EXE := $(BIN_DIR)/qr
SRC := $(wildcard $(SRC_DIR)/*.c)
# Don't compile old test file
SRC := $(filter-out src/qrtest.c, $(SRC))
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS := -Wall -ggdb3
LDFLAGS :=
LDLIBS := -lm

.PHONY: all clean

all: $(EXE)

# Build the executable
$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Make required directories
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Remove build files
clean: 
	@$(RM) -rv $(EXE) $(OBJ_DIR)

-include $(OBJ:.o=.d)
