TOP      = Top
RTL_DIR  = rtl
OBJ_DIR  = obj_dir
TB_OBJ_DIR = obj_dir_tb

VERILATOR        = verilator
VERILATOR_ROOT  ?= $(shell $(VERILATOR) --getenv VERILATOR_ROOT)
VERILATOR_FLAGS    = --cc --Wall --trace-fst -Mdir $(OBJ_DIR)
VERILATOR_TB_FLAGS = --cc --Wall --trace-fst --timing -Mdir $(TB_OBJ_DIR)

SV_SOURCES = \
	$(RTL_DIR)/Pipeline.sv \
	$(RTL_DIR)/PipelineLoop.sv \
	$(RTL_DIR)/Plus1.sv \
	$(RTL_DIR)/ForEach.sv \
	$(RTL_DIR)/Repeat.sv \
	$(RTL_DIR)/Top.sv

TB_SV_SOURCES = $(SV_SOURCES) tb_top.sv

CXX      = g++
CXXFLAGS = -std=c++17 -O2 \
           -I$(VERILATOR_ROOT)/include \
           -I$(VERILATOR_ROOT)/include/vltstd

.PHONY: all sim_cpp sim_tb lint clean

all: sim_cpp

$(OBJ_DIR)/V$(TOP).mk: $(SV_SOURCES)
	$(VERILATOR) $(VERILATOR_FLAGS) --top-module $(TOP) $(SV_SOURCES)

$(OBJ_DIR)/V$(TOP)__ALL.a: $(OBJ_DIR)/V$(TOP).mk
	$(MAKE) -C $(OBJ_DIR) -f V$(TOP).mk

$(OBJ_DIR)/libverilated.a: $(OBJ_DIR)/V$(TOP)__ALL.a

sim_cpp: sim.cpp $(OBJ_DIR)/V$(TOP)__ALL.a $(OBJ_DIR)/libverilated.a
	$(CXX) $(CXXFLAGS) -I$(OBJ_DIR) sim.cpp \
		$(OBJ_DIR)/V$(TOP)__ALL.a \
		$(OBJ_DIR)/libverilated.a \
		-o $@ -lz

# --- Verilog testbench (tb_top.sv) targets ---

$(TB_OBJ_DIR)/Vtb_top.mk: $(TB_SV_SOURCES)
	$(VERILATOR) $(VERILATOR_TB_FLAGS) --top-module tb_top $(TB_SV_SOURCES)

$(TB_OBJ_DIR)/Vtb_top__ALL.a: $(TB_OBJ_DIR)/Vtb_top.mk
	$(MAKE) -C $(TB_OBJ_DIR) -f Vtb_top.mk

$(TB_OBJ_DIR)/libverilated.a: $(TB_OBJ_DIR)/Vtb_top__ALL.a

sim_tb: main_tb.cpp $(TB_OBJ_DIR)/Vtb_top__ALL.a $(TB_OBJ_DIR)/libverilated.a
	$(CXX) $(CXXFLAGS) -I$(TB_OBJ_DIR) main_tb.cpp \
		$(TB_OBJ_DIR)/Vtb_top__ALL.a \
		$(TB_OBJ_DIR)/libverilated.a \
		-o $@ -lz

lint: $(SV_SOURCES)
	$(VERILATOR) --lint-only --Wall --top-module $(TOP) $(SV_SOURCES)

clean:
	rm -rf $(OBJ_DIR) $(TB_OBJ_DIR) sim tb cpp.fst tb.fst
