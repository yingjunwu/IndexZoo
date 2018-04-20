CC=g++
CFLAGS=-O2 -std=c++11
TARGET=$(patsubst %.cpp, %, $(wildcard *.cpp))

all: $(TARGET)

$(TARGET): % : %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)