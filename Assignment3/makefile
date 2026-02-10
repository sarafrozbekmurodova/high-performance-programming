CC = gcc
CFLAGS = -O3 -Wall -Wextra -pedantic -g
INCLUDES=-I/opt/X11/include
LDLIBS=-L/opt/X11/lib -lX11 -lm
VECTOR_FLAGS = -march=native -ffast-math -ftree-vectorize -fopt-info-vec

galsim: galsim.c
	$(CC) $(CFLAGS) $(VECTOR_FLAGS) -o $@ $^ $(LDLIBS)

test_performance: galsim
	time ./galsim 10000 ./input_data/ellipse_N_10000.gal 200 0

clean:
	rm -f galsim results.gal
