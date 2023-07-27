CXX = clang
CXXFLAGS = -std=c11
LDFLAGS =

MODULES = ansipixel printf
TARGET = ansipixel

ansipixel = ansipixel.h printf.h
printf = printf.h

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
