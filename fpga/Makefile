# ======================================================================
# FPGA-Accelerated OrderBook Makefile
# ======================================================================
#
# This Makefile is designed to build the FPGA kernel (`orderbook_kernel.cpp`)
# and the corresponding host application (`orderbook_test.cpp`) that interfaces
# with the FPGA using Xilinx Runtime (XRT).
#
# Usage:
#   make all TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform>
#   make run TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform>
#   make build TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform>
#   make clean
#   make cleanall
#
# Example:
#   make all TARGET=hw PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1
#
# ======================================================================

# ------------------------- Project Variables -------------------------

# FPGA Platform
PLATFORM := xilinx_u250_gen3x16_xdma_4_1_202210_1

# Target: sw_emu, hw_emu, or hw
TARGET ?= hw

# Kernel Frequency (Hz)
FREQ = 300000000

# Paths
SRC_DIR := .
BUILD_DIR := ./build_dir.$(TARGET).$(PLATFORM)
TEMP_DIR := ./_x.$(TARGET).$(PLATFORM)

# Kernel and Host Sources
KERNEL_SRCS := orderbook_kernel.cpp
HOST_SRCS := orderbook_test.cpp

# Executable Names
HOST_EXEC := orderbook_test_fpga

# XCLBIN Name
XCLBIN := $(BUILD_DIR)/orderbook_kernel.xclbin

# Include and Library Paths
XILINX_XRT := $(XILINX_XRT) # Ensure this environment variable is set
XILINX_VIVADO := $(XILINX_VIVADO) # Ensure this environment variable is set

# Compiler Flags
CXXFLAGS := -std=c++17 -O3 -march=native -flto -Wall -Wextra -pedantic -I. -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include -ggdb

# Linker Flags
LDFLAGS := -L$(XILINX_XRT)/lib -lxrt_coreutil -lxrt_core -lOpenCL -lpthread

# HLS Compiler Flags
VPP_FLAGS := --save-temps

# Phony Targets
.PHONY: all build run clean cleanall help

# ------------------------- Default Target ---------------------------

all: build $(HOST_EXEC) emconfig

# ------------------------- Build Targets ----------------------------

# Build FPGA Kernel
build: $(XCLBIN)

# Compile Kernel to XCLBIN
$(XCLBIN): $(TEMP_DIR)/orderbook_kernel.xo
	mkdir -p $(BUILD_DIR)
	v++ -l --clock.defaultFreqHz $(FREQ) $(VPP_FLAGS) -t $(TARGET) --platform $(PLATFORM) --temp_dir $(TEMP_DIR) -o $(XCLBIN) $<

# Compile Kernel to XO (Object) File
$(TEMP_DIR)/orderbook_kernel.xo: $(KERNEL_SRCS)
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) -t $(TARGET) --platform $(PLATFORM) -k process_order --temp_dir $(TEMP_DIR) -I'$(<D)' -o '$@' '$<'

# Build Host Executable
$(HOST_EXEC): $(HOST_SRCS)
	g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

# ------------------------- Emulation Configuration -----------------

# Generate emconfig.json for emulation
emconfig: $(TEMP_DIR)/emconfig.json
	cp $(TEMP_DIR)/emconfig.json .

$(TEMP_DIR)/emconfig.json:
	emconfigutil --platform $(PLATFORM) --od $(TEMP_DIR)

# ------------------------- Run Target -------------------------------

run: all
ifeq ($(TARGET),sw_emu)
	cp -rf $(TEMP_DIR)/emconfig.json .
	XCL_EMULATION_MODE=sw_emu ./$(HOST_EXEC) $(XCLBIN)
else ifeq ($(TARGET),hw_emu)
	cp -rf $(TEMP_DIR)/emconfig.json .
	XCL_EMULATION_MODE=hw_emu ./$(HOST_EXEC) $(XCLBIN)
else ifeq ($(TARGET),hw)
	./$(HOST_EXEC) $(XCLBIN)
else
	@echo "Unknown TARGET: $(TARGET). Use sw_emu, hw_emu, or hw."
endif

# ------------------------- Clean Targets ----------------------------

# Clean build artifacts
clean:
	-rm -f $(HOST_EXEC)
	-rm -rf $(TEMP_DIR)
	-rm -rf $(BUILD_DIR)

# Clean all including generated xclbin
cleanall: clean
	-rm -f *.xclbin
	-rm -rf _x* build_dir* package.* *.rpt *.csv *.log

# ------------------------- Help Target ------------------------------

help:
	@echo "Makefile Usage:"
	@echo "  make all TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform>"
	@echo "      Build the FPGA kernel and host application."
	@echo ""
	@echo "  make run TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform>"
	@echo "      Run the host application with the FPGA kernel."
	@echo ""
	@echo "  make clean"
	@echo "      Remove build artifacts."
	@echo ""
	@echo "  make cleanall"
	@echo "      Remove all generated files."
	@echo ""
	@echo "  make help"
	@echo "      Display this help message."

# ------------------------- End of Makefile --------------------------

