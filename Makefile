CC=clang

.PHONY: poligon meh clean distclean
all: poligon meh

poligon: poligon.c poligon.h
	$(CC) -Wall `sdl-config --cflags --libs` -lSGE $< -o $@

poligon.so: thelib.c poligon.h
	$(CC) -Wall `sdl-config --cflags` -fPIC -c thelib.c
	$(CC) -shared thelib.o -o $@

meh: meh.so poligon.so
meh.so: meh.c poligon.h
	$(CC) -Wall `sdl-config --cflags` -fPIC -lpoligon -c meh.c
	$(CC) -shared meh.o -o $@

clean:
	rm -rf *.so
	rm -rf *.o

distclean: clean
	rm poligon
