CC=clang

all: poligon triton

poligon: poligon.c
	$(CC) -Wall -Werror `sdl-config --cflags --libs` -lSDL_gfx $< -o $@

triton: triton.so
triton.so: triton.c
	$(CC) -Wall -Werror -fPIC -c triton.c
	$(CC) -shared triton.o -o triton.so

clean:
	rm -rf *.so
	rm -rf *.o

distclean: clean
	rm poligon
