#include "animations.h"

#include <string.h>

int animations_off(struct led_rgb *pixels, int size, brightness_level brightness)
{
	return 1000;
}

#define RGB(_r, _g, _b)                                                                            \
	{                                                                                          \
		.r = (_r), .g = (_g), .b = (_b)                                                    \
	}

static const struct led_rgb colors[] = {
	RGB(0xff, 0x00, 0x00),
	RGB(0x00, 0x7f, 0x00),
	RGB(0x00, 0x00, 0xff),
};

int american_police(struct led_rgb pixels[], int size, brightness_level brightness)
{
	static size_t counter = 0;

	memset(pixels, 0x00, sizeof(struct led_rgb) * size);

	if (counter == 0) {
		for (int i = 0; i < size / 2; i++) {
			pixels[(i + 32) % size].r = 0xff >> brightness;
		}
		counter = 1;
	} else {
		for (int i = size / 2; i < size; i++) {
			pixels[(i + 32) % size].b = 0xff >> brightness;
		}
		counter = 0;
	}

	return 500;
}

int siren(struct led_rgb pixels[], int size, brightness_level brightness, struct led_rgb color)
{
	static size_t counter = 0;

	memset(pixels, 0x00, sizeof(struct led_rgb) * size);

	int number = size / 5;

	for (int i = 0; i < number; i++) {
		pixels[(i + counter) % size].r = color.r >> brightness;
		pixels[(i + counter) % size].g = color.g >> brightness;
		pixels[(i + counter) % size].b = color.b >> brightness;
	}
	counter = (counter + 1) % size;

	return 2;
}

int blaulicht(struct led_rgb pixels[], int size, brightness_level brightness)
{
	static struct led_rgb blau = RGB(0x00, 0x00, 0xff);
	return siren(pixels, size, brightness, blau);
}

int bauhelm(struct led_rgb pixels[], int size, brightness_level brightness)
{
	static struct led_rgb gelb = RGB(0xff, 0x7F, 0x00);
	return siren(pixels, size, brightness, gelb);
}

int color_fill(struct led_rgb *pixels, int size, brightness_level brightness)
{
	static size_t cursor = 0;
	static size_t color = 0;

	cursor++;
	if (cursor >= size) {
		cursor = 0;
		color++;
		if (color == ARRAY_SIZE(colors)) {
			color = 0;
		}
	}

	pixels[cursor].r = colors[color].r >> brightness;
	pixels[cursor].g = colors[color].g >> brightness;
	pixels[cursor].b = colors[color].b >> brightness;
	return 20;
}

int chasing_pixel(struct led_rgb *pixels, int size, brightness_level brightness)
{
	memset(pixels, 0x00, sizeof(struct led_rgb) * size);
	return color_fill(pixels, size, brightness);
}

int color_in_rainbow(struct led_rgb *pixels, int size, brightness_level brightness)
{
	static size_t offset = 0;
	offset = (offset + 1) % size;
	memset(pixels, 0x00, sizeof(struct led_rgb) * size);
	int section_length = (size + 2) / 3;

	for (int i = 0; i < section_length; i++) {
		// red to green
		pixels[(i + offset) % size].r =
			(0xff * (section_length - i) / section_length) >> brightness;
		pixels[(i + offset) % size].g = (0x7f * i / section_length) >> brightness;
		// green to blue
		pixels[(i + section_length + offset) % size].g =
			(0x7f * (section_length - i) / section_length) >> brightness;
		pixels[(i + section_length + offset) % size].b =
			(0xff * i / section_length) >> brightness;
		// blue to red
		pixels[(i + section_length * 2 + offset) % size].b =
			(0xff * (section_length - i) / section_length) >> brightness;
		pixels[(i + section_length * 2 + offset) % size].r =
			(0xff * i / section_length) >> brightness;
	}
	return 200;
}
