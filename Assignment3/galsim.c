#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct Particle {
    double x_pos;
    double y_pos;
    double mass;
    double x_velocity;
    double y_velocity;
    double brightness;
};

struct Particle *particles;

int n;
char *filename;
int nsteps;
double delta_time;
bool graphics;

void readArguments(char *argv[]) {
    n = atoi(argv[1]);
    filename = argv[2];
    nsteps = atoi(argv[3]);
    delta_time = atof(argv[4]);
    graphics = atoi(argv[5]);
}

void readFile() {
    particles = malloc(sizeof(struct Particle) * n);

    FILE *file = fopen(filename, "r");

    for (int i = 0; i < n; i++) {
        fread(&particles[i], sizeof(struct Particle), 1, file);
    }
}

int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Usage: ./galsim N filename nsteps delta_time graphics");
        return 1;
    }
    readArguments(argv);
    readFile();

    printf("%lf\n", particles[1].brightness);
}
