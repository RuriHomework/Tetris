CXX := g++
CC := gcc
CXXFLAGS := -std=c++11 -O2 -Wall -pthread
CFLAGS := -std=c99 -O2 -Wall -pthread

BUILD_DIR := build
TARGET_TRAIN := $(BUILD_DIR)/train
TARGET_MAIN := $(BUILD_DIR)/main

SRC_TRAIN := train.cpp
SRC_MAIN := main.c

all: $(TARGET_TRAIN) $(TARGET_MAIN)

$(BUILD_DIR):
	@mkdir -p $@

$(TARGET_TRAIN): $(SRC_TRAIN) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET_MAIN): $(SRC_MAIN) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

run-train: $(TARGET_TRAIN)
	@./$(TARGET_TRAIN)

run: $(TARGET_MAIN)
	@./$(TARGET_MAIN)

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all build run-train run clean