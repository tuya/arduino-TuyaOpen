#include "WiFi.h"
#include "WiFiGeneric.h"
#include "WiFiSTA.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <lwip/ip_addr.h>
#include "lwip/err.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "tal_log.h"

bool WiFiSTAClass::_autoReconnect = true;
bool WiFiSTAClass::_useStaticIp = false;
static WF_STATION_STAT_E _sta_status = WSS_IDLE;
static EventGroupHandle_t _sta_status_group = NULL;

void WiFiSTAClass::_setStatus(WF_STATION_STAT_E status)
{
    if(!_sta_status_group){
        _sta_status_group = xEventGroupCreate();
        if(!_sta_status_group){
            PR_ERR("STA Status Group Create Failed!");
            _sta_status = status;
            return;
        }
    }
    xEventGroupClearBits(_sta_status_group, 0x00FFFFFF);
    xEventGroupSetBits(_sta_status_group, status);
}


WF_STATION_STAT_E WiFiSTAClass::status()
{
    OPERATE_RET rt = OPRT_OK;
    WF_STATION_STAT_E rtVal = WSS_IDLE;

    rt = tkl_wifi_station_get_status(&rtVal);
    if (rt != OPRT_OK) {
        return WSS_CONN_FAIL;
    }
    return rtVal ; 
}


WF_STATION_STAT_E WiFiSTAClass::begin(const char* wpa2_ssid, wpa2_auth_method_t method, const char* wpa2_identity, const char* wpa2_username, const char *wpa2_password, const char* ca_pem, const char* client_crt, const char* client_key, int32_t channel, const uint8_t* bssid, bool connect)
{
    return status();
}

WF_STATION_STAT_E WiFiSTAClass::begin(const char* ssid, const char *passphrase, int32_t channel, const uint8_t* bssid, bool connect)
{
    if(!WiFi.enableSTA(true)) {
        PR_ERR("STA enable failed!");
        return WSS_CONN_FAIL;
    }

    if(!ssid || *ssid == 0x00 || strlen(ssid) > 32) {
        PR_ERR("SSID too long or missing!");
        return WSS_CONN_FAIL;
    }

    if(passphrase && strlen(passphrase) > 64) {
        PR_ERR("passphrase too long!");
        return WSS_CONN_FAIL;
    }
    if(connect){
    	if( tkl_wifi_station_connect((const int8_t *)ssid,(const int8_t *)passphrase) != OPRT_OK) {
			PR_ERR("connect failed!");			
		}
    }
    return status();
}

WF_STATION_STAT_E WiFiSTAClass::begin(char* ssid, char *passphrase, int32_t channel, const uint8_t* bssid, bool connect)
{
    return begin((const char*) ssid, (const char*) passphrase, channel, bssid, connect);
}


WF_STATION_STAT_E WiFiSTAClass::begin()
{
    return status();
}


bool WiFiSTAClass::reconnect()
{
    return false;
}


bool WiFiSTAClass::disconnect(bool wifioff, bool eraseap)
{
    OPERATE_RET rt = OPRT_OK;
    WiFiMode_t workMode = WiFi.getMode();
    if (workMode == WIFI_AP) {
        rt = tkl_wifi_stop_ap();
    } else if (workMode == WIFI_STA) {
        rt = tkl_wifi_station_disconnect();
    } else if (workMode == WIFI_AP_STA) {
        rt = tkl_wifi_stop_ap();
        rt = tkl_wifi_station_disconnect();
    } else {
        return false;
    }
    return (rt == OPRT_OK) ? true : false;
}


bool WiFiSTAClass::eraseAP(void) {

    return OPRT_OK;
}


bool WiFiSTAClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
{
    NW_IP_S sta_ip;
    if(!WiFi.enableSTA(true)) {
        return false;
    }

    memset(&sta_ip, 0, sizeof(NW_IP_S));
    
    String ip_str = local_ip.toString();
    String gw_str = gateway.toString();
    String mask_str = subnet.toString();
    
    strncpy(sta_ip.ip, ip_str.c_str(), sizeof(sta_ip.ip) - 1);
    sta_ip.ip[sizeof(sta_ip.ip) - 1] = '\0';
    
    strncpy(sta_ip.gw, gw_str.c_str(), sizeof(sta_ip.gw) - 1);
    sta_ip.gw[sizeof(sta_ip.gw) - 1] = '\0';
    
    strncpy(sta_ip.mask, mask_str.c_str(), sizeof(sta_ip.mask) - 1);
    sta_ip.mask[sizeof(sta_ip.mask) - 1] = '\0';
    
    int res = tkl_wifi_set_ip(WF_STATION, &sta_ip);
    
    return (res == OPRT_OK);
}

