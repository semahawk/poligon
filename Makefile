CC=clang

all: poligon meh

poligon: poligon.c poligon.h
	$(CC) -Wall `sdl-config --cflags --libs` -lSGE $< -o $@

meh: meh.so
meh.so: meh.c poligon.h
	$(CC) -Wall `sdl-config --cflags` -fPIC -c meh.c
	$(CC) -shared meh.o -o meh.so

clean:
	rm -rf *.so
	rm -rf *.o

distclean: clean
	rm poligon
