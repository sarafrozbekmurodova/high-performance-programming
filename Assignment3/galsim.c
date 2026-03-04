#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUMCOLORS 512

Display *global_display_ptr;

Window win;
Pixmap pixmap;
XEvent report;
GC gc;
unsigned black, white;
unsigned width, height;
unsigned colors[NUMCOLORS];
float caxis[2];

#define min(a, b) (a) < (b) ? (a) : (b)
#define max(a, b) (a) > (b) ? (a) : (b)

struct Particle {
    double *x_pos;
    double *y_pos;
    double *mass;
    double *x_velocity;
    double *y_velocity;
    double *brightness;
};

struct Particle *particles;

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

void read_bytes(double *var, FILE *file) {
    size_t bytes_read = fread(var, sizeof(double), 1, file);
    if (!bytes_read) {
        fprintf(stderr, "Error reading file\n");
        exit(1);
    }
}

void read_file() {
    particles = malloc(sizeof(struct Particle));
    particles->x_pos = malloc(sizeof(double) * n);
    particles->y_pos = malloc(sizeof(double) * n);
    particles->mass = malloc(sizeof(double) * n);
    particles->x_velocity = malloc(sizeof(double) * n);
    particles->y_velocity = malloc(sizeof(double) * n);
    particles->brightness = malloc(sizeof(double) * n);

    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        read_bytes(&particles->x_pos[i], file);
        read_bytes(&particles->y_pos[i], file);
        read_bytes(&particles->mass[i], file);
        read_bytes(&particles->x_velocity[i], file);
        read_bytes(&particles->y_velocity[i], file);
        read_bytes(&particles->brightness[i], file);

        if (particles->mass[i] > largest_particle) {
            largest_particle = particles->mass[i];
        }
        if (particles->brightness[i] > brightest) {
            brightest = particles->brightness[i];
        }
    }
    fclose(file);
}

void write_file() {
    FILE *file = fopen("result.gal", "w");
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        fwrite(&particles->x_pos[i], sizeof(double), 1, file);
        fwrite(&particles->y_pos[i], sizeof(double), 1, file);
        fwrite(&particles->mass[i], sizeof(double), 1, file);
        fwrite(&particles->x_velocity[i], sizeof(double), 1, file);
        fwrite(&particles->y_velocity[i], sizeof(double), 1, file);
        fwrite(&particles->brightness[i], sizeof(double), 1, file);
    }
    fclose(file);
}

Window create_simple_window(Display *display, int width, int height, int x,
                            int y) {
    int screen_num = DefaultScreen(display);
    int win_border_width = 2;

    Window win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                                     x, y, width, height, win_border_width,
                                     BlackPixel(display, screen_num),
                                     WhitePixel(display, screen_num));

    XMapWindow(display, win);
    XFlush(display);

    return win;
}

GC create_gc(Display *display, Window win) {
    unsigned long valuemask = 0;
    XGCValues values;
    int screen_num = DefaultScreen(display);

    GC gc = XCreateGC(display, win, valuemask, &values);
    if (gc < (GC)0) {
        fprintf(stderr, "XCreateGC: \n");
    }

    XSetForeground(display, gc, WhitePixel(display, screen_num));
    XSetBackground(display, gc, BlackPixel(display, screen_num));

    XSetFillStyle(display, gc, FillSolid);

    return gc;
}

void SetCAxes(float cmin, float cmax) {
    caxis[0] = cmin;
    caxis[1] = cmax;
}

void InitializeGraphics(char *command, int windowWidth, int windowHeight) {
    char *display_name = getenv("DISPLAY");
    Colormap screen_colormap;

    width = windowWidth;
    height = windowHeight;

    global_display_ptr = XOpenDisplay(display_name);
    if (global_display_ptr == NULL) {
        fprintf(stderr, "%s: cannot connect to X server '%s'\n", command,
                display_name);
        exit(1);
    }

    Screen *screen = ScreenOfDisplay(global_display_ptr, 0);
    int screen_num = XScreenNumberOfScreen(screen);

    win = create_simple_window(global_display_ptr, width, height, 0, 0);
    pixmap = XCreatePixmap(global_display_ptr, win, width, height,
                           DefaultDepthOfScreen(screen));

    gc = create_gc(global_display_ptr, win);
    XSync(global_display_ptr, False);

    screen_colormap =
        DefaultColormap(global_display_ptr, DefaultScreen(global_display_ptr));

    black = BlackPixel(global_display_ptr, screen_num);
    white = WhitePixel(global_display_ptr, screen_num);

    XColor color;
    for (int i = 0; i < NUMCOLORS; i++) {
        color.red = ((double)(NUMCOLORS - i) / (double)NUMCOLORS) * 0xFFFF;
        color.blue = color.red;
        color.green = color.red;
        XAllocColor(global_display_ptr, screen_colormap, &color);
        colors[i] = color.pixel;
    }
    SetCAxes(0, 1);
}

