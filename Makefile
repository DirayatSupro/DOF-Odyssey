PROJECT_NAME := odyssey
SRC_DIR := src
BUILD_DIR := build

SOURCES := $(shell find $(SRC_DIR) -name '*.c')
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

CC := gcc
CSTD := -std=c11
WARN := -Wall -Wextra
OPT := -O1
INCLUDES := -I$(SRC_DIR)

UNAME_S := $(shell uname -s 2>/dev/null)

ifeq ($(OS),Windows_NT)
    PLATFORM := WINDOWS
else ifeq ($(UNAME_S),Darwin)
    PLATFORM := MACOS
else ifeq ($(UNAME_S),Linux)
    PLATFORM := LINUX
else
    PLATFORM := WINDOWS
endif

ifeq ($(PLATFORM),MACOS)
    RAYLIB_PREFIX := $(shell brew --prefix raylib 2>/dev/null)
    ifeq ($(RAYLIB_PREFIX),)
        RAYLIB_PREFIX := /opt/homebrew
    endif
    INCLUDES += -I$(RAYLIB_PREFIX)/include
    LDFLAGS := -L$(RAYLIB_PREFIX)/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreAudio -framework AudioToolbox
endif

ifeq ($(PLATFORM),LINUX)
    LDFLAGS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

ifeq ($(PLATFORM),WINDOWS)
    EXE_EXT := .exe
    LDFLAGS := -lraylib -lopengl32 -lgdi32 -lwinmm
endif

TARGET := $(BUILD_DIR)/$(PROJECT_NAME)$(EXE_EXT)

.PHONY: all run clean dirs

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CSTD) $(WARN) $(OPT) $(INCLUDES) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
