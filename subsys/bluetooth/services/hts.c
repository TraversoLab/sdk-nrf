/*
Copyright (c) 2024 Traverso Laboratory, Massachusetts Institute of Technology

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <bluetooth/services/hts.h>

LOG_MODULE_REGISTER(bt_hts, CONFIG_BT_HTS_LOG_LEVEL);

static bool hts_temp_indicate_enabled = false, 
    hts_intermediate_notify_enabled = false, 
    hts_interval_indicate_enabled = false;

static struct hts_callbacks hts_cb;

static void hts_temp_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value) {
	hts_temp_indicate_enabled = (value == BT_GATT_CCC_INDICATE);
}

static void hts_intermediate_temp_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value) {
    hts_intermediate_notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}

static void hts_intermediate_temp_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value) {
    hts_interval_indicate_enabled = (value == BT_GATT_CCC_INDICATE);
}

static HTS_Data hts_data = {
            .error = 0,
            .temperature = { 
                .decoded = HTS_MEASUREMENT_NAN //init as NAN value because no measurement has occurred yet
                }, 
            .config = {
                .encoded_flags = 0
            }
        };

static uint8_t hts_measurement_prepare(struct HTS_Data* p_hts_meas) {
    if (p_hts_meas->error) {
        return 0;
    }

    if (p_hts_meas->temperature.decoded.mantissa.value == 0x7FFFFF) {
        return 0;
    }

    return 1;
}

static void hts_polled_read_temperature(struct bt_conn *conn,
			  const struct bt_gatt_attr *attr) {

	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle,
		(void *)conn);

	if (lbs_cb.read_temp) {
        auto flags = bt_gatt_attr_read(conn, )
		return 
	}

	return 0.0;
}

BT_GATT_SERVICE_DEFINE(hts_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_HTS),
    BT_GATT_CHARACTERISTIC(BT_UUID_HTS_TEMPERATURE,
			       BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ_LESC, hts_polled_read_temperature, NULL,
			       &temperature_data),
    BT_GATT_CCC(hts_temp_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(BT_UUID_HTS_TEMPERATURE_TYPE,
			       BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ_LESC, hts_polled_read_temperature, NULL,
			       &temperature_data),
    BT_GATT_CHARACTERISTIC(BT_UUID_HTS_INTERMEDIATE_TEMPERATURE,
                   BT_GATT_CHRC_NOTIFY,
                   BT_GATT_PERM_READ_LESC, NULL, NULL,
                   &temperature_data),
    BT_GATT_CCC(hts_intermediate_temp_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(BT_UUID_HTS_MEASUREMENT_INTERVAL,
                   BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                   BT_GATT_PERM_READ_LESC | BT_GATT_PERM_WRITE_LESC, NULL, NULL,
                   NULL),
    BT_GATT_CCC(hts_intermediate_temp_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
); 

void bt_hts_indicate_temperature(ieee_float32_encoded_t temperature) {
    if (hts_temp_indicate_enabled) {
        bt_gatt_notify(NULL, &hts_svc.attrs[1], &temperature, sizeof(temperature));
    }
}

int bt_hts_init(struct bt_hts_cb *hts_callbacks) {
    if (hts_callbacks) {
        hts_cb.indicated = hts_callbacks->indicated;
        hts_cb.intermediate_temperature_notified = hts_callbacks->intermediate_temperature_notified;
        hts_cb.temp_config_changed = hts_callbacks->temp_config_changed;
        hts_cb.temp_type_read = hts_callbacks->temp_type_read;
    }

    return 0;
}