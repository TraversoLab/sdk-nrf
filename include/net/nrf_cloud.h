/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NRF_CLOUD_H__
#define NRF_CLOUD_H__

#include <zephyr.h>
#include <zephyr/types.h>
#include <net/mqtt.h>
#if defined(CONFIG_MODEM_INFO)
#include <modem/modem_info.h>
#endif
#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup nrf_cloud nRF Cloud
 * @{
 */

/** @defgroup nrf_cloud_mqtt_msg_ids MQTT message IDs for nRF Cloud.
 * @{
 */
#define NCT_MSG_ID_USE_NEXT_INCREMENT     0
#define NCT_MSG_ID_CC_SUB               100
#define NCT_MSG_ID_DC_SUB               101
#define NCT_MSG_ID_CG_SUB               102
#define NCT_MSG_ID_FOTA_SUB             103
#define NCT_MSG_ID_CC_UNSUB             150
#define NCT_MSG_ID_DC_UNSUB             151
#define NCT_MSG_ID_CG_UNSUB             152
#define NCT_MSG_ID_FOTA_UNSUB           153
#define NCT_MSG_ID_STATE_REQUEST        200
#define NCT_MSG_ID_FOTA_REQUEST         201
#define NCT_MSG_ID_FOTA_BLE_REQUEST     202
#define NCT_MSG_ID_STATE_REPORT         300
#define NCT_MSG_ID_PAIR_STATUS_REPORT   301
#define NCT_MSG_ID_FOTA_REPORT          302
#define NCT_MSG_ID_FOTA_BLE_REPORT      303
#define NCT_MSG_ID_INCREMENT_BEGIN     1000
#define NCT_MSG_ID_INCREMENT_END       9999
#define NCT_MSG_ID_USER_TAG_BEGIN      (NCT_MSG_ID_INCREMENT_END + 1)
#define NCT_MSG_ID_USER_TAG_END        0xFFFF /* MQTT message IDs are uint16_t */
/** @} */

#define IS_VALID_USER_TAG(tag) ((tag >= NCT_MSG_ID_USER_TAG_BEGIN) && \
				(tag <= NCT_MSG_ID_USER_TAG_END))

#define NRF_CLOUD_SETTINGS_NAME "nrf_cloud"

#define NRF_CLOUD_FOTA_VER              2
#define NRF_CLOUD_FOTA_VER_STR "fota_v" STRINGIFY(NRF_CLOUD_FOTA_VER)

#define NRF_CLOUD_STAGE_ID_MAX_LEN      8
#define NRF_CLOUD_TENANT_ID_MAX_LEN     64
#define NRF_CLOUD_CLIENT_ID_MAX_LEN     64

/** @brief Asynchronous nRF Cloud events notified by the module. */
enum nrf_cloud_evt_type {
	/** The transport to the nRF Cloud is established. */
	NRF_CLOUD_EVT_TRANSPORT_CONNECTED = 0x1,
	/** In the process of connecting to nRF Cloud. */
	NRF_CLOUD_EVT_TRANSPORT_CONNECTING,
	/** There was a request from nRF Cloud to associate the device
	 * with a user on the nRF Cloud.
	 */
	NRF_CLOUD_EVT_USER_ASSOCIATION_REQUEST,
	/** The device is successfully associated with a user. */
	NRF_CLOUD_EVT_USER_ASSOCIATED,
	/** The device can now start sending sensor data to the cloud. */
	NRF_CLOUD_EVT_READY,
	/** The device received data from the cloud. */
	NRF_CLOUD_EVT_RX_DATA,
	/** The data sent to the cloud was acknowledged. */
	NRF_CLOUD_EVT_SENSOR_DATA_ACK,
	/** The transport was disconnected. */
	NRF_CLOUD_EVT_TRANSPORT_DISCONNECTED,
	/** The device should be restarted to apply a firmware upgrade */
	NRF_CLOUD_EVT_FOTA_DONE,
	/** There was an error communicating with the cloud. */
	NRF_CLOUD_EVT_ERROR = 0xFF
};

