# Compiler settings - Can be customized.
CC = g++
CXXFLAGS = -Wall -g

# Project files
# List all your .cpp files here (Request.h is a header only, so it doesn't need a .o)
OBJS = main.o LoadBalancer.o WebServer.o

# The name of the executable file
TARGET = lb_sim

# Rule to link the program
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile main.cpp
main.o: main.cpp LoadBalancer.h Request.h
	$(CC) $(CXXFLAGS) -c main.cpp

# Rule to compile LoadBalancer.cpp
LoadBalancer.o: LoadBalancer.cpp LoadBalancer.h WebServer.h Request.h
	$(CC) $(CXXFLAGS) -c LoadBalancer.cpp

# Rule to compile WebServer.cpp
WebServer.o: WebServer.cpp WebServer.h Request.h
	$(CC) $(CXXFLAGS) -c WebServer.cpp

# Rule to clean up files (removes .o files and the executable)
# Run 'make clean' to use this
clean:
	rm -f $(OBJS) $(TARGET)