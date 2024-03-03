#include <errno.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

static const struct device *const strip = DEVICE_DT_GET(DT_ALIAS(led_strip));

#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)

#define RGB(_r, _g, _b)                                                                            \
	{                                                                                          \
		.r = (_r), .g = (_g), .b = (_b)                                                    \
	}

static const struct led_rgb colors[] = {
	RGB(0x07, 0x00, 0x00),
	RGB(0x00, 0x03, 0x00),
	RGB(0x00, 0x00, 0x07),
};

struct led_rgb pixels[STRIP_NUM_PIXELS];

int main(void)
{
	size_t cursor = 0, color = 0;

	if (!device_is_ready(strip)) {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return -ENODEV;
	}

	LOG_INF("Displaying pattern on strip");
	while (1) {
		memcpy(&pixels[cursor], &colors[color], sizeof(struct led_rgb));
		int rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);

		if (rc) {
			LOG_ERR("couldn't update strip: %d", rc);
		}

		cursor++;
		if (cursor >= STRIP_NUM_PIXELS) {
			cursor = 0;
			color++;
			if (color == ARRAY_SIZE(colors)) {
				color = 0;
			}
		}

		k_sleep(K_MSEC(20));
	}
	return 0;
}
