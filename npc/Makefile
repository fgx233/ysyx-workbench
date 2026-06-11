TOP = top
BIN = sim

WAVE = wave.fst

BUILD_DIR = build
CSRCS = $(shell find ./csrc -name "*.cpp")
VSRCS = $(shell find ./vsrc -name "*.v")

VCC = verilator
CFLAGS = -I$(BUILD_DIR) -I$(VERILATOR_ROOT)/include
LINKAGES = $(BUILD_DIR)/libV$(TOP).a $(BUILD_DIR)/libverilated.a -lz -lpthread

VERILATOR_ROOT = $(shell $(VCC) --getenv VERILATOR_ROOT)

all: $(BUILD_DIR)/$(BIN)
	
lint:
	$(VCC) --lint-only -Wall $(VSRCS)

head: $(BUILD_DIR)/libV$(TOP).a

sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	./$(BUILD_DIR)/$(BIN)

wave: $(WAVE)
	gtkwave $(WAVE)

$(WAVE): $(BUILD_DIR)/$(BIN)
	./$(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN): $(CSRCS) $(BUILD_DIR)/libV$(TOP).a
	g++ $(CFLAGS) $(CSRCS) $(LINKAGES) -o $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/libV$(TOP).a: $(VSRCS)
	$(VCC) --cc --build --trace-fst -Wall --Mdir $(BUILD_DIR) $(VSRCS) --top-module $(TOP)

clean:
	rm -rf $(BUILD_DIR) $(WAVE)

.PHONY: all lint head run wave clean

include ../Makefile
