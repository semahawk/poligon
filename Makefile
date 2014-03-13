CC=clang
CFLAGS := -Wall -g `sdl-config --cflags --libs`

.PHONY: lib meh clean distclean
all: lib poligon meh

poligon: poligon.c poligon.h
	$(CC) $(CFLAGS) -c poligon.c
	$(CC) $(CFLAGS) -lSGE poligon.o -Wl,-rpath -Wl,. -o $@

lib: poligon.a
poligon.a: poligon.c poligon.h
	$(CC) $(CFLAGS) -fPIC -c poligon.c
	ar rcs poligon.a poligon.o

meh: lib meh.so
meh.so: meh.c poligon.h
	$(CC) $(CFLAGS) -fPIC -c meh.c
	$(CC) -shared poligon.a meh.o -o $@

clean:
	rm -rf *.so
	rm -rf *.o
	rm -rf *.a

distclean: clean
	rm -f poligon

