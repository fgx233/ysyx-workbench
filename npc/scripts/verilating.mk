TOPNAME = top
VERILATOR_PATH = /home/fgx/work/oss-cad-suite-linux-x64-20261111/oss-cad-suite/share/verilator/include
VERILATOR_FLAGS = --cc --build -O3 --x-assign fast --x-initial fast --noassert --trace-fst


RTLS = $(shell find elaborate/src -name "*.scala")
VSRC_DIR = vsrc
VSRCS = $(shell find $(VSRC_DIR) -name "*.sv")
FILELIST = $(VSRC_DIR)/filelist.f

$(FILELIST): $(RTLS)
	$(call git_commit, "generate verilog")
	rm -rf $(VSRC_DIR)
	mkdir -p $(VSRC_DIR)
	./mill -i elaborate.run --target-dir $(VSRC_DIR)

verilog: $(FILELIST)

MODEL_DIR = $(BUILD_DIR)/obj_dir
RTL_MODLE_ARCHIVES = $(shell find $(MODEL_DIR) -name "*.a")
ONE_OF_LIBS = build/obj_dir/libverilated.a
$(ONE_OF_LIBS): $(FILELIST)
	rm -rf $(MODEL_DIR)
	mkdir -p $(MODEL_DIR)
	verilator $(VERILATOR_FLAGS) $(VSRCS) --top-module $(TOPNAME) --Mdir $(MODEL_DIR)

model: $(ONE_OF_LIBS)

help-chisel:
	./mill -i elaborate.run --help

reformat:
	./mill -i __.reformat

checkformat:
	./mill -i __.checkFormat

bsp:
	./mill -i mill.bsp.BSP/install