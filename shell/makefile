# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic

# Executable name
TARGET = shell

# Source files
SRCS = auto_complete.cpp cd.cpp echo.cpp fg.cpp history.cpp ls.cpp main.cpp pinfo.cpp raw_mode.cpp  user_env.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Header files (optional, helps with dependency tracking)
HDRS = $(wildcard *.h)

# Default rule
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to build object files
%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
