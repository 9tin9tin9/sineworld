CXX = clang
CXXFLAGS = -std=c11 -g
LDFLAGS =

LIB_DIR = lib
LIBS = ansiinput ansipixel raymath flecs

SRC_DIR = src
MODULES = main terrain input graphics movable player sprite ui

TARGET_DIR = target
TARGET = main

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

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.c $$(shell ./extract_dep.sh %) makefile
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
