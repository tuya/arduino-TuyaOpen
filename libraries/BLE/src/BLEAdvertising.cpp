#include "BLEAdvertising.h"
#include "tal_api.h"

void BLEAdvertising::setMinInterval(uint16_t mininterval) 
{
	advParams.adv_interval_min = mininterval;
}

void BLEAdvertising::setMaxInterval(uint16_t maxinterval) 
{
	advParams.adv_interval_max = maxinterval;
}

void BLEAdvertising::setScanResponse(bool set) 
{
	scanResp = set;
}

void BLEAdvertising::setAdvertisementData(uint8_t *adv_data, uint8_t len) 
{
	TKL_BLE_DATA_T p_adv;
	p_adv.p_data = (uint8_t *)tal_malloc(sizeof(uint8_t)*len);
	if (NULL == p_adv.p_data) {
		PR_ERR("malloc adv data failed");
		return;
	}
	memcpy(p_adv.p_data, adv_data, len);
	p_adv.length = len;
	tkl_ble_gap_adv_rsp_data_set(&p_adv, NULL);	
}

void BLEAdvertising::setScanResponseData(uint8_t *adv_rsp_data,uint8_t len) 
{
	TKL_BLE_DATA_T p_scan_rsp;
	p_scan_rsp.p_data = (uint8_t *)tal_malloc(sizeof(uint8_t)*len);
	if (NULL == p_scan_rsp.p_data) {
		PR_ERR("malloc scan rsp data failed");
		return;
	}
	memcpy(p_scan_rsp.p_data, adv_rsp_data, len);
	p_scan_rsp.length = len;
	tkl_ble_gap_adv_rsp_data_set(NULL, &p_scan_rsp);	
} 

void BLEAdvertising::start() 
{
	advParams.adv_type = TKL_BLE_GAP_ADV_TYPE_CONN_SCANNABLE_UNDIRECTED;
	advParams.adv_channel_map =  0x01 | 0x02 | 0x04;
	tkl_ble_gap_adv_start(&advParams);
} 

void BLEAdvertising::stop() 
{
	tkl_ble_gap_adv_stop();
} 
