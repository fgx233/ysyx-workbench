TARGET = $(BIN_DIR)/cpu_sim
TOP = top

CXX = g++
VCC = verilator
CXXFLAGS = -I$(BUILD_DIR) -I$(VERILATOR_ROOT)/include -Iinclude -MMD -MP
LINKAGES = $(BUILD_DIR)/libV$(TOP).a $(BUILD_DIR)/libverilated.a -lz -lpthread -llz4

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
VERILATOR_ROOT = $(shell $(VCC) --getenv VERILATOR_ROOT)

VSRC_DIR = ../vsrc
CSRC_DIR = ./src

CSRCS := $(shell find $(CSRC_DIR) -name '*.cpp')
VSRCS := $(shell find $(abspath $(VSRC_DIR)) -name '*.sv')

OBJS := $(patsubst $(CSRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CSRCS))
SV_LIB := $(BUILD_DIR)/libV$(TOP).a
DEPS := $(OBJS:.o=.d)
WAVE_FILE = wave.fst

all: $(TARGET)

wave: 
	gtkwave $(BUILD_DIR)/$(WAVE_FILE)

sim: $(TARGET)
	./$(TARGET)

head: $(SV_LIB)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET): $(OBJS) $(SV_LIB)
	mkdir -p $(dir $@)
	$(CXX) $(OBJS) $(LINKAGES) -o $@

$(OBJS): $(SV_LIB)

$(OBJ_DIR)/%.o: $(CSRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SV_LIB): $(VSRCS)
	$(VCC) --cc --build --trace-fst -Wall -Wno-UNUSEDSIGNAL --Mdir $(BUILD_DIR) $(VSRCS) --top-module $(TOP)

-include $(DEPS)

.PHONY: all head sim wave clean