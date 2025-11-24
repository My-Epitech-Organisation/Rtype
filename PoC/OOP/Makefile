# Makefile for OOP PoC
# Traditional OOP approach for R-Type

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS := -lm

# Directories
BUILD_DIR := build
SRC_DIR := .

# Output
TARGET := $(BUILD_DIR)/oop_poc

# Source files
SOURCES := GameObject.cpp \
           Movable.cpp \
           Player.cpp \
           Enemy.cpp \
           DiamondProblem.cpp \
           main.cpp

# Object files
OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

# Default target
.PHONY: all
all: debug

# Debug build
.PHONY: debug
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)
	@echo ""
	@echo "═══════════════════════════════════════════════════════"
	@echo "✓ Debug build complete: $(TARGET)"
	@echo "═══════════════════════════════════════════════════════"

# Release build
.PHONY: release
release: CXXFLAGS += -O3 -DNDEBUG
release: $(TARGET)
	@echo ""
	@echo "═══════════════════════════════════════════════════════"
	@echo "✓ Release build complete: $(TARGET)"
	@echo "═══════════════════════════════════════════════════════"

# Link
$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Run
.PHONY: run
run: debug
	@echo ""
	@echo "═══════════════════════════════════════════════════════"
	@echo "Running OOP PoC..."
	@echo "═══════════════════════════════════════════════════════"
	@echo ""
	@$(TARGET)

# Clean
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@echo "✓ Clean complete"

# Help
.PHONY: help
help:
	@echo "OOP PoC Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build debug version (default)"
	@echo "  make debug    - Build debug version with symbols"
	@echo "  make release  - Build optimized release version"
	@echo "  make run      - Build and run debug version"
	@echo "  make clean    - Remove build files"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make && ./build/oop_poc"
	@echo "  make release && ./build/oop_poc"
	@echo "  make run"

# Dependencies
$(BUILD_DIR)/GameObject.o: GameObject.cpp GameObject.hpp
$(BUILD_DIR)/Movable.o: Movable.cpp Movable.hpp GameObject.hpp
$(BUILD_DIR)/Player.o: Player.cpp Player.hpp Movable.hpp GameObject.hpp
$(BUILD_DIR)/Enemy.o: Enemy.cpp Enemy.hpp Movable.hpp GameObject.hpp
$(BUILD_DIR)/DiamondProblem.o: DiamondProblem.cpp DiamondProblem.hpp GameObject.hpp
$(BUILD_DIR)/main.o: main.cpp GameObject.hpp Movable.hpp Player.hpp Enemy.hpp DiamondProblem.hpp

.PHONY: info
info:
	@echo "Build Information:"
	@echo "  CXX: $(CXX)"
	@echo "  CXXFLAGS: $(CXXFLAGS)"
	@echo "  Sources: $(SOURCES)"
	@echo "  Objects: $(OBJECTS)"
	@echo "  Target: $(TARGET)"
