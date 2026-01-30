#include "Arduino.h"
#include "WiFi.h"
#include "WiFiGeneric.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include "lwip/ip_addr.h"
#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include <vector>
#include "lwip_netif_address.h"
#include "net.h"
#include "tal_api.h"
#include "tkl_wifi.h"
#include "tal_wifi.h"

#define QUEUE_WAIT_FROEVER 0xFFFFFFFF

typedef struct WiFiEventCbList {
    static wifi_event_id_t current_id;
    wifi_event_id_t id;
    WiFiEventCb cb;
    WiFiEventFuncCb fcb;
    WiFiEventSysCb scb;
    arduino_event_id_t event;

    WiFiEventCbList() : id(current_id++), cb(NULL), fcb(NULL), scb(NULL), event(ARDUINO_EVENT_WIFI_READY) {}
} WiFiEventCbList_t;

static bool _bk_wifi_started = false;
static QUEUE_HANDLE _arduino_event_queue = NULL;
static THREAD_HANDLE _arduino_event_task_handle = NULL;
static EventGroupHandle_t _arduino_event_group = NULL;
static bool lowLevelInitDone = false;
static std::vector<WiFiEventCbList_t> cbEventList;

bool WiFiGenericClass::_wifiUseStaticBuffers = false;
bool WiFiGenericClass::_persistent = true;
bool WiFiGenericClass::_long_range = false;

wifi_event_id_t WiFiEventCbList::current_id = 1;

static void _arduino_event_task(void * arg)
{
    arduino_event_t data ;
    for (;;) {
        if(tal_queue_fetch(_arduino_event_queue, &data,QUEUE_WAIT_FROEVER) == OPRT_OK){
            WiFiGenericClass::_eventCallback(&data);
        }
    }
    tal_thread_delete(NULL);
    _arduino_event_task_handle = NULL;

}

