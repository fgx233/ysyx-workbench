VSRC = vsrc
# npc模板自带目标
test:
	mill -i test

verilog:
	$(call git_commit, "generate verilog")
	mkdir -p $(VSRC)
	mill -i runMain Elaborate --target-dir $(VSRC)

help:
	mill -i runMain Elaborate --help

reformat:
	mill -i __.reformat

checkformat:
	mill -i __.checkFormat

bsp:
	mill -i mill.bsp.BSP/install

idea:
	mill -i mill.idea.GenIdea/idea

clean:
	-rm -rf $(VSRC)/*
	-rm -rf $(SYN_OUTPUT)/*
	$(MAKE) -C verification clean
	$(MAKE) -C nvboard clean

# verilator仿真
head:
	$(call git_commit, "get model")
	$(MAKE) -C verification head

sim:
	$(call git_commit, "verify verilog")
	$(MAKE) -C verification sim

wave:
	$(call git_commit, "watch wave")
	$(MAKE) -C verification wave

# nvboard上板
link:
	$(call git_commit, "link pins")
	$(MAKE) -C nvboard link

board:
	$(call git_commit, "on board")
	$(MAKE) -C nvboard run

# yosys综合+时序分析
YOSYS_PATH = /home/fgx/projects/Verilog/yosys-sta
SYN_OUTPUT = syn_result
SYN_VARS = O=$(abspath $(SYN_OUTPUT)) \
           DESIGN=top \
           RTL_FILES="$(shell find $(abspath ./vsrc) -name '*.sv')" \
           CLK_FREQ_MHZ=50 \
           CLK_PORT_NAME=clock

syn:
	$(call git_commit, "make syn")
	$(MAKE) -C $(YOSYS_PATH) $(SYN_VARS) syn

sta:
	$(call git_commit, "make sta")
	$(MAKE) -C $(YOSYS_PATH) $(SYN_VARS) sta

.PHONY: test verilog help reformat checkformat clean head sim wave board link syn sta
-include ../Makefile