/**@ nRF Cloud disconnect status. */
enum nrf_cloud_disconnect_status {
	NRF_CLOUD_DISCONNECT_USER_REQUEST,
	NRF_CLOUD_DISCONNECT_CLOSED_BY_REMOTE,
	NRF_CLOUD_DISCONNECT_INVALID_REQUEST,
	NRF_CLOUD_DISCONNECT_MISC,
	NRF_CLOUD_DISCONNECT_COUNT
};

/**@ nRF Cloud connect result. */
enum nrf_cloud_connect_result {
	NRF_CLOUD_CONNECT_RES_SUCCESS = 0,
	NRF_CLOUD_CONNECT_RES_ERR_NOT_INITD = -1,
	NRF_CLOUD_CONNECT_RES_ERR_INVALID_PARAM = -2,
	NRF_CLOUD_CONNECT_RES_ERR_NETWORK = -3,
	NRF_CLOUD_CONNECT_RES_ERR_BACKEND = -4,
	NRF_CLOUD_CONNECT_RES_ERR_MISC = -5,
	NRF_CLOUD_CONNECT_RES_ERR_NO_MEM = -6,
	/* Invalid private key */
	NRF_CLOUD_CONNECT_RES_ERR_PRV_KEY = -7,
	/* Invalid CA or client cert */
	NRF_CLOUD_CONNECT_RES_ERR_CERT = -8,
	/* Other cert issue */
	NRF_CLOUD_CONNECT_RES_ERR_CERT_MISC = -9,
	/* Timeout, SIM card may be out of data */
	NRF_CLOUD_CONNECT_RES_ERR_TIMEOUT_NO_DATA = -10,
	NRF_CLOUD_CONNECT_RES_ERR_ALREADY_CONNECTED = -11,
};

/** @brief Sensor types supported by the nRF Cloud. */
enum nrf_cloud_sensor {
	/** The GPS sensor on the device. */
	NRF_CLOUD_SENSOR_GPS,
	/** The FLIP movement sensor on the device. */
	NRF_CLOUD_SENSOR_FLIP,
	/** The Button press sensor on the device. */
	NRF_CLOUD_SENSOR_BUTTON,
	/** The TEMP sensor on the device. */
	NRF_CLOUD_SENSOR_TEMP,
	/** The Humidity sensor on the device. */
	NRF_CLOUD_SENSOR_HUMID,
	/** The Air Pressure sensor on the device. */
	NRF_CLOUD_SENSOR_AIR_PRESS,
	/** The Air Quality sensor on the device. */
	NRF_CLOUD_SENSOR_AIR_QUAL,
	/** The RSPR data obtained from the modem. */
	NRF_CLOUD_LTE_LINK_RSRP,
	/** The descriptive DEVICE data indicating its status. */
	NRF_CLOUD_DEVICE_INFO,
	/** The light sensor on the device. */
	NRF_CLOUD_SENSOR_LIGHT,
};

/** @brief Topic types supported by nRF Cloud. */
enum nrf_cloud_topic_type {
	/** Endpoint used to update the cloud-side device shadow state . */
	NRF_CLOUD_TOPIC_STATE = 0x1,
	/** Endpoint used to directly message the nRF Cloud Web UI. */
	NRF_CLOUD_TOPIC_MESSAGE,
	/** Endpoint used to publish bulk messages to nRF Cloud. Bulk messages combine multiple
	 *  messages into a single message that will be unwrapped and re-published to the
	 *  message topic in the nRF Cloud backend.
	 */
	NRF_CLOUD_TOPIC_BULK
};

/**@brief FOTA status reported to nRF Cloud. */
enum nrf_cloud_fota_status {
	NRF_CLOUD_FOTA_QUEUED = 0,
	NRF_CLOUD_FOTA_IN_PROGRESS = 1,
	NRF_CLOUD_FOTA_FAILED = 2,
	NRF_CLOUD_FOTA_SUCCEEDED = 3,
	NRF_CLOUD_FOTA_TIMED_OUT = 4,
	NRF_CLOUD_FOTA_CANCELED = 5,
	NRF_CLOUD_FOTA_REJECTED = 6,
	NRF_CLOUD_FOTA_DOWNLOADING = 7,
};

/**@brief FOTA update type. */
enum nrf_cloud_fota_type {
	NRF_CLOUD_FOTA_TYPE__FIRST = 0,

