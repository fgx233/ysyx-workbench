ifeq ($(shell uname -s), Darwin)
CROSS_COMPILE := riscv64-elf-
else
CROSS_COMPILE := riscv64-linux-gnu-
endif
COMMON_CFLAGS := -fno-pic -march=rv64g -mcmodel=medany -mstrict-align -ffreestanding
CFLAGS        += $(COMMON_CFLAGS) -static
ASFLAGS       += $(COMMON_CFLAGS) -O0
LDFLAGS       += -melf64lriscv

# overwrite ARCH_H defined in $(AM_HOME)/Makefile
ARCH_H := arch/riscv.h
