all: simd mem_speed

simd: simd.cpp *.h
	g++ -O2 -std=c++11 -mavx2 simd.cpp -o simd
mem_speed: mem_speed.cpp
	g++ -O2 -std=c++11 -mavx2 -pthread mem_speed.cpp -o mem_speed

clean:
	rm -f simd mem_speed