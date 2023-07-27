CXX = clang
CXXFLAGS = -std=c11
LDFLAGS =

MODULES = raymath
TARGET = raymath

ansiinput = raymath.h

OBJ = $(addprefix $(TARGET_DIR)/, $(addsuffix .o, $(MODULES)))

.PHONY: all
all: $(TARGET_DIR)/lib$(TARGET).a
	@echo > /dev/null

$(TARGET_DIR)/lib$(TARGET).a: $(OBJ)
	@echo linking $(TARGET_DIR)/lib$(TARGET)
	@ar -r $@ $^

.SECONDEXPANSION:

$(TARGET_DIR)/%.o: %.c $$($$*) makefile
	@echo compiling $@
	@$(CXX) $(CXXFLAGS) $< -c -o $@
