#for the core dumped.



# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

# Directories
INCDIR = include
SRCDIR = src
OBJDIR = obj

# Project name
TARGET = abx_client

# Source files
SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/ABXClient.cpp

# Object files
OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Include directories
INCLUDES = -I$(INCDIR)

# Targets
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up object and binary files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run