int postArduinoEvent(arduino_event_t *data)
{
    if (data == NULL) {
        return OPRT_COM_ERROR;
    }

    int ret = tal_queue_post(_arduino_event_queue, data, 0) ;
    if (ret != OPRT_OK) {
        PR_ERR("Arduino Event Send Failed!\r\n");
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

static void __netconn_wifi_event(WF_EVENT_E event, void *arg)
{
    arduino_event_t arduino_event;
	arduino_event.event_id = ARDUINO_EVENT_MAX;

    if (event == WFE_CONNECTED) {
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_GOT_IP;
        PR_INFO("ARDUINO_EVENT_WIFI_STA_GOT_IP\n");
        NW_IP_S ip;
        tkl_wifi_get_ip(WF_STATION,&ip);
        memcpy(&arduino_event.event_info.got_ip.ip_info.ip,ip.ip,16);
        memcpy(&arduino_event.event_info.got_ip.ip_info.gw,ip.gw,16);
        memcpy(&arduino_event.event_info.got_ip.ip_info.netmask,ip.mask,16);
    } 
    else if (event == WFE_CONNECT_FAILED)
    {   
        PR_INFO("EVT_CONNECT_FAILED\r\n");
    }
    else
    {  
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_DISCONNECTED;
        PR_INFO("EVT_STA_DISCONNECTED\r\n");
    }

    if(arduino_event.event_id < ARDUINO_EVENT_MAX){
		postArduinoEvent(&arduino_event);
	}
    return;
}

static bool _start_network_event_task()
{
    if(!_arduino_event_group){
        _arduino_event_group = xEventGroupCreate();
        if(!_arduino_event_group){
            PR_ERR("Network Event Group Create Failed!");
            return false;
        }
        xEventGroupSetBits(_arduino_event_group, WIFI_DNS_IDLE_BIT);
    }
    if(!_arduino_event_queue){
        tal_queue_create_init(&_arduino_event_queue,sizeof(arduino_event_t),32);
        if(!_arduino_event_queue){
            PR_ERR("Network Event Queue Create Failed!");
            return false;
        }
    }

    tkl_wifi_init(__netconn_wifi_event);

    THREAD_CFG_T param;
    param.priority = THREAD_PRIO_3;
    param.stackDepth = 4096;
    char threadName[] = "arduino_events";
    param.thrdname =  threadName;

    if(!_arduino_event_task_handle)
    {
        tal_thread_create_and_start( &_arduino_event_task_handle, NULL, NULL, _arduino_event_task, NULL, &param);
        if(!_arduino_event_task_handle){
            PR_ERR("Network Event Task Start Failed!");
            return false;
        }
    }
    return true;
}

bool tcpipInit(){
    static bool initialized = false;
    if(!initialized){
        initialized = true;
        
        if(initialized){
            initialized = _start_network_event_task();
        } else {
            PR_ERR("netif_init failed!\r\n");
        }
    }
    return initialized;
}


bool WiFiGenericClass::useStaticBuffers(){
    return _wifiUseStaticBuffers;
}

void WiFiGenericClass::useStaticBuffers(bool bufferMode){
    if (lowLevelInitDone) {
        PR_INFO("WiFi already started. Call WiFi.mode(WIFI_MODE_NULL) before setting Static Buffer Mode.\r\n");
    } 
    _wifiUseStaticBuffers = bufferMode;
}

bool wifiLowLevelInit(bool persistent){
    if(!lowLevelInitDone){
        lowLevelInitDone = true;
        if(!tcpipInit()){
        	lowLevelInitDone = false;
            PR_ERR("FALSE  lowLevelInitDone:%d \n",lowLevelInitDone);
        	return lowLevelInitDone;
        }
        
        if(lowLevelInitDone){
			arduino_event_t arduino_event;
			arduino_event.event_id = ARDUINO_EVENT_WIFI_READY;
			postArduinoEvent(&arduino_event);
        }
    }
    return lowLevelInitDone;
}

static bool wifiLowLevelDeinit(){
    
    if(lowLevelInitDone){
    	lowLevelInitDone = !true;
    }
    return !lowLevelInitDone;
}


static bool bkWiFiStart(){

    if(_bk_wifi_started){
        return true;
    }
    _bk_wifi_started = true;
    return _bk_wifi_started;

}

static bool bkWiFiStop(){
    
    if(!_bk_wifi_started){
        return true;
    }
    _bk_wifi_started = false;
    tkl_wifi_stop_ap();
    tkl_wifi_station_disconnect() ;
    tkl_wifi_set_work_mode(WWM_POWERDOWN);
    return wifiLowLevelDeinit();
}


int WiFiGenericClass::setStatusBits(int bits){
    if(!_arduino_event_group){
        return 0;
    }
    return xEventGroupSetBits(_arduino_event_group, bits);
}

int WiFiGenericClass::clearStatusBits(int bits){
    if(!_arduino_event_group){
        return 0;
    }
    return xEventGroupClearBits(_arduino_event_group, bits);
}

int WiFiGenericClass::getStatusBits(){
    if(!_arduino_event_group){
        return 0;
    }
    return xEventGroupGetBits(_arduino_event_group);
}

int WiFiGenericClass::waitStatusBits(int bits, uint32_t timeout_ms){
    if(!_arduino_event_group){
        return 0;
    }
    return xEventGroupWaitBits(_arduino_event_group, bits, pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS ) & bits;
}

wifi_event_id_t WiFiGenericClass::onEvent(WiFiEventCb cbEvent, arduino_event_id_t event)
{
    if(!cbEvent) {
        return 0;
    }
    WiFiEventCbList_t newEventHandler;
    newEventHandler.cb = cbEvent;
    newEventHandler.fcb = NULL;
    newEventHandler.scb = NULL;
    newEventHandler.event = event;
    cbEventList.push_back(newEventHandler);
    return newEventHandler.id;
}

wifi_event_id_t WiFiGenericClass::onEvent(WiFiEventFuncCb cbEvent, arduino_event_id_t event)
{
    if(!cbEvent) {
        return 0;
    }
    WiFiEventCbList_t newEventHandler;
    newEventHandler.cb = NULL;
    newEventHandler.fcb = cbEvent;
    newEventHandler.scb = NULL;
    newEventHandler.event = event;
    cbEventList.push_back(newEventHandler);
    return newEventHandler.id;
}

wifi_event_id_t WiFiGenericClass::onEvent(WiFiEventSysCb cbEvent, arduino_event_id_t event)
{
    if(!cbEvent) {
        return 0;
    }
    WiFiEventCbList_t newEventHandler;
    newEventHandler.cb = NULL;
    newEventHandler.fcb = NULL;
    newEventHandler.scb = cbEvent;
    newEventHandler.event = event;
    cbEventList.push_back(newEventHandler);
    return newEventHandler.id;
}

void WiFiGenericClass::removeEvent(WiFiEventCb cbEvent, arduino_event_id_t event)
{
    if(!cbEvent) {
        return;
    }

    for(uint32_t i = 0; i < cbEventList.size(); i++) {
        WiFiEventCbList_t entry = cbEventList[i];
        if(entry.cb == cbEvent && entry.event == event) {
            cbEventList.erase(cbEventList.begin() + i);
        }
    }
}

void WiFiGenericClass::removeEvent(WiFiEventSysCb cbEvent, arduino_event_id_t event)
{
    if(!cbEvent) {
        return;
    }

    for(uint32_t i = 0; i < cbEventList.size(); i++) {
        WiFiEventCbList_t entry = cbEventList[i];
        if(entry.scb == cbEvent && entry.event == event) {
            cbEventList.erase(cbEventList.begin() + i);
        }
    }
}

void WiFiGenericClass::removeEvent(wifi_event_id_t id)
{
    for(uint32_t i = 0; i < cbEventList.size(); i++) {
        WiFiEventCbList_t entry = cbEventList[i];
        if(entry.id == id) {
            cbEventList.erase(cbEventList.begin() + i);
        }
    }
}

int WiFiGenericClass::_eventCallback(arduino_event_t *event)
{
    if(!event) return OPRT_OK;        
    if(event->event_id == ARDUINO_EVENT_WIFI_STA_GOT_IP  ) {
        WiFiSTAClass::_setStatus(WSS_GOT_IP);
        setStatusBits(STA_CONNECTED_BIT);

    } else if(event->event_id == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
        WiFiSTAClass::_setStatus(WSS_CONN_FAIL);
    }

    for(uint32_t i = 0; i < cbEventList.size(); i++) {
    WiFiEventCbList_t entry = cbEventList[i];
    if(entry.cb || entry.fcb || entry.scb) {
            if(entry.event == (arduino_event_id_t) event->event_id || entry.event == ARDUINO_EVENT_MAX) {
                if(entry.cb) {
                    entry.cb((arduino_event_id_t) event->event_id);
                } else if(entry.fcb) {
                    entry.fcb((arduino_event_id_t) event->event_id, (arduino_event_info_t) event->event_info);
                } else {
                    entry.scb(event);
                }
            }
        } 
    }                     
    return OPRT_OK;
}

int32_t WiFiGenericClass::channel(void)
{
    if(!lowLevelInitDone){
        return 0;
    }
    uint8_t channel;
    tkl_wifi_get_cur_channel(&channel);
    return channel;
}


void WiFiGenericClass::persistent(bool persistent)
{
    _persistent = persistent;
}


void WiFiGenericClass::enableLongRange(bool enable)
{
    _long_range = enable;
}

bool WiFiGenericClass::mode(WiFiMode_t m)
{
    WF_WK_MD_E cm = getMode();
    int err;
    if(cm == m) {
        return true;
    }
    if(!cm && m)
    {
        if(!wifiLowLevelInit(_persistent)){
            return false;
        }
    } 
    else if(cm && !m){
        PR_INFO("bkWiFiStop return\n");
        return bkWiFiStop();
    }
    err = tkl_wifi_set_work_mode(m);
    if(err){
        PR_ERR("Could not set mode! %d", err);
        return false;
    }
    if(!bkWiFiStart()){
        return false;
    }
    return true;
}

WF_WK_MD_E WiFiGenericClass::getMode()
{
    // if(!lowLevelInitDone || !_bk_wifi_started){
    //     return WWM_POWERDOWN; 
    // }
    WF_WK_MD_E mode;
    if(tkl_wifi_get_work_mode(&mode) != 0){
        PR_ERR("WiFi not started");
        return WWM_POWERDOWN;
    }
    return mode;
}

bool WiFiGenericClass::enableSTA(bool enable)
{
    WF_WK_MD_E currentMode = getMode();
    bool isEnabled = ((currentMode & WWM_STATION) != 0);
    if(isEnabled != enable) {
        if(enable) {
            return mode((WF_WK_MD_E)(currentMode | WWM_STATION));
        }
        return mode((WF_WK_MD_E)(currentMode & (~WWM_STATION)));
    }
    return true;
}

bool WiFiGenericClass::enableAP(bool enable)
{
    WF_WK_MD_E currentMode = getMode();
    bool isEnabled = ((currentMode & WWM_SOFTAP) != 0);
    if(isEnabled != enable) {
        if(enable) {
            return mode((WF_WK_MD_E)(currentMode | WWM_SOFTAP));
        }
        return mode((WF_WK_MD_E)(currentMode & (~WWM_SOFTAP)));
    }
    return true;
}

bool WiFiGenericClass::setSleep(bool enabled){
    if(enabled)
        return tal_wifi_lp_enable();
    else 
        return tal_wifi_lp_disable();
}

bool WiFiGenericClass::setDualAntennaConfig(uint8_t gpio_ant1, uint8_t gpio_ant2, wifi_rx_ant_t rx_mode, wifi_tx_ant_t tx_mode)
{
    return false;
}

static void wifi_dns_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    if(ipaddr) {
        (*reinterpret_cast<IPAddress*>(callback_arg)) = ipaddr->addr;
    }
    xEventGroupSetBits(_arduino_event_group, WIFI_DNS_DONE_BIT);
}

int WiFiGenericClass::hostByName(const char* aHostname, IPAddress& aResult)
{

    if (!aResult.fromString(aHostname))
    {
        ip_addr_t addr;
        aResult = static_cast<uint32_t>(0);
        waitStatusBits(WIFI_DNS_IDLE_BIT, 16000);
        clearStatusBits(WIFI_DNS_IDLE_BIT | WIFI_DNS_DONE_BIT);
        err_t err = dns_gethostbyname(aHostname, &addr, &wifi_dns_found_callback, &aResult);
        if(err == ERR_OK && addr.addr) {
            aResult = addr.addr;
        } else if(err == ERR_INPROGRESS) {
            waitStatusBits(WIFI_DNS_DONE_BIT, 15000);  
            clearStatusBits(WIFI_DNS_DONE_BIT);
        }
        setStatusBits(WIFI_DNS_IDLE_BIT);
        if((uint32_t)aResult == 0){
            PR_ERR("DNS Failed for %s\r\n", aHostname);
        }
    }

    return (uint32_t)aResult != 0;
}

IPAddress WiFiGenericClass::calculateNetworkID(IPAddress ip, IPAddress subnet) {
	IPAddress networkID;

	for (size_t i = 0; i < 4; i++)
		networkID[i] = subnet[i] & ip[i];

	return networkID;
}

IPAddress WiFiGenericClass::calculateBroadcast(IPAddress ip, IPAddress subnet) {
    IPAddress broadcastIp;
    
    for (int i = 0; i < 4; i++)
        broadcastIp[i] = ~subnet[i] | ip[i];

    return broadcastIp;
}

uint8_t WiFiGenericClass::calculateSubnetCIDR(IPAddress subnetMask) {
	uint8_t CIDR = 0;

	for (uint8_t i = 0; i < 4; i++) {
		if (subnetMask[i] == 0x80)  // 128
			CIDR += 1;
		else if (subnetMask[i] == 0xC0)  // 192
			CIDR += 2;
		else if (subnetMask[i] == 0xE0)  // 224
			CIDR += 3;
		else if (subnetMask[i] == 0xF0)  // 242
			CIDR += 4;
		else if (subnetMask[i] == 0xF8)  // 248
			CIDR += 5;
		else if (subnetMask[i] == 0xFC)  // 252
			CIDR += 6;
		else if (subnetMask[i] == 0xFE)  // 254
			CIDR += 7;
		else if (subnetMask[i] == 0xFF)  // 255
			CIDR += 8;
	}

	return CIDR;
}
