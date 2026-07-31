TARGET = $(BIN_DIR)/cpu_sim
TOP = top

CXX = g++
VCC = verilator
CXXFLAGS = -I$(BUILD_DIR) -isystem $(VERILATOR_ROOT)/include -isystem $(VERILATOR_ROOT)/include/vltstd -Iinclude -MMD -MP -D__GUEST_ISA__=riscv32e -Wall -g
CXXFLAGS += -I$(NEMU_HOME)/tools/capstone/repo/include

# macOS: Homebrew 的头文件/库不在编译器默认搜索路径里
ifeq ($(shell uname -s),Darwin)
  BREW         := $(shell brew --prefix)
  BREW_LDFLAGS := -L$(BREW)/lib
	BREW_LDFLAGS += -Wl,-U,__Z15vl_time_stamp64v,-U,__Z13sc_time_stampv,-U,_vlog_startup_routines
  VCC_EXTRA    := -CFLAGS -I$(BREW)/include
endif

LINKAGES = $(BUILD_DIR)/libV$(TOP).a $(BUILD_DIR)/libverilated.a $(BREW_LDFLAGS) -lz -lpthread -llz4 -lreadline

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
WAVE_FILE = $(BUILD_DIR)/wave.fst

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt
ELF ?= $(basename $(IMG)).elf
override ARGS += $(if $(IMG), -e $(ELF))
NPC_EXEC := $(TARGET) $(ARGS) --wave=$(WAVE_FILE) $(IMG) 

all: $(TARGET)

wave: 
	gtkwave $(BUILD_DIR)/$(WAVE_FILE)

run: $(TARGET)
	$(NPC_EXEC)

head: $(SV_LIB)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET): $(OBJS) $(SV_LIB)
	@mkdir -p $(dir $@)
	@echo + LD $@
	@$(CXX) $(OBJS) $(LINKAGES) -o $@

$(OBJS): $(SV_LIB)

$(OBJ_DIR)/%.o: $(CSRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo + CXX $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(SV_LIB): $(VSRCS)
	@echo + VCC npc
	@$(VCC) --quiet --cc --build --trace-fst -Wall -Wno-UNUSEDSIGNAL --Mdir $(BUILD_DIR) $(VSRCS) --top-module $(TOP) $(VCC_EXTRA)

-include $(DEPS)

.PHONY: all head run wave clean