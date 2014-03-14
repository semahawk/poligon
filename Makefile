CC=clang
CFLAGS := -Wall -g

.PHONY: lib meh
all: lib poligon meh

# poligon
poligon.o: poligon.c
	clang `sdl-config --cflags` -fPIC -c poligon.c

poligon: poligon.o
	clang poligon.o -o poligon -Wl,-rpath=. `sdl-config --cflags --libs` -lSGE

# libpoligon
lib: libpoligon.so.0
libpoligon.so.0: poligon.o
	clang -shared -Wl,-soname,libpoligon.so.0 -o libpoligon.so.0.1 poligon.o
	ln -fs libpoligon.so.0.1 libpoligon.so.0
	ln -fs libpoligon.so.0.1 libpoligon.so

# meh
meh: meh.so
meh.so: meh.o
	clang -shared -Wl,-rpath=. -Wl,-soname,meh.so -o libmeh.so.0.1 meh.o -L. -lpoligon
	ln -fs libmeh.so.0.1 meh.so
meh.o: meh.c poligon.h
	clang `sdl-config --cflags` -fPIC -c meh.c

clean:
	rm -rf *.so*
	rm -rf *.o
	rm -rf *.a

distclean: clean
	rm -f poligon

