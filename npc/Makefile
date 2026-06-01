NPC_HOME  := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
YSYX_HOME := $(abspath $(NPC_HOME)/..)
NEMU_HOME ?= $(YSYX_HOME)/nemu
SONPC_HOME := $(NPC_HOME)

.DEFAULT_GOAL := sim

ifeq ($(ENABLE_GIT_COMMIT),1)
-include $(YSYX_HOME)/Makefile
endif
include $(NPC_HOME)/scripts/config.mk

-include $(NPC_HOME)/include/config/auto.conf
-include $(NPC_HOME)/include/config/auto.conf.cmd

TOP_MODULE ?= ysyx_core_npc_top
VERILATOR  ?= verilator

OBJ_DIR    := $(NPC_HOME)/obj_dir/$(TOP_MODULE)
NPC_BIN    := $(OBJ_DIR)/V$(TOP_MODULE)
NPC_LOG    := $(NPC_HOME)/npc-log.txt
NEMU_SO    := $(NEMU_HOME)/build/riscv64-nemu-interpreter-so
NPC_DIFF_SO ?= $(NEMU_SO)

CXX_SRCS_ALL := $(shell find $(NPC_HOME)/csrc -type f \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \) | sort)
# The local LLVM headers are not guaranteed to exist. Keep the interface alive
# with a tiny stub; real instruction tracing can be restored by compiling disasm.cc.
CXX_SRCS := $(filter-out $(NPC_HOME)/csrc/utils/disasm.cc $(NPC_HOME)/csrc/utils/disasm.c,$(CXX_SRCS_ALL))

define npc_git_commit
$(if $(ENABLE_GIT_COMMIT),$(call git_commit,$(1)),@true)
endef

CORE_VSRCS := $(wildcard $(NPC_HOME)/vsrc/core/*.v) \
              $(wildcard $(NPC_HOME)/vsrc/core/utils/*.v)

SIM_DPI_VSRCS := $(NPC_HOME)/vsrc/sim/sim_sram_dpic.v \
                 $(NPC_HOME)/vsrc/sim/rv64_mem_arb.v \
                 $(NPC_HOME)/vsrc/sim/DifftestArchIntRegState.v \
                 $(NPC_HOME)/vsrc/sim/DifftestCSRState.v \
                 $(NPC_HOME)/vsrc/sim/DifftestInstrCommit.v \
                 $(NPC_HOME)/vsrc/sim/DifftestPerformRegState.v \
                 $(NPC_HOME)/vsrc/sim/DifftestTrapEvent.v

ifeq ($(TOP_MODULE),ysyx_core_npc_top)
VSRCS := $(CORE_VSRCS) \
         $(NPC_HOME)/vsrc/ysyx_core/ysyx_23060081.v \
         $(SIM_DPI_VSRCS) \
         $(NPC_HOME)/vsrc/sim/ysyx_core_npc_top.v
VINC  := -I$(NPC_HOME)/vsrc/core \
         -I$(NPC_HOME)/vsrc/core/utils \
         -I$(NPC_HOME)/vsrc/ysyx_core \
         -I$(NPC_HOME)/vsrc/sim
else
$(error Unsupported TOP_MODULE=$(TOP_MODULE), current Makefile builds ysyx_core_npc_top)
endif

TRACE_FLAGS := --trace-fst

VERILATOR_CFLAGS := -I$(NPC_HOME)/include \
                    -I$(NPC_HOME)/csrc \
                    -I$(NPC_HOME)/csrc/sdb \
                    -I$(NPC_HOME)/csrc/remote_bitbang \
                    -I$(NPC_HOME)/csrc/lightssss \
                    -O2 \
                    -DVTOP=V$(TOP_MODULE) \
                    -DVTOP_H=\\\"V$(TOP_MODULE).h\\\" \
                    -DTOPNAME=\\\"$(TOP_MODULE)\\\" \
                    -DPC_RST=0x80000000ULL \
                    -D__GUEST_ISA__=riscv64

VERILATOR_FLAGS := -Wno-fatal \
                   -Wall \
                   --cc \
                   $(TRACE_FLAGS) \
                   --exe --build \
                   --top-module $(TOP_MODULE) \
                   --Mdir $(OBJ_DIR) \
                   --timescale "1ns/1ns" \
                   --no-timing \
                   -CFLAGS "$(VERILATOR_CFLAGS)" \
                   -LDFLAGS "-lreadline -ldl -pthread" \
                   $(VINC)

NPC_IMG ?=
NPC_FLAGS ?= --log=$(NPC_LOG) -b
RUN_DEPS :=

ifeq ($(CONFIG_DIFFTEST),y)
RUN_DEPS += nemu-so
override NPC_FLAGS += --diff=$(NPC_DIFF_SO)
endif

.PHONY: sim run gdb clean nemu-so

sim:
	mkdir -p $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_FLAGS) $(CXX_SRCS) $(VSRCS)
	$(call npc_git_commit, "sim RTL")

run: sim $(RUN_DEPS)
	$(NPC_BIN) $(NPC_FLAGS) $(NPC_IMG)

gdb: sim $(RUN_DEPS)
	gdb --args $(NPC_BIN) $(NPC_FLAGS) $(NPC_IMG)

nemu-so:
	$(MAKE) -C $(NEMU_HOME) SHARE=1

clean:
	rm -rf $(NPC_HOME)/obj_dir $(NPC_HOME)/npc-log.txt
