CXX := g++
CC := gcc
CXXFLAGS := -std=c++11 -O2 -Wall -pthread
CFLAGS := -std=c99 -O2 -Wall -pthread

BUILD_DIR := build
TARGET_MAIN := $(BUILD_DIR)/main

SRC_MAIN := main.c

all: $(TARGET_TRAIN) $(TARGET_MAIN)

$(BUILD_DIR):
	@mkdir -p $@

$(TARGET_MAIN): $(SRC_MAIN) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET_MAIN)
	@./$(TARGET_MAIN)

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all build run clean