SRC_DIR=src
BUILD_DIR=build
CC=gcc
PLR_DIR=plr
PLR_ASM=python tools/assembler.py

.PHONY: run

$(BUILD_DIR)/main.exe: $(BUILD_DIR) $(SRC_DIR)/main.c $(SRC_DIR)/instruction_defines.h
	$(CC) $(SRC_DIR)/main.c -o $(BUILD_DIR)/main.exe

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

run: $(BUILD_DIR)/main.exe
	$(BUILD_DIR)/main.exe

plr: $(PLR_DIR)/main.plr
	$(PLR_ASM) $(PLR_DIR)/main.plr -o $(PLR_DIR)/out.bin