	/** Application update. */
	NRF_CLOUD_FOTA_APPLICATION = NRF_CLOUD_FOTA_TYPE__FIRST,
	/** Modem update. */
	NRF_CLOUD_FOTA_MODEM = 1,
	/** Bootloader update. */
	NRF_CLOUD_FOTA_BOOTLOADER = 2,

	NRF_CLOUD_FOTA_TYPE__INVALID
};

/**@brief Common FOTA job info */
struct nrf_cloud_fota_job_info {
	enum nrf_cloud_fota_type type;
	/** Null-terminated FOTA job identifier */
	char *id;
	char *host;
	char *path;
	int file_size;
};

/**@brief Generic encapsulation for any data that is sent to the cloud. */
struct nrf_cloud_data {
	/** Length of the data. */
	uint32_t len;
	/** Pointer to the data. */
	const void *ptr;
};

/**@brief MQTT topic. */
struct nrf_cloud_topic {
	/** Length of the topic. */
	uint32_t len;
	/** Pointer to the topic. */
	const void *ptr;
};

/**@brief Sensors that are supported by the device. */
struct nrf_cloud_sensor_list {
	/** Size of the list. */
	uint8_t size;
	/** Supported sensor types. */
	const enum nrf_cloud_sensor *ptr;
};

/**@brief Connection parameters. */
struct nrf_cloud_connect_param {
	/** Supported sensor types. May be NULL. */
	const struct nrf_cloud_sensor_list *sensor;
};

/**@brief Sensor data transmission parameters. */
struct nrf_cloud_sensor_data {
	/** The sensor that is the source of the data. */
	enum nrf_cloud_sensor type;
	/** Sensor data to be transmitted. */
	struct nrf_cloud_data data;
	/** Unique tag to identify the sent data. Can be used to match
	 * acknowledgment on the NRF_CLOUD_EVT_SENSOR_DATA_ACK event.
	 * Valid range: NCT_MSG_ID_USER_TAG_BEGIN to NCT_MSG_ID_USER_TAG_END.
	 * Any other value will suppress the NRF_CLOUD_EVT_SENSOR_DATA_ACK event.
	 */
	uint16_t tag;
};

/**@brief Asynchronous events received from the module. */
struct nrf_cloud_evt {
	/** The event that occurred. */
	enum nrf_cloud_evt_type type;
	/** Any status associated with the event. */
	uint32_t status;
	/** Received data. */
	struct nrf_cloud_data data;
	/** Topic on which data was received. */
	struct nrf_cloud_topic topic;
};

/**@brief Structure used to send pre-encoded data to nRF Cloud. */
struct nrf_cloud_tx_data {
	/** Data that is to be published. */
	struct nrf_cloud_data data;
	/** Endpoint topic type published to. */
	enum nrf_cloud_topic_type topic_type;
	/** Quality of Service of the message. */
	enum mqtt_qos qos;
};

/**@brief Controls which values are added to the FOTA array in the "serviceInfo" shadow section */
struct nrf_cloud_svc_info_fota {
	uint8_t bootloader:1;
	uint8_t modem:1;
	uint8_t application:1;

	uint8_t _rsvd:5;
};

/**@brief Controls which values are added to the UI array in the "serviceInfo" shadow section */
struct nrf_cloud_svc_info_ui {
	/** Items with UI support on nRF Cloud */
	uint8_t temperature:1;
	uint8_t gps:1;
	uint8_t flip:1; /* Orientation */
	uint8_t humidity:1;
	uint8_t air_pressure:1;
	uint8_t rsrp:1;

	/** Items without UI support on nRF Cloud */
	uint8_t air_quality:1;
	uint8_t light_sensor:1;
	uint8_t button:1;

	uint8_t _rsvd:7;
};

/**@brief How the info sections are handled when encoding shadow data */
enum nrf_cloud_shadow_info {
	/** Data will not be modified */
	NRF_CLOUD_INFO_NO_CHANGE = 0,
	/** Data will be set/updated */
	NRF_CLOUD_INFO_SET = 1,
	/** Data section will be cleared */
	NRF_CLOUD_INFO_CLEAR = 2,
};

