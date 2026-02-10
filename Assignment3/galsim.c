#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graphics/graphics.h"

#define min(a, b) (a) < (b) ? (a) : (b)
#define max(a, b) (a) > (b) ? (a) : (b)

struct Particle {
    double x_pos;
    double y_pos;
    double mass;
    double x_velocity;
    double y_velocity;
    double brightness;
};

struct ParticleChange {
    double x_velocity;
    double y_velocity;
};

struct Particle *particles;
struct ParticleChange *temp_particles;

int n;
char *filename;
int nsteps;
double delta_time;
bool graphics;

double largest_particle = 0;
double brightest = 0;

const double epsilon = 0.001;

void readArguments(char *argv[]) {
    n = atoi(argv[1]);
    filename = argv[2];
    nsteps = atoi(argv[3]);
    delta_time = atof(argv[4]);
    graphics = atoi(argv[5]);
}

void readFile() {
    particles = malloc(sizeof(struct Particle) * n);
    temp_particles = malloc(sizeof(struct ParticleChange) * n);

    FILE *file = fopen(filename, "r");

    for (int i = 0; i < n; i++) {
        fread(&particles[i], sizeof(struct Particle), 1, file);
        if (particles[i].mass > largest_particle) {
            largest_particle = particles[i].mass;
        }
        if (particles[i].brightness > brightest) {
            brightest = particles[i].brightness;
        }
    }
}

void writeFile() {
    FILE *file = fopen("results.gal", "w");

    fwrite(particles, sizeof(struct Particle), n, file);
}

const double framerate = 1.0 / 60.0;
struct timespec lastFrame;

void drawGalaxy() {
    double time_spent;
    do {
        struct timespec currentTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        time_spent = (currentTime.tv_sec - lastFrame.tv_sec) +
                     (currentTime.tv_nsec - lastFrame.tv_nsec) / 1000000000.0;
    } while (time_spent < framerate);

    ClearScreen();
    for (int i = 0; i < n; i++) {
        double x = particles[i].x_pos;
        double y = particles[i].y_pos;
        printf("%lf, %lf\n", x, y);
        double r = max(0.002, 0.1 / n * particles[i].mass / largest_particle);
        double color = 1.0 - particles[i].brightness / brightest;
        DrawCircle(x * 1, y * 1, 1, 1, r, color);
    }
    Refresh();
    clock_gettime(CLOCK_MONOTONIC, &lastFrame);
}

void step() {
    const double G = 100.0 / n;
    for (int i = 0; i < n; i++) {
        temp_particles[i].x_velocity = 0;
        temp_particles[i].y_velocity = 0;
    }

    for (int i = 0; i < n; i++) {
        double mass_i = particles[i].mass;

        for (int j = i + 1; j < n; j++) {
            double mass_j = particles[j].mass;

            double dx = particles[i].x_pos - particles[j].x_pos;
            double dy = particles[i].y_pos - particles[j].y_pos;

            double distance = sqrt(dx * dx + dy * dy);

            double force_multiplier = G / pow(distance + epsilon, 3);

            float force_x = force_multiplier * dx;
            float force_y = force_multiplier * dy;

            double accel_i_x = -force_x * mass_j;
            double accel_i_y = -force_y * mass_j;

            double accel_j_x = force_x * mass_i;
            double accel_j_y = force_y * mass_i;

            temp_particles[i].x_velocity += delta_time * accel_i_x;
            temp_particles[i].y_velocity += delta_time * accel_i_y;

            temp_particles[j].x_velocity += delta_time * accel_j_x;
            temp_particles[j].y_velocity += delta_time * accel_j_y;
        }
    }

    for (int i = 0; i < n; i++) {
        particles[i].x_velocity += temp_particles[i].x_velocity;
        particles[i].y_velocity += temp_particles[i].y_velocity;

        particles[i].x_pos += particles[i].x_velocity * delta_time;
        particles[i].y_pos += particles[i].y_velocity * delta_time;
    }

    if (graphics) {
        drawGalaxy();
    }
}

int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Usage: ./galsim N filename nsteps delta_time graphics");
        return 1;
    }
    readArguments(argv);
    readFile();

    if (graphics) {
        InitializeGraphics(argv[0], 1600, 1600);
    }

    for (int i = 0; i < nsteps; i++) {
        step();
    }

    writeFile();
}
