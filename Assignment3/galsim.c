#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct Particle {
    double x_pos;
    double y_pos;
    double mass;
    double x_velocity;
    double y_velocity;
    double brightness;
};

struct Particle *particles;
struct Particle *temp_particles;

int n;
char *filename;
int nsteps;
double delta_time;
bool graphics;

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
	temp_particles = malloc(sizeof(struct Particle) * n);

    FILE *file = fopen(filename, "r");

    for (int i = 0; i < n; i++) {
        fread(&particles[i], sizeof(struct Particle), 1, file);
    }
	memcpy(temp_particles, particles, sizeof(struct Particle) * n);
}

void write_file() {
	FILE *file = fopen("results.gal", "w");

	fwrite(particles, sizeof(struct Particle), n, file);
}

void step() {
	const double G = 100.0 / n; 
	for (int i = 0; i < n; i++)
	{
		double force_x = 0, force_y = 0;
		for (int j = 0; j < n; j++)
		{
			if (j == i) {
				continue;
			}
			double dx = particles[i].x_pos - particles[j].x_pos;
			double dy = particles[i].y_pos - particles[j].y_pos;

			double distance = sqrt(dx * dx + dy * dy);

			force_x += (particles[j].mass / pow(distance + epsilon, 3)) * dx;
			force_y += (particles[j].mass / pow(distance + epsilon, 3)) * dy; 
		}
		force_x *= -G * particles[i].mass;
		force_y *= -G * particles[i].mass;

		double accel_x = force_x / particles[i].mass;
		double accel_y = force_y / particles[i].mass;
		
		double x_velocity = particles[i].x_velocity + delta_time * accel_x;
		double y_velocity = particles[i].y_velocity + delta_time * accel_y;

		double x_pos = particles[i].x_pos + delta_time * x_velocity;
		double y_pos = particles[i].y_pos + delta_time * y_velocity;

		temp_particles[i].x_pos = x_pos;
		temp_particles[i].y_pos = y_pos;
		temp_particles[i].x_velocity = x_velocity;
		temp_particles[i].y_velocity = y_velocity;
	}
	struct Particle *temp;
	temp = particles;
	particles = temp_particles;
	temp_particles = temp;
}

int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Usage: ./galsim N filename nsteps delta_time graphics");
        return 1;
    }
    readArguments(argv);
    readFile();

	for (int i = 0; i < nsteps; i++)
	{
		step();
	}
	
	write_file();
}
