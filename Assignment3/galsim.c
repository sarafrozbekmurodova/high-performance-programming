#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUMCOLORS 512

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

void read_arguments(char *argv[]) {
    n = atoi(argv[1]);
    filename = argv[2];
    nsteps = atoi(argv[3]);
    delta_time = atof(argv[4]);
    graphics = atoi(argv[5]);
}

void read_file() {
    particles = malloc(sizeof(struct Particle) * n);
    temp_particles = malloc(sizeof(struct ParticleChange) * n);

    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        size_t bytes_read =
            fread(&particles[i], sizeof(struct Particle), 1, file);
        if (!bytes_read) {
            fprintf(stderr, "Error reading file\n");
            exit(1);
        }
        if (particles[i].mass > largest_particle) {
            largest_particle = particles[i].mass;
        }
        if (particles[i].brightness > brightest) {
            brightest = particles[i].brightness;
        }
    }
    fclose(file);
}

void write_file() {
    FILE *file = fopen("results.gal", "w");
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    fwrite(particles, sizeof(struct Particle), n, file);
    fclose(file);
}
const double frame_rate = 1.0 / 60.0;
struct timespec last_frame;

void step() {
    // Reset the temp_particles
    memset(temp_particles, 0, sizeof(struct ParticleChange) * n);

    const double G = 100.0 / n;

    for (int i = 0; i < n; i++) {
        double mass_i = particles[i].mass;

        // Using Newtons third law, we can save about 50% of all iterations
        for (int j = i + 1; j < n; j++) {
            double mass_j = particles[j].mass;

            double dx = particles[i].x_pos - particles[j].x_pos;
            double dy = particles[i].y_pos - particles[j].y_pos;

            double distance = sqrt(dx * dx + dy * dy);

            double force_multiplier = G / pow(distance + epsilon, 3);

            double force_x = force_multiplier * dx;
            double force_y = force_multiplier * dy;

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

    // Update all velocities and positions in one go
    for (int i = 0; i < n; i++) {
        particles[i].x_velocity += temp_particles[i].x_velocity;
        particles[i].y_velocity += temp_particles[i].y_velocity;

        particles[i].x_pos += particles[i].x_velocity * delta_time;
        particles[i].y_pos += particles[i].y_velocity * delta_time;
    }
}

int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Usage: ./galsim N filename nsteps delta_time graphics");
        return 1;
    }
    read_arguments(argv);
    read_file();

    for (int i = 0; i < nsteps; i++) {
        step();
    }

    write_file();

    free(particles);
    free(temp_particles);
}
