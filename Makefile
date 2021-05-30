CC=g++
CFLAGS=-lboost_program_options -Wall -s -Ofast -fopenmp
TARGET=rustkill

$(TARGET): main.cpp
	$(CC) main.cpp $(CFLAGS) -o $(TARGET)

install:
	cp $(TARGET) /usr/local/bin/

clean:
	$(RM) $(TARGET)
