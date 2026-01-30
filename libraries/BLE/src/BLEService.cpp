#include "BLEService.h"
#include "BLEServer.h"
#include "BLECharacteristic.h"
#include <string.h>
#include "tal_api.h"

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

int hexstr2bin(const char *hex, uint8_t *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	uint8_t *opos = buf;

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}

TKL_BLE_GATTS_PARAMS_T* BLEService:: ble_gatt_service = NULL;
TKL_BLE_SERVICE_PARAMS_T* BLEService:: ble_service = NULL;
TKL_BLE_CHAR_PARAMS_T* BLEService :: ble_service_char = NULL;
uint8_t BLEService::char_index ;

BLEService::BLEService(std::string uuid) 
{
	char_index = 0;
			
    if (!ble_gatt_service) {
        ble_gatt_service = (TKL_BLE_GATTS_PARAMS_T*)tal_malloc(sizeof(TKL_BLE_GATTS_PARAMS_T));
    }
    if (!ble_service) {
        ble_service = (TKL_BLE_SERVICE_PARAMS_T*)tal_malloc(sizeof(TKL_BLE_SERVICE_PARAMS_T));
    }
    if (!ble_service_char) {
        ble_service_char = (TKL_BLE_CHAR_PARAMS_T *)tal_malloc(sizeof(TKL_BLE_CHAR_PARAMS_T) * TKL_BLE_GATT_CHAR_MAX_NUM);
    }
	if (!ble_gatt_service || !ble_service || !ble_service_char) {
		PR_ERR("malloc ble service failed");
		return;
	}

	ble_gatt_service->svc_num  = 1 ;
    ble_gatt_service->p_service = ble_service;
    ble_service->handle = TKL_BLE_GATT_INVALID_HANDLE;

	if( uuid.length() == 4) 
    {
        ble_service->svc_uuid.uuid_type = TKL_BLE_UUID_TYPE_16;
        hexstr2bin(uuid.data(),(uint8_t*)&ble_service->svc_uuid.uuid.uuid16,uuid.length()/2);
    }
	else if( uuid.length() == 8) 
    {
        ble_service->svc_uuid.uuid_type = TKL_BLE_UUID_TYPE_32;
        hexstr2bin(uuid.data(),(uint8_t*)&ble_service->svc_uuid.uuid.uuid32,uuid.length()/2);
    }
    else if( uuid.length() == 32) 
    {
        ble_service->svc_uuid.uuid_type = TKL_BLE_UUID_TYPE_128;
        hexstr2bin(uuid.data(),(uint8_t*)&ble_service->svc_uuid.uuid.uuid128,uuid.length()/2);
    }
	ble_service->type = TKL_BLE_UUID_SERVICE_PRIMARY;
    ble_service->char_num  = TKL_BLE_GATT_CHAR_MAX_NUM,
    ble_service->p_char     = ble_service_char;
} 

BLECharacteristic* BLEService::createCharacteristic(const char* uuid, uint32_t properties) 
{
	BLECharacteristic* pCharacteristic = new BLECharacteristic(uuid, properties);
	return pCharacteristic;
} 

void BLEService::start() 
{
 	tkl_ble_gatts_service_add(ble_gatt_service);
    tkl_ble_stack_init(TKL_BLE_ROLE_SERVER);
}
