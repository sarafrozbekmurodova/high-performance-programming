CC = gcc
CFLAGS = -O3 -Wall -Wextra -pedantic -g
INCLUDES=-I/opt/X11/include
LDLIBS=-L/opt/X11/lib -lX11 -lm
VECTOR_FLAGS = -march=native -ffast-math -ftree-vectorize -fopt-info-vec

galsim: galsim.c
	$(CC) $(CFLAGS) $(VECTOR_FLAGS) -o $@ $^ $(LDLIBS)

test_performance: galsim
	TIMEFMT='%uE real, %uU user, %uS sys'; time ./galsim 00010 ./input_data/ellipse_N_00010.gal 100 0.00001 0
	TIMEFMT='%uE real, %uU user, %uS sys'; time ./galsim 00100 ./input_data/ellipse_N_00100.gal 100 0.00001 0
	TIMEFMT='%uE real, %uU user, %uS sys'; time ./galsim 01000 ./input_data/ellipse_N_01000.gal 100 0.00001 0
	TIMEFMT='%uE real, %uU user, %uS sys'; time ./galsim 10000 ./input_data/ellipse_N_10000.gal 100 0.00001 0
clean:
	rm -f galsim results.gal
