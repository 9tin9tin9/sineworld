CXX = clang
CXXFLAGS = -std=gnu99 -DFLECS_NO_CPP
LDFLAGS =

MODULES = flecs
TARGET = flecs

ansiinput = flecs.h

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