/**@brief Modem info data and which sections should be encoded */
struct nrf_cloud_modem_info {
	enum nrf_cloud_shadow_info device;
	enum nrf_cloud_shadow_info network;
	enum nrf_cloud_shadow_info sim;

#if defined(CONFIG_MODEM_INFO)
	/** Pointer to a populated @ref modem_param_info struct.
	 * If NULL, modem data will be fetched.
	 */
	const struct modem_param_info *mpi;
#endif

};

/**@brief Structure to specify which components are added to the encoded service info object */
struct nrf_cloud_svc_info {
	/** Specify FOTA components to enable, set to NULL to remove the FOTA entry */
	struct nrf_cloud_svc_info_fota *fota;
	/** Specify UI components to enable, set to NULL to remove the UI entry */
	struct nrf_cloud_svc_info_ui *ui;
};

/**@brief Structure to specify which components are added to the encoded device status object */
struct nrf_cloud_device_status {
	/** Specify which modem info components to include, set to NULL to skip */
	struct nrf_cloud_modem_info *modem;
	/** Specify which service info components to include, set to NULL to skip */
	struct nrf_cloud_svc_info *svc;
};

#ifdef CONFIG_NRF_CLOUD_GATEWAY
/**@brief Structure to hold message received from nRF Cloud. */
struct nrf_cloud_gw_data {
	struct nrf_cloud_data data;
	uint16_t id;
};
#endif

/**
 * @brief  Event handler registered with the module to handle asynchronous
 * events from the module.
 *
 * @param[in]  evt The event and any associated parameters.
 */
typedef void (*nrf_cloud_event_handler_t)(const struct nrf_cloud_evt *evt);

/**@brief Initialization parameters for the module. */
struct nrf_cloud_init_param {
	/** Event handler that is registered with the module. */
	nrf_cloud_event_handler_t event_handler;
	/** NULL-terminated MQTT client ID string.
	 * Must not exceed NRF_CLOUD_CLIENT_ID_MAX_LEN.
	 * Must be set if NRF_CLOUD_CLIENT_ID_SRC_RUNTIME
	 * is enabled; otherwise, NULL.
	 */
	char *client_id;
};

/**
 * @brief Initialize the module.
 *
 * @warning This API must be called prior to using nRF Cloud
 *  and it must return successfully.
 *
 * @param[in] param Initialization parameters.
 *
 * @retval 0       If successful.
 * @retval -EACCES Already initialized or @ref nrf_cloud_uninit is in progress.
 *                 Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_init(const struct nrf_cloud_init_param *param);

/**
 * @brief Uninitialize nRF Cloud; disconnects cloud connection
 *  and cleans up allocated memory.
 *
 * @note If nRF Cloud FOTA is enabled and a FOTA job is active
 *  uninit will not be performed.
 *
 * @retval 0      If successful.
 * @retval -EBUSY If a FOTA job is in progress.
 *                Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_uninit(void);

/**
 * @brief Connect to the cloud.
 *
 * In any stage of connecting to the cloud, an @ref NRF_CLOUD_EVT_ERROR
 * might be received.
 * If it is received before @ref NRF_CLOUD_EVT_TRANSPORT_CONNECTED,
 * the application may repeat the call to @ref nrf_cloud_connect to try again.
 *
 * @param[in] param Parameters to be used for the connection.
 *
 * @retval Connect result defined by enum nrf_cloud_connect_result.
 */
int nrf_cloud_connect(const struct nrf_cloud_connect_param *param);

