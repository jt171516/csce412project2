# Compiler settings - Can be customized.
CC = g++
CXXFLAGS = -Wall -g

# Project files
# List all your .cpp files here (Request.h is a header only, so it doesn't need a .o)
OBJS = main.o loadBalancer.o webServer.o

# The name of the executable file
TARGET = lb_sim

# Rule to link the program
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile main.cpp
main.o: main.cpp loadBalancer.h request.h
	$(CC) $(CXXFLAGS) -c main.cpp

# Rule to compile LoadBalancer.cpp
loadBalancer.o: loadBalancer.cpp loadBalancer.h webServer.h request.h
	$(CC) $(CXXFLAGS) -c loadBalancer.cpp

# Rule to compile WebServer.cpp
webServer.o: webServer.cpp webServer.h request.h
	$(CC) $(CXXFLAGS) -c webServer.cpp

# Rule to clean up files (removes .o files and the executable)
# Run 'make clean' to use this
clean:
	rm -f $(OBJS) $(TARGET)