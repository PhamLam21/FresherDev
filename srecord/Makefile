# Compiler and flags
CXX := gcc
CXXFLAGS := -Wall -Wextra -I./inc -pthread

# Directories
SRCDIR := ./src
INCDIR := ./inc
BINDIR := ./bin

# Source and object files
SRCFILES := $(wildcard $(SRCDIR)/*.c)
OBJFILES := $(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.o, $(SRCFILES))
TARGET := $(BINDIR)/main.exe

# Rules
all: $(TARGET)
	@echo "Build successful! Running the program..."

# Link object files to create the executable
$(TARGET): $(OBJFILES)
	@echo "Linking: $@"
	$(CXX) $(OBJFILES) -o $@

# Compile source files to object files
$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	@echo "Compiling: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR):
	@mkdir -p $(BINDIR)
	@touch $(BINDIR)/Output.txt
# Clean up build files
clean:
	@echo "Cleaning up..."
	rm -rf $(BINDIR)

.PHONY: all clean