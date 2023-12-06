# Variables
CC = g++
CFLAGS = -Iinclude
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
LIB = libmylib.a
TEST = compiler

# Default rule
all: $(LIB) $(TEST)

# Rule to compile all cpp files to o files
%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

# Rule to link all o files to a library
$(LIB): $(OBJS)
	ar rvs $@ $^

# Rule to compile and link the test program
$(TEST): program/compiler.o $(LIB)
	$(CC) -o $@ $^

# Rule to clean all generated files
clean:
	rm -f $(OBJS) $(LIB) $(TEST)