void Refresh(void) {
    XCopyArea(global_display_ptr, pixmap, win, gc, 0, 0, width, height, 0, 0);
    XFlush(global_display_ptr);
}

void ClearScreen(void) {
    XSetForeground(global_display_ptr, gc, black);
    XFillRectangle(global_display_ptr, pixmap, gc, 0, 0, width, height);
}

void DrawCircle(float x, float y, float W, float H, float radius, float color) {
    int i = (int)((x - radius) / W * width);
    int j = height - (int)((y + radius) / H * height);
    int arcrad = 2 * (int)(radius / W * width);
    int icolor;

    if (color >= caxis[1])
        icolor = NUMCOLORS - 1;
    else if (color < caxis[0])
        icolor = 0;
    else
        icolor = (int)((color - caxis[0]) / (caxis[1] - caxis[0]) *
                       (float)NUMCOLORS);

    XSetForeground(global_display_ptr, gc, colors[icolor]);
    XFillArc(global_display_ptr, pixmap, gc, i, j, arcrad, arcrad, 0, 64 * 360);
}

void FlushDisplay() { XFlush(global_display_ptr); }

void CloseDisplay() {
    XFreeGC(global_display_ptr, gc);
    XCloseDisplay(global_display_ptr);
}

const double frame_rate = 1.0 / 60.0;
struct timespec last_frame;

void draw_galaxy() {
    double time_spent;
    // If the last frame was less than framerate seconds ago, wait to draw it
    do {
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        time_spent = (current_time.tv_sec - last_frame.tv_sec) +
                     (current_time.tv_nsec - last_frame.tv_nsec) / 1000000000.0;
    } while (time_spent < frame_rate);

    ClearScreen();
    for (int i = 0; i < n; i++) {
        double x = particles->x_pos[i];
        double y = particles->y_pos[i];
        double r = max(0.002, 0.1 / n * particles->mass[i] / largest_particle);
        double color = 1.0 - particles->brightness[i] / brightest;
        DrawCircle(x * 1, y * 1, 1, 1, r, color);
    }
    Refresh();
    clock_gettime(CLOCK_MONOTONIC, &last_frame);
}

void step() {
    const double G = 100.0 / n;

    double *accel_x = calloc(n, sizeof(double));
    double *accel_y = calloc(n, sizeof(double));

    for (int i = 0; i < n; i++) {
        double accel_i_x = 0;
        double accel_i_y = 0;
        // Using Newtons third law, we can save about 50% of all iterations
        for (int j = i + 1; j < n; j++) {
            double dx = particles->x_pos[i] - particles->x_pos[j];
            double dy = particles->y_pos[i] - particles->y_pos[j];

            double distance = sqrt(dx * dx + dy * dy);

            double force_multiplier = G / pow(distance + epsilon, 3);

            double force_x = force_multiplier * dx;
            double force_y = force_multiplier * dy;

            accel_i_x -= force_x * particles->mass[j];
            accel_i_y -= force_y * particles->mass[j];

            accel_x[j] += force_x * particles->mass[i];
            accel_y[j] += force_y * particles->mass[i];
        }

        accel_x[i] += accel_i_x;
        accel_y[i] += accel_i_y;
    }

    // Update all velocities and positions in one go
    for (int i = 0; i < n; i++) {
        particles->x_velocity[i] += accel_x[i] * delta_time;
        particles->y_velocity[i] += accel_y[i] * delta_time;

        particles->x_pos[i] += particles->x_velocity[i] * delta_time;
        particles->y_pos[i] += particles->y_velocity[i] * delta_time;
    }

    if (graphics) {
        draw_galaxy();
    }

    free(accel_x);
    free(accel_y);
}

void free_particles() {
    free(particles->x_pos);
    free(particles->y_pos);
    free(particles->mass);
    free(particles->x_velocity);
    free(particles->y_velocity);
    free(particles->brightness);
    free(particles);
}

int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Usage: ./galsim N filename nsteps delta_time graphics");
        return 1;
    }
    read_arguments(argv);
    read_file();

    if (graphics) {
        InitializeGraphics(argv[0], 3000, 1500);
    }

    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    double start = start_time.tv_sec + start_time.tv_nsec / 1000000000.0;

    for (int i = 0; i < nsteps; i++) {
        step();
    }

    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double end = end_time.tv_sec + end_time.tv_nsec / 1000000000.0;

    printf("wall seconds: %.15lf \n", end - start);

    if (graphics) {
        FlushDisplay();
        CloseDisplay();
    }

    write_file();

    free_particles();
}
