#include <errno.h>
#include <stddef.h>

#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/types.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "animations.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

animation animations[] = {
	color_fill,
	chasing_pixel,
	color_in_rainbow,
	animations_off,
};

/* LED Service Variables */
#define BT_UUID_LED_SERVICE_VAL                                                                    \
	BT_UUID_128_ENCODE(0x0863b1c3, 0x7722, 0x4fcc, 0xaf75, 0xd5f6f3100000)

static struct bt_uuid_128 led_uuid = BT_UUID_INIT_128(BT_UUID_LED_SERVICE_VAL);

static struct bt_uuid_128 led_brightness_uuid =
	BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x0863b1c3, 0x7722, 0x4fcc, 0xaf75, 0xd5f6f3100001));

static struct bt_uuid_128 led_animation_uuid =
	BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x0863b1c3, 0x7722, 0x4fcc, 0xaf75, 0xd5f6f3100002));

static struct bt_uuid_128 led_speed_uuid =
	BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x0863b1c3, 0x7722, 0x4fcc, 0xaf75, 0xd5f6f3100003));

struct bluetooth_value {
	int min;
	int max;
	int value;
};

static struct bluetooth_value bt_brightness = {
	.min = SUPER_BRIGHT,
	.max = SUPER_DIMM,
	.value = NORMAL,
};
static struct bluetooth_value bt_animation = {
	.min = 0,
	.max = ARRAY_SIZE(animations) - 1,
	.value = 0,
};
static struct bluetooth_value bt_speed = {
	.min = -9,
	.max = 10,
	.value = 0,
};

static ssize_t read_bt_value(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			     uint16_t len, uint16_t offset)
{
	const struct bluetooth_value *value = attr->user_data;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &(value->value), sizeof(int));
}

static ssize_t write_bt_value(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			      const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	struct bluetooth_value *value = attr->user_data;

	if (offset + len > sizeof(int)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	int old = value->value;

	memcpy(&(value->value) + offset, buf, len);

	if (value->value < value->min || value->value > value->max) {
		value->value = old;
		return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);
	}
	return len;
}

static void led_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	;
}

#define BLE_GATT_CPF_FORMAT_SINT32 0x10

static const struct bt_gatt_cpf value_cpf = {
	.format = BLE_GATT_CPF_FORMAT_SINT32,
};

/* LED Service Declaration */
BT_GATT_SERVICE_DEFINE(
	led_service, BT_GATT_PRIMARY_SERVICE(&led_uuid),
	BT_GATT_CHARACTERISTIC(&led_brightness_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, read_bt_value,
			       write_bt_value, &bt_brightness),
	BT_GATT_CUD("Brightness", BT_GATT_PERM_READ), BT_GATT_CPF(&value_cpf),
	BT_GATT_CCC(led_ccc_cfg_changed, BT_GATT_PERM_READ),
	BT_GATT_CHARACTERISTIC(&led_animation_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, read_bt_value,
			       write_bt_value, &bt_animation),
	BT_GATT_CUD("Animation", BT_GATT_PERM_READ), BT_GATT_CPF(&value_cpf),
	BT_GATT_CCC(led_ccc_cfg_changed, BT_GATT_PERM_READ),
	BT_GATT_CHARACTERISTIC(&led_speed_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, read_bt_value,
			       write_bt_value, &bt_speed),
	BT_GATT_CUD("Speed", BT_GATT_PERM_READ), BT_GATT_CPF(&value_cpf),
	BT_GATT_CCC(led_ccc_cfg_changed, BT_GATT_PERM_READ), );

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_DIS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LED_SERVICE_VAL),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Connection failed (err 0x%02x)", err);
	} else {
		LOG_INF("Connected");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason 0x%02x)", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static const struct device *const strip = DEVICE_DT_GET(DT_ALIAS(led_strip));

#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)

int main(void)
{
	int random_value;
	sys_csrand_get(&random_value, sizeof(random_value));
	bt_animation.value = random_value % ARRAY_SIZE(animations);

	struct led_rgb pixels[STRIP_NUM_PIXELS] = {0};

	if (!device_is_ready(strip)) {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return -ENODEV;
	}

	int err;

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return 0;
	}

	LOG_INF("Bluetooth initialized");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return 0;
	}

	LOG_INF("Advertising successfully started");

	LOG_INF("Displaying pattern on strip");
	while (1) {
		int delay = animations[bt_animation.value](pixels, STRIP_NUM_PIXELS,
							   bt_brightness.value);
		int rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
		if (rc) {
			LOG_ERR("couldn't update strip: %d", rc);
		}

		k_sleep(K_MSEC(delay * (bt_speed.value + 10) / 10));
	}
	return 0;
}
