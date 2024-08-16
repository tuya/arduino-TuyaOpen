#include "WiFi.h"
#include "WiFiGeneric.h"
#include "WiFiAP.h"
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
#include "lwip/dns.h"
#include "lwip/net.h"
#include "lwip/netif.h"
}

bool WiFiAPClass::softAP(const char* ssid, const char* passphrase, int channel, int ssid_hidden, int max_connection, bool ftm_responder)
{

    if(!ssid || *ssid == 0) {
        PR_ERR("SSID missing!\r\n");
        return false;
    }
    if(passphrase && (strlen(passphrase) > 0 && strlen(passphrase) < 8)) {
        PR_ERR("passphrase too short!\r\n");
        return false;
    }

    if(!WiFi.enableAP(true)) {
        PR_ERR("enable AP first!\r\n");
        return false;
    }

    ap_cfg_info.chan = channel;
    ap_cfg_info.ssid_hidden = ssid_hidden;
    ap_cfg_info.max_conn = max_connection;
    ap_cfg_info.md = WAAM_WPA2_PSK;
    ap_cfg_info.ms_interval =  100;

    ap_cfg_info.s_len = strlen(ssid);
    strcpy((char *)ap_cfg_info.ssid ,ssid);
    ap_cfg_info.p_len = strlen(passphrase);
    strcpy((char *)ap_cfg_info.passwd,passphrase);

    if(!ap_cfg_info.ip.ip[0])
    {
        strcpy((char *)ap_cfg_info.ip.ip, WLAN_DEFAULT_IP);
        strcpy((char *)ap_cfg_info.ip.gw, WLAN_DEFAULT_GW);
        strcpy((char *)ap_cfg_info.ip.mask, WLAN_DEFAULT_MASK);
    }
    
    int ret = tkl_wifi_start_ap(&ap_cfg_info);
    return (ret == OPRT_OK) ? true : false;
    
}

String WiFiAPClass::softAPSSID() const
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return String();
    }
    return  String((char *)ap_cfg_info.ssid);
}

bool WiFiAPClass::softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dhcp_lease_start)
{
    OPERATE_RET err = OPRT_OK;
    if(!WiFi.enableAP(true)) {
        // enable AP failed
        return false;
    }
    strcpy((char *)ap_cfg_info.ip.ip, local_ip.toString().c_str());
    strcpy((char *)ap_cfg_info.ip.gw, gateway.toString().c_str());
    strcpy((char *)ap_cfg_info.ip.mask, subnet.toString().c_str());

    PR_INFO("ip:%s,Gateway: %s,Netmask: %s,dns:%s\r\n", local_ip.toString().c_str(), gateway.toString().c_str(), subnet.toString().c_str());
    return  err == OPRT_OK; 
}

bool WiFiAPClass::softAPdisconnect(bool wifioff)
{
    bool ret = 0;
    if(wifioff) {
        ret = WiFi.enableAP(false) == OPRT_OK;
    }
    ret = tkl_wifi_stop_ap();
    return ret;
}

uint8_t WiFiAPClass::softAPgetStationNum()
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return 0;
    }
    sta_info = NULL;
    sta_num = 0;
    if( tkl_wifi_get_all_sta_info(&sta_info,&sta_num)== OPRT_OK) {
        return sta_num;
    }
    return 0;
}

IPAddress WiFiAPClass::softAPIP()
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip;
    if( tkl_wifi_get_ip(WF_AP,&ip)!= OPRT_OK){
    	PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }
    return IPAddress(ip.ip);
}

IPAddress WiFiAPClass::softAPBroadcastIP()
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip;
    if(tkl_wifi_get_ip(WF_AP,&ip) != OPRT_OK){
    	PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }
    return WiFiGenericClass::calculateBroadcast(IPAddress(ip.gw), IPAddress(ip.mask));

}

IPAddress WiFiAPClass::softAPNetworkID()
{
   if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip;
    if(tkl_wifi_get_ip(WF_AP,&ip) != OPRT_OK){
    	PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }
    return WiFiGenericClass::calculateNetworkID(IPAddress(ip.gw), IPAddress(ip.mask));
}

IPAddress WiFiAPClass::softAPSubnetMask()
{
   if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return IPAddress();
    }
    NW_IP_S ip;
    if(tkl_wifi_get_ip(WF_AP,&ip) != OPRT_OK){
    	PR_ERR("Netif Get IP Failed!\r\n");
    	return IPAddress();
    }
    return IPAddress(ip.mask);
}

uint8_t WiFiAPClass::softAPSubnetCIDR()
{
    return WiFiGenericClass::calculateSubnetCIDR(softAPSubnetMask());
}

uint8_t* WiFiAPClass::softAPmacAddress(uint8_t* mac)
{
    if(WiFiGenericClass::getMode() != WWM_POWERDOWN){
        tkl_wifi_get_mac(WF_AP,(NW_MAC_S*)mac);
    }
    return mac;
}

String WiFiAPClass::softAPmacAddress(void)
{
    char macStr[18] = { 0 };
    uint8_t mac[6];
    
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return String();
    }

    softAPmacAddress(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return String(macStr);
}

const char * WiFiAPClass::softAPgetHostname()
{
    const char * hostname = NULL;
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return hostname;
    }
    hostname = (char *)netif_get_hostname((struct netif *)net_get_uap_handle());
    return hostname;
}

bool WiFiAPClass::softAPsetHostname(const char * hostname)
{
    if(WiFiGenericClass::getMode() == WWM_POWERDOWN){
        return false;
    }
    static char net_hostname[32] = {0};
    memset(net_hostname,0,sizeof(net_hostname));
    strcpy(net_hostname,hostname);
    netif_set_hostname((struct netif *)net_get_uap_handle(),net_hostname);
    return true;
}

