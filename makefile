CXX = clang
CXXFLAGS = -std=c11
LDFLAGS =

LIB_DIR = lib
LIBS = ansiinput ansipixel raymath

SRC_DIR = src
MODULES = main terrain

TARGET_DIR = target
TARGET = main

# prerequisites for each module
# add the module even if there is no prerequisite
# full path from project root dir
main = lib/ansiinput/ansiinput.h lib/ansipixel/ansipixel.h lib/raymath/raymath.h
terrain = src/terrain.h lib/raymath/raymath.h

.PHONY: all
all: $(TARGET_DIR) $(TARGET_DIR)/$(TARGET)
	@echo > /dev/null

run: all
	@./$(TARGET_DIR)/$(TARGET) $(ARGS)

$(TARGET_DIR):
	@mkdir -p $@

MODULE_OBJ = $(addprefix $(TARGET_DIR)/, $(addsuffix .o, $(MODULES)))
LIB_OBJ = $(addprefix $(TARGET_DIR)/$(LIB_DIR)/lib, $(LIBS))

CXXFLAGS += $(addprefix -I, $(LIB_DIR))
LDFLAGS += $(addprefix -L, $(LIB_OBJ)) $(addprefix -l, $(LIBS))

$(TARGET_DIR)/$(TARGET): $(LIB_OBJ) $(MODULE_OBJ)
	@echo linking $@
	@$(CXX) $(LDFLAGS) $(CXXFLAGS) $(MODULE_OBJ) -o $@

.SECONDEXPANSION:

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.c $$($$*) makefile
	@echo compiling $@
	@$(CXX) $(CXXFLAGS) $< -c -o $@

CURRENT_DIR = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(TARGET_DIR)/$(LIB_DIR)/lib%: $$(shell find $(LIB_DIR)/% -type f)
	@echo compiling $@
	@mkdir -p $@
	@cd $(LIB_DIR)/$* && $(MAKE) TARGET_DIR=$(CURRENT_DIR)/$@

.PHONY: clean
clean:
	rm -rf $(TARGET_DIR)