bool WiFiSTAClass::isConnected()
{
    return (status() == WSS_GOT_IP);
}

bool WiFiSTAClass::setAutoConnect(bool autoConnect)
{
    return false;//now deprecated
}

bool WiFiSTAClass::getAutoConnect()
{
    return false;//now deprecated
}

bool WiFiSTAClass::setAutoReconnect(bool autoReconnect)
{
    _autoReconnect = autoReconnect;
    return true;
}

bool WiFiSTAClass::getAutoReconnect()
{
    return _autoReconnect;
}

uint8_t WiFiSTAClass::waitForConnectResult(unsigned long timeoutLength)
{
    if((WiFiGenericClass::getMode() & WF_STATION) == 0) {
        return WSS_CONN_FAIL;
    }
    return status();
}

IPAddress WiFiSTAClass::localIP()
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip_info;
    if(tkl_wifi_get_ip(WF_STATION,&ip_info) != OPRT_OK){
        PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }
    return IPAddress(ip_info.ip);
}

uint8_t* WiFiSTAClass::macAddress(uint8_t* mac)
{
    tkl_wifi_get_mac(WF_STATION,(NW_MAC_S*)mac);
    return mac;
}

String WiFiSTAClass::macAddress(void)
{
    uint8_t mac[6] = { 0 };
    tkl_wifi_get_mac(WF_STATION,(NW_MAC_S*)mac);
    char macStr[18] = { 0 };
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

IPAddress WiFiSTAClass::subnetMask()
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip_info;
    if(tkl_wifi_get_ip(WF_STATION,&ip_info) != OPRT_OK){
    	return IPAddress();
    }
    return IPAddress(ip_info.mask);
}

IPAddress WiFiSTAClass::gatewayIP()
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip_info;
    if(tkl_wifi_get_ip(WF_STATION,&ip_info) != OPRT_OK){
        PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }
    return IPAddress(ip_info.gw);
}


IPAddress WiFiSTAClass::dnsIP(uint8_t dns_no)
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    const ip_addr_t* p_dns = dns_getserver(dns_no);

    char dns_str[40];

    memset(dns_str, 0, sizeof(dns_str));

    ipaddr_ntoa_r(p_dns, dns_str, sizeof(dns_str));

    return IPAddress(dns_str);
}

IPAddress WiFiSTAClass::broadcastIP()
{
    
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip_info;
    if(tkl_wifi_get_ip(WF_STATION,&ip_info) != OPRT_OK){
        PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }

    return WiFiGenericClass::calculateBroadcast(IPAddress(ip_info.gw), IPAddress(ip_info.mask));
}

IPAddress WiFiSTAClass::networkID()
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip_info;
    if(tkl_wifi_get_ip(WF_STATION,&ip_info) != OPRT_OK){
        PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }

    return WiFiGenericClass::calculateNetworkID(IPAddress(ip_info.gw), IPAddress(ip_info.mask));
}


uint8_t WiFiSTAClass::subnetCIDR()
{

    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip_info;
    if(tkl_wifi_get_ip(WF_STATION,&ip_info) != OPRT_OK){
        PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }

    return WiFiGenericClass::calculateSubnetCIDR(IPAddress(ip_info.mask));
}


String WiFiSTAClass::SSID() const
{

    return String();
}


String WiFiSTAClass::psk() const
{
    
    return String();
}


uint8_t* WiFiSTAClass::BSSID(void)
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return NULL;
    }
    static uint8_t bssid[6];
    tkl_wifi_get_bssid((uint8_t*)&bssid);
    return bssid;
}


String WiFiSTAClass::BSSIDstr(void)
{
    uint8_t* bssid = BSSID();
    if(!bssid){
        return String();
    }
    char mac[18] = { 0 };
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    return String(mac);
}


int8_t WiFiSTAClass::RSSI(void)
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return 0;
    }
    int8_t rssi;
    tkl_wifi_station_get_conn_ap_rssi(&rssi);
    return rssi;
}

