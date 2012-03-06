CC = g++
CFLAGS = -g
EXTRA_CFLAGS = 

all: 
	g++ Cache.cpp -o sim_cache

clean:
	rm -f sim_cache

