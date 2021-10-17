TARGET = ./npshell
OBJ = bin/noop bin/number bin/removetag bin/removetag0
CPP = g++
SRC = $(OBJ:=.cpp)

all: $(TARGET)

$(TARGET): npshell.cpp
	$(CPP) $< -o $@

ex: $(OBJ)

$(OBJ): % : %.cpp
	$(CPP) $< -o $@

test: npshell.cpp
	$(CPP) $< -o $(TARGET)
	$(TARGET)
		

reset:
	rm $(OBJ)
clean:
	rm $(TARGET)
	
