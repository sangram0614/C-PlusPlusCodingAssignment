CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -fPIC -Isrc/lib
LDFLAGS = -pthread

SRC_DIR = src
BUILD_DIR = build

LIB_DIR = $(SRC_DIR)/lib
DAEMON_DIR = $(SRC_DIR)/daemon
CLI_DIR = $(SRC_DIR)/cli

LIB_OBJS = $(BUILD_DIR)/comms.o
LIB = $(BUILD_DIR)/libcomms.dylib

DAEMON = $(BUILD_DIR)/daemon
CLI = $(BUILD_DIR)/cli

all: $(DAEMON) $(CLI)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build comms.o
$(BUILD_DIR)/comms.o: $(LIB_DIR)/comms.cpp $(LIB_DIR)/comms.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build shared library (macOS .dylib)
$(LIB): $(BUILD_DIR)/comms.o
	$(CXX) -dynamiclib -o $@ $^

# Build daemon
$(DAEMON): $(DAEMON_DIR)/daemon.cpp $(LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(DAEMON_DIR)/daemon.cpp -L$(BUILD_DIR) -lcomms -Wl,-rpath,@loader_path $(LDFLAGS)

# Build CLI
$(CLI): $(CLI_DIR)/cli.cpp $(LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(CLI_DIR)/cli.cpp -L$(BUILD_DIR) -lcomms -Wl,-rpath,@loader_path $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

run: all
	./$(DAEMON)

.PHONY: all clean run

