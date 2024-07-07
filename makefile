CC = gcc
SRC_DIR = ./source/system_programs
BIN_DIR = ./bin
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%)
MAIN_SRC = ./source/shell.c
MAIN_EXEC = cseshell
INI_FILE = ./config/custom.ini

# Special rule for main executable
all: $(OBJECTS) $(MAIN_EXEC) copy_ini_file

$(BIN_DIR)/%: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $< -o $@

$(MAIN_EXEC): $(MAIN_SRC)
	$(CC) $< -o $@

# Copy .ini file to output directory
copy_ini_file:
	mkdir -p $(BIN_DIR)/config
	cp $(INI_FILE) $(BIN_DIR)/$(INI_FILE)

clean:
	rm -f $(OBJECTS) $(MAIN_EXEC)
