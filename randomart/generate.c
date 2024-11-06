#include <math.h>
#include <stdint.h>
#include <libgen.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


typedef struct {
	uint8_t r, g, b, a;
} RGBA;

#define WIDTH 1000
#define HEIGHT 1000

float clamp(float x, float min, float max)
{
	return x > max ? max : x < min ? min : x;
}

RGBA color(float r, float g, float b)
{
	uint8_t rr = (clamp(r, -1, 1)+1)/2 * 255;
	uint8_t gg = (clamp(g, -1, 1)+1)/2 * 255;
	uint8_t bb = (clamp(b, -1, 1)+1)/2 * 255;

	return (RGBA) {rr,gg,bb,255};
}

float add(float a, float b)
{
	return a + b;
}

float mult(float a, float b)
{
	return a * b;
}

float mod(float a, float b)
{
	return fmodf(a, b);
}

RGBA randomart(float x, float y)
{
	return
#include "simple.c"
	;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <output path> \n", basename(argv[0]));
		return 2;
	}

	char const *output_path = argv[1];

	RGBA pixels[WIDTH * HEIGHT];

	for (size_t y = 0; y < HEIGHT; ++y) {
		float const ny = ((float)y)/(HEIGHT-1) * 2 - 1;
		for (size_t x = 0; x < WIDTH; ++x) {
			float const nx = ((float)x)/(WIDTH-1) * 2 - 1;
			pixels[y*WIDTH+x] = randomart(nx, ny);
		}
	}

	if (!stbi_write_png(output_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(RGBA))) {
		fprintf(stderr, "failed to write png to: %s\n", output_path);
		return 1;
	}

	return 0;
}
