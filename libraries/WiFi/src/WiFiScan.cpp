#include <Arduino.h>
#include "WiFi.h"
#include "WiFiGeneric.h"
#include "WiFiScan.h"
#include "tal_log.h"

extern "C" {
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <lwip/ip_addr.h>
#include "lwip/err.h"
}

int16_t WiFiScanClass::scanNetworks(bool async, bool show_hidden, bool passive, uint32_t max_ms_per_chan, uint8_t channel, const char * ssid, const uint8_t * bssid)
{
    ap_info = NULL;
    ap_num = 0;
    tkl_wifi_scan_ap(NULL,&ap_info,&ap_num);
    return ap_num;
}

void WiFiScanClass::scanDelete()
{
    if( ap_info != NULL)
        tkl_wifi_release_ap(ap_info);
    ap_num = 0;
    PR_INFO("scanDelete Done!\n"); 
}

void * WiFiScanClass::_getScanInfoByIndex(uint32_t i)
{
    if (i >= ap_num || !ap_info)
    {
        return 0;
    }
    return (AP_IF_S*)(ap_info + i);
}

bool WiFiScanClass::getNetworkInfo(uint32_t i, String &ssid, uint8_t &encType, int32_t &rssi, uint8_t* &bssid, int32_t &channel)
{
    AP_IF_S* it = reinterpret_cast<AP_IF_S*>(_getScanInfoByIndex(i));
    if(!it) {
        return false;
    }
    ssid = (const char*) it->ssid;
    encType = it->security;
    rssi = it->rssi;
    bssid = it->bssid;
    channel = it->channel;
    return true;
}

String WiFiScanClass::SSID(uint8_t i)
{
    AP_IF_S* it = reinterpret_cast<AP_IF_S*>(_getScanInfoByIndex(i));
    if(!it) {
        return String();
    } 
    return String(reinterpret_cast<const char*>(it->ssid));
}

WF_AP_AUTH_MODE_E WiFiScanClass::encryptionType(uint8_t i)
{
    AP_IF_S* it = reinterpret_cast<AP_IF_S*>(_getScanInfoByIndex(i));
    if(!it) {
        return WAAM_OPEN;
    }
    return (WF_AP_AUTH_MODE_E)it->security;
}

int32_t WiFiScanClass::RSSI(uint8_t i)
{
    AP_IF_S* it = reinterpret_cast<AP_IF_S*>(_getScanInfoByIndex(i));
    if(!it) {
        return 0;
    }
    return (int32_t)it->rssi;
}

uint8_t * WiFiScanClass::BSSID(uint8_t i)
{
    AP_IF_S* it = reinterpret_cast<AP_IF_S*>(_getScanInfoByIndex(i));
    if(!it) {
        return 0;
    }
    return it->bssid; 
}

String WiFiScanClass::BSSIDstr(uint8_t i)
{
    char mac[18] = { 0 };
    AP_IF_S* it = reinterpret_cast<AP_IF_S*>(_getScanInfoByIndex(i));
    if(!it) {
        return String();
    }
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", it->bssid[0], it->bssid[1], it->bssid[2], it->bssid[3], it->bssid[4], it->bssid[5]);
    return String(mac);
}

int32_t WiFiScanClass::channel(uint8_t i)
{
    AP_IF_S* it = reinterpret_cast<AP_IF_S*>(_getScanInfoByIndex(i));
    if(!it) {
        return 0;
    }
    return it->channel;   
}

