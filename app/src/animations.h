#include <zephyr/drivers/led_strip.h>

typedef enum {
	SUPER_DIMM = 5,
	DIMM = 4,
	NORMAL = 3,
	BRIGHT = 2,
	BRIGHTER = 1,
	SUPER_BRIGHT = 0,
} brightness_level;

typedef int (*animation)(struct led_rgb *pixels, int size, brightness_level brightness);

int animations_off(struct led_rgb *pixels, int size, brightness_level brightness);
int american_police(struct led_rgb *pixels, int size, brightness_level brightness);
int blaulicht(struct led_rgb pixels[], int size, brightness_level brightness);
int bauhelm(struct led_rgb pixels[], int size, brightness_level brightness);
int color_fill(struct led_rgb *pixels, int size, brightness_level brightness);
int chasing_pixel(struct led_rgb *pixels, int size, brightness_level brightness);
int color_in_rainbow(struct led_rgb *pixels, int size, brightness_level brightness);
