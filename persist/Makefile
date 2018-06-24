CC=g++
CFLAGS=-O2 -std=c++11 -mavx -msse2
TARGET=$(patsubst %.cpp, %, $(wildcard *.cpp))

all: $(TARGET)

$(TARGET): % : %.cpp *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)