/**
 * @brief Send sensor data reliably.
 *
 * This API should only be called after receiving an
 * @ref NRF_CLOUD_EVT_READY event.
 * If the API succeeds, you can expect the
 * @ref NRF_CLOUD_EVT_SENSOR_DATA_ACK event for data sent with
 * a valid tag value.
 *
 * @param[in] param Sensor data; the data pointed to by param->data.ptr
 *                  must be a string.
 *
 * @retval 0       If successful.
 * @retval -EACCES Cloud connection is not established; wait for @ref NRF_CLOUD_EVT_READY.
 *                 Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_sensor_data_send(const struct nrf_cloud_sensor_data *param);

/**
 * @brief Update the device shadow with sensor data.
 *
 * @param[in] param Sensor data; the data pointed to by param->data.ptr must be a
 *                  valid JSON string.
 *
 * @retval 0       If successful.
 * @retval -EACCES Cloud connection is not established; wait for @ref NRF_CLOUD_EVT_READY.
 *                 Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_shadow_update(const struct nrf_cloud_sensor_data *param);

/**
 * @brief Update the device status in the shadow.
 *
 * @param[in] dev_status Device status to be encoded.
 *
 * @retval 0       If successful.
 * @retval -EACCES Cloud connection is not established; wait for @ref NRF_CLOUD_EVT_READY.
 *                 Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_shadow_device_status_update(const struct nrf_cloud_device_status * const dev_status);

/**
 * @brief Stream sensor data. Uses lowest QoS; no acknowledgment,
 *
 * This API should only be called after receiving an
 * @ref NRF_CLOUD_EVT_READY event.
 *
 * @param[in] param Sensor data; tag value is ignored.
 *
 * @retval 0       If successful.
 * @retval -EACCES Cloud connection is not established; wait for @ref NRF_CLOUD_EVT_READY.
 *                 Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_sensor_data_stream(const struct nrf_cloud_sensor_data *param);

/**
 * @brief Send data to nRF Cloud.
 *
 * This API is used to send pre-encoded data to nRF Cloud.
 *
 * @param[in] msg Pointer to a structure containting data and topic
 *                information.
 *
 * @retval 0       If successful.
 * @retval -EACCES Cloud connection is not established; wait for @ref NRF_CLOUD_EVT_READY.
 *                 Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_send(const struct nrf_cloud_tx_data *msg);

/**
 * @brief Disconnect from the cloud.
 *
 * This API may be called any time after receiving the
 * @ref NRF_CLOUD_EVT_TRANSPORT_CONNECTED event.
 * If the API succeeds, you can expect the
 * @ref NRF_CLOUD_EVT_TRANSPORT_DISCONNECTED event.
 *
 * @retval 0       If successful.
 * @retval -EACCES Cloud connection is not established; wait for
 *                 @ref NRF_CLOUD_EVT_TRANSPORT_CONNECTED.
 *                 Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_disconnect(void);

/**
 * @brief Function that must be called periodically to keep the module
 * functional.
 */
void nrf_cloud_process(void);

/**
 * @brief The application has handled re-init after a modem FOTA update and the
 *        LTE link has been re-established.
 *        This function must be called in order to complete the modem update.
 *
 * @param[in] fota_success true if modem update was successful, false otherwise.
 *
 * @retval 0 If successful.
 *           Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_modem_fota_completed(const bool fota_success);

/**
 * @brief Add service info into the provided cJSON object.
 *
 * @param[in]     svc_inf     Service info to add.
 * @param[in,out] svc_inf_obj cJSON object to which service info will be added.
 *
 * @retval 0 If successful.
 *           Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_service_info_json_encode(const struct nrf_cloud_svc_info * const svc_inf,
				       cJSON * const svc_inf_obj);

/**
 * @brief Add modem info into the provided cJSON object.
 *
 * @note To add modem info, CONFIG_MODEM_INFO must be enabled.
 *
 * @param[in]     mod_inf     Modem info to add.
 * @param[in,out] mod_inf_obj cJSON object to which modem info will be added.
 *
 * @retval 0 If successful.
 *           Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_modem_info_json_encode(const struct nrf_cloud_modem_info * const mod_inf,
				     cJSON * const mod_inf_obj);

/**
 * @brief Function to retrieve the current device ID.
 *
 * @param[in,out] id_buf Buffer to receive the device ID.
 * @param[in] id_len     Size of buffer (NRF_CLOUD_CLIENT_ID_MAX_LEN).
 *
 * @retval 0 If successful.
 *           Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_client_id_get(char *id_buf, size_t id_len);

/**
 * @brief Function to retrieve the current customer tenant ID.
 *
 * @param[in,out] id_buf Buffer to receive the tenant ID.
 * @param[in] id_len     Size of buffer (NRF_CLOUD_TENANT_ID_MAX_LEN).
 *
 * @retval 0 If successful.
 *           Otherwise, a (negative) error code is returned.
 */
int nrf_cloud_tenant_id_get(char *id_buf, size_t id_len);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NRF_CLOUD_H__ */
