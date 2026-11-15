.DEFAULT_GOAL = app

WORK_DIR  = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build

INC_PATH = $(WORK_DIR)/include $(MODEL_DIR)
OBJ_DIR  = $(BUILD_DIR)/obj-$(NAME)
BINARY   = $(BUILD_DIR)/$(NAME)

CXX := g++

LD := $(CXX)
INCLUDES = $(addprefix -I, $(INC_PATH))
INCLUDES += -isystem $(VERILATOR_PATH)
INCLUDES += -isystem $(VERILATOR_PATH)/vltstd

CFLAGS  += -O2 -MMD -Wall -Werror $(INCLUDES)
LDFLAGS := -O2 $(LDFLAGS)

OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: %.cpp
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<
	$(call call_fixdep, $(@:.o=.d), $@)

# Depencies
-include $(OBJS:.o=.d)

# Some convenient rules

.PHONY: app clean

app: $(BINARY)

include $(NPC_HOME)/scripts/verilating.mk

ARCHIVES += $(RTL_MODLE_ARCHIVES)
LIBS += -lreadline -llz4 -lz

$(BINARY):: $(ONE_OF_LIBS) $(OBJS) $(ARCHIVES) 
	@echo + LD $@
	@$(LD) -o $@ $(OBJS) $(LDFLAGS) $(ARCHIVES) $(LIBS)

clean:
	-rm -rf $(BUILD_DIR) $(VSRC_DIR)
