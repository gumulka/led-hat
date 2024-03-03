#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>

#include "animations.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

static const struct device *const strip = DEVICE_DT_GET(DT_ALIAS(led_strip));

#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)

animation animations[] = {
	color_fill,
	chasing_pixel,
	color_in_rainbow,
};

int main(void)
{
	struct led_rgb pixels[STRIP_NUM_PIXELS] = {0};

	animation current = animations[1];

	if (!device_is_ready(strip)) {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return -ENODEV;
	}

	LOG_INF("Displaying pattern on strip");
	while (1) {
		int delay = current(pixels, STRIP_NUM_PIXELS, DIMM);
		int rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
		if (rc) {
			LOG_ERR("couldn't update strip: %d", rc);
		}

		k_sleep(K_MSEC(delay));
	}
	return 0;
}
