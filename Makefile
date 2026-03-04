CC = clang
CFLAGS = -g -Wall -O2 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -std=c11
INC = -I include/

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

NAIF_SRC = $(SRC_DIR)/solveur_naif.c
RUSE_SRC = $(SRC_DIR)/solveur_ruse.c

NAIF_OBJ = $(BUILD_DIR)/solveur_naif.o
RUSE_OBJ = $(BUILD_DIR)/solveur_ruse.o

NAIF_BIN = $(BIN_DIR)/solveur_naif
RUSE_BIN = $(BIN_DIR)/solveur_ruse

all: $(NAIF_BIN) $(RUSE_BIN)

# -------- Compilation en .o --------

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# -------- Linkage --------

$(NAIF_BIN): $(NAIF_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(RUSE_BIN): $(RUSE_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# -------- Nettoyage --------

clean:
	rm -f $(BUILD_DIR)/*.o $(BIN_DIR)/*

.PHONY: all clean
