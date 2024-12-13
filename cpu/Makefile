# Makefile

# ======================================================================
# Compiler and Flags
# ======================================================================
CXX := g++
# Note: Both -O3 and -O0 are present. Choose one based on your requirements.
# -O3 for optimization, -O0 for debugging.
# For this example, we'll use -O3. Remove -O0 if not needed.
CXXFLAGS := -std=c++17 -O3 -Wall -Wextra -pedantic -Iinclude -ggdb

# ======================================================================
# Libraries
# ======================================================================
LIBS := -lpthread

# ======================================================================
# Directories
# ======================================================================
SRC_DIR := src
INC_DIR := include

# ======================================================================
# Executables
# ======================================================================
# Added 'orderbook_test' to the list of targets
TARGETS := server_main client_main order_generation orderbook_test

# ======================================================================
# Source Files
# ======================================================================
SRCS_SERVER_MAIN := $(SRC_DIR)/server_main.cpp $(SRC_DIR)/server.cpp $(SRC_DIR)/orderbook.cpp
SRCS_CLIENT_MAIN := $(SRC_DIR)/client_main.cpp $(SRC_DIR)/client.cpp $(SRC_DIR)/orderbook.cpp
SRCS_ORDER_GEN := $(SRC_DIR)/order_generation.cpp $(SRC_DIR)/orderbook.cpp
# Defined sources for orderbook_test, including utilities.cpp
SRCS_ORDERBOOK_TEST := $(SRC_DIR)/orderbook_test.cpp $(SRC_DIR)/orderbook.cpp $(SRC_DIR)/utilities.cpp

# ======================================================================
# Object Files
# ======================================================================
OBJS_SERVER_MAIN := server_main.o server.o orderbook.o
OBJS_CLIENT_MAIN := client_main.o client.o orderbook.o
OBJS_ORDER_GEN := order_generation.o orderbook.o
# Defined object files for orderbook_test
OBJS_ORDERBOOK_TEST := orderbook_test.o orderbook.o

# ======================================================================
# Default Target
# ======================================================================
all: $(TARGETS)

# ======================================================================
# Rules to Build Executables
# ======================================================================

# server_main executable
server_main: $(OBJS_SERVER_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# client_main executable
client_main: $(OBJS_CLIENT_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# order_generation executable
order_generation: $(OBJS_ORDER_GEN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# orderbook_test executable
orderbook_test: $(OBJS_ORDERBOOK_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# ======================================================================
# Pattern Rule to Compile .cpp to .o
# ======================================================================
%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ======================================================================
# Clean Up Build Artifacts
# ======================================================================
clean:
	rm -f *.o $(TARGETS)

# ======================================================================
# Phony Targets
# ======================================================================
.PHONY: all clean
