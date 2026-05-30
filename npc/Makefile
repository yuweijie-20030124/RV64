NPC_HOME  := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
NEMU_HOME := $(abspath $(NPC_HOME)/../nemu)

include $(shell verilator --getenv VERILATOR_ROOT)/include/verilated.mk

# Include rules for menuconfig
include $(NPC_HOME)/csrc/configs/config.mk

# check NPC_HOME & NEMU_HOME
ifeq ($(wildcard $(NPC_HOME)/csrc/main.c),)
  $(error NPC_HOME=$(NPC_HOME) is not a NPC repo)
endif

-include $(NPC_HOME)/include/config/auto.conf
-include $(NPC_HOME)/include/config/auto.conf.cmd

# SRC_FILES = $(abspath $(wildcard $(SONPC_HOME)/csrc/*.c))

TOP_MODULE := top
VERILATOR_MDIR := obj_dir/$(TOP_MODULE)
TOP_BINARY := ./$(VERILATOR_MDIR)/V$(TOP_MODULE)
TOP_SCOPE  := TOP.$(TOP_MODULE)

CORE_VERILOG_FILES = $(wildcard vsrc/core/*.v vsrc/core/*.sv)
