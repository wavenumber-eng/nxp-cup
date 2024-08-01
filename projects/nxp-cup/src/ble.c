/*
 * ble.c Sets up a peripheral BLE service with a characteristic for a board using
 * Zephyr BLE libraries. The purpose is to forward commands from a BLE delegate
 * to a peripheral and then translate sent data to i2c traffic.
 */

/*
 * Forwarded from:
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/ias.h>

#include "motor.h"

/* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL \
    BT_UUID_128_ENCODE(0x4fafc201, 0x1fb5, 0x459e, 0x8fcc, 0xc5c9c331915c)

static const struct bt_uuid_128 vnd_uuid = BT_UUID_INIT_128(
    BT_UUID_CUSTOM_SERVICE_VAL);

#define BT_UUID_CUSTOM_CHARACTERISTIC_VAL \
    BT_UUID_128_ENCODE(0xbeb5483e, 0x36e1, 0x4688, 0xb7f5, 0xea07361b26a8)

static const struct bt_uuid_128 vnd_char_uuid = BT_UUID_INIT_128(
    BT_UUID_CUSTOM_CHARACTERISTIC_VAL);

static const struct bt_uuid_128 vnd_enc_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0xbeb5483e, 0x36e1, 0x4688, 0xb7f5, 0xea07361b26a8));

#define VND_MAX_LEN 20

static uint8_t vnd_value[VND_MAX_LEN + 1] = { 'V', 'e', 'n', 'd', 'o', 'r'};

static ssize_t read_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 strlen(value));
}

// Callback for when the characteristic changes and prints it to the shell.
static void on_vnd_value_change(const struct bt_gatt_attr *attr)
{
    uint8_t *value = attr->user_data;

	char *converted_val = (char *)value;

    printk("Characteristic value changed: %s\n", value);
	printk("Converted Val: %s\n", converted_val);

	char int_str[10]; // Buffer to hold the substring as a string
    int value_as_int;

    // Ensure the end index is within bounds and that the substring is properly null-terminated
    strncpy(int_str, converted_val + 1, sizeof(int_str) - 1);
    int_str[sizeof(int_str) - 1] = '\0'; // Null-terminate the substring

    // Convert the substring to an integer
    value_as_int = atoi(int_str);

    printk("Converted integer value: %d\n", value_as_int);

	switch (converted_val[0])
	{
        case 'P':
            printk("BLE Characteristic: Drive detected\n");

			// Change atomic variable for Drive
			motor_set_state_drive_tgt(value_as_int);

            break;
        case 'S':
            printk("BLE Characteristic: Steer detected\n");

			// Change atomic variable for Steer
			motor_set_state_steer_tgt(value_as_int);

            break;
        default:
            printk("BLE Characteristic: Unhandled command detected\n");
            break;
    }
}

static ssize_t write_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         const void *buf, uint16_t len, uint16_t offset,
                         uint8_t flags)
{
    uint8_t *value = attr->user_data;

    if (offset + len > VND_MAX_LEN) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);
    value[offset + len] = 0;

    on_vnd_value_change(attr);

    return len;
}

static uint8_t simulate_vnd;
static uint8_t indicating;
static struct bt_gatt_indicate_params ind_params;

static ssize_t write_without_rsp_vnd(struct bt_conn *conn,
				     const struct bt_gatt_attr *attr,
				     const void *buf, uint16_t len, uint16_t offset,
				     uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (!(flags & BT_GATT_WRITE_FLAG_CMD)) {
		/* Write Request received. Reject it since this Characteristic
		 * only accepts Write Without Response.
		 */
		return BT_GATT_ERR(BT_ATT_ERR_WRITE_REQ_REJECTED);
	}

	if (offset + len > VND_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

static void indicate_cb(struct bt_conn *conn,
			struct bt_gatt_indicate_params *params, uint8_t err)
{
	printk("Indication %s\n", err != 0U ? "fail" : "success");
}

static void indicate_destroy(struct bt_gatt_indicate_params *params)
{
	printk("Indication complete\n");
	indicating = 0U;
}

/* Vendor Primary Service Declaration */
BT_GATT_SERVICE_DEFINE(vnd_svc,
    BT_GATT_PRIMARY_SERVICE(&vnd_uuid),
    BT_GATT_CHARACTERISTIC(&vnd_char_uuid.uuid,
                           BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_WRITE, NULL,
                           write_vnd, &vnd_value),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_HRS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_CTS_VAL)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {
	.att_mtu_updated = mtu_updated
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void) {
    int err;

    printk("Bluetooth initialized\n");

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("Advertising successfully started\n");
}

int ble_init(void)
{
	struct bt_gatt_attr *vnd_ind_attr;
	char str[BT_UUID_STR_LEN];
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	bt_ready();

	bt_gatt_cb_register(&gatt_callbacks);

	vnd_ind_attr = bt_gatt_find_by_uuid(vnd_svc.attrs, vnd_svc.attr_count,
					    &vnd_enc_uuid.uuid);
	bt_uuid_to_str(&vnd_enc_uuid.uuid, str, sizeof(str));
	printk("Indicate VND attr %p (UUID %s)\n", vnd_ind_attr, str);

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */

	/*
	while (1) {
		k_sleep(K_SECONDS(1));
		if (simulate_vnd && vnd_ind_attr) {
			if (indicating) {
				continue;
			}

			ind_params.attr = vnd_ind_attr;
			ind_params.func = indicate_cb;
			ind_params.destroy = indicate_destroy;
			ind_params.data = &indicating;
			ind_params.len = sizeof(indicating);

			if (bt_gatt_indicate(NULL, &ind_params) == 0) {
				indicating = 1U;
			}
		}
	}
	*/

	return 0;
}