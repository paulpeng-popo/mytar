CC = g++
FLAGS = -Wall -std=c++17
HEADER = tar
TARGET = mytar

all: dep
	$(CC) $(FLAGS) -o $(TARGET) $(TARGET).o $(HEADER).o

dep:
	$(CC) $(FLAGS) -c $(HEADER).cpp
	$(CC) $(FLAGS) -c $(TARGET).cpp

clean:
	rm -f *.o $(TARGET)
