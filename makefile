# Declaration of variables
CC = g++
CC_FLAGS = -Wall -std=c++0x

# File names
EXEC = ./augeasDemo
#SOURCES = $(wildcard src/*.cpp)
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
LIBS = -lmosquitto -lwiringPi -laugeas -lboost_system -lboost_filesystem
INC=-I./../include -I/usr/include/libxml2


# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(EXEC) -pthread 

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $(INC) $< -o $@

# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)
