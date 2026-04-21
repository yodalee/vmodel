TOP      = Top
RTL_DIR  = rtl
OBJ_DIR  = obj_dir

VERILATOR        = verilator
VERILATOR_FLAGS  = --cc --Wall -Mdir $(OBJ_DIR)

SV_SOURCES = \
	$(RTL_DIR)/Pipeline.sv \
	$(RTL_DIR)/PipelineLoop.sv \
	$(RTL_DIR)/Plus1.sv \
	$(RTL_DIR)/ForEach.sv \
	$(RTL_DIR)/Repeat.sv \
	$(RTL_DIR)/Top.sv

.PHONY: all lint clean

all: $(OBJ_DIR)/V$(TOP).mk

$(OBJ_DIR)/V$(TOP).mk: $(SV_SOURCES)
	$(VERILATOR) $(VERILATOR_FLAGS) --top-module $(TOP) $(SV_SOURCES)

lint: $(SV_SOURCES)
	$(VERILATOR) --lint-only --Wall --top-module $(TOP) $(SV_SOURCES)

clean:
	rm -rf $(OBJ_DIR)
