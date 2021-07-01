CC=g++
CFLAGS=-lboost_program_options -pthread -Wall -s -Ofast -fopenmp
TARGET=rustkill

$(TARGET): main.cpp
	$(CC) main.cpp $(CFLAGS) -o $@

install:
	cp $(TARGET) /usr/local/bin/

clean:
	$(RM) $(TARGET)
