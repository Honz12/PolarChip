SRC_DIR=src
BUILD_DIR=build
CC=gcc

.PHONY: run

$(BUILD_DIR)/main.exe: $(BUILD_DIR) $(SRC_DIR)/main.c
	$(CC) $(SRC_DIR)/main.c -o $(BUILD_DIR)/main.exe

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

run: $(BUILD_DIR)/main.exe
	$(BUILD_DIR)/main.exe
