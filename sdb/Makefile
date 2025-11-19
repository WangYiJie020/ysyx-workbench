CXX_SRC = $(wildcard src/*.cpp)
C_SRC = $(wildcard src/*.c)

BUILD_DIR = ./build

CXX_OBJ = $(CXX_SRC:src/%.cpp=$(BUILD_DIR)/%.o)
C_OBJ = $(C_SRC:src/%.c=$(BUILD_DIR)/%.o)
OBJS = $(CXX_OBJ) $(C_OBJ)
DEPS = $(OBJS:.o=.d)

INC_PATH = ./include ./src

LIBCAPSTONE = tools/capstone/repo/libcapstone.so.5
INC_PATH += $(abspath tools/capstone/repo/include)

INCLUDES = $(foreach dir,$(INC_PATH),-I$(dir))

CC = gcc
CXX = g++
AR = ar rcs

COMMON_FLAGS = -MMD -MP -Wall -Wextra -O2 -g $(INCLUDES) 
CFLAGS = $(COMMON_FLAGS)
CXXFLAGS = $(COMMON_FLAGS) -std=c++20

TARGET = $(BUILD_DIR)/libsdb.a
all: $(BUILD_DIR) $(TARGET)

src/disasm.c: $(LIBCAPSTONE)

$(LIBCAPSTONE):
	$(MAKE) -C tools/capstone

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(AR) $@ $^

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(BUILD_DIR)/*

.PHONY: all clean
