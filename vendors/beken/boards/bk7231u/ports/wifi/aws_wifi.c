/*
 * Amazon FreeRTOS Wi-Fi V1.0.0
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file aws_wifi.c
 * @brief Wi-Fi Interface.
 */

/* Socket and Wi-Fi interface includes. */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "aws_wifi.h"

/* Wi-Fi configuration includes. */
#include "aws_wifi_config.h"

#include "wlan_ui_pub.h"
#include "mem_pub.h"
#include "str_pub.h"
#include "param_config.h"
#include "ieee802_11_demo.h"
#include "net.h"
#include <lwip/netdb.h>

#define BEKEN_CONNECTED_BIT        0x00000001
#define BEKEN_DISCONNECTED_BIT  0x00000002
#define BEKEN_STARTED_BIT            0x00000004

struct aws_wifi_beken
{
    uint8_t initialized;
    WIFIDeviceMode_t device_mode;
    WIFIPMMode_t pm_mode;
    bool wifi_conn_state;

    network_InitTypeDef_st ap_config;

    beken_semaphore_t scan_done_sem;
    beken_mutex_t       wifi_lock;
    EventGroupHandle_t wifi_event_group;
};

static struct aws_wifi_beken g_aws_wifi_beken;

extern int bk_wlan_dtim_rf_ps_timer_start(void);
extern int bk_wlan_dtim_rf_ps_timer_pause(void);
extern OSStatus ping(struct in_addr *target_addr, uint32_t times, size_t size, uint32_t interval);

static void beken_scan_ap_callback(void *ctxt, uint8_t param)
{
    if (g_aws_wifi_beken.scan_done_sem)
    {
        rtos_set_semaphore(&g_aws_wifi_beken.scan_done_sem);
        os_printf("%s:%d done\n", __FUNCTION__, __LINE__);
    }
}

static int beken_rw_event_callback(enum rw_evt_type evt_type, void *data)
{
    //struct rw_evt_payload *evt_payload = (struct rw_evt_payload *)data;
    char *event_tags[] = 
    {
        "station connected",
        "station got ip",
        "station disconnected",
        "station connect failed",
        "softap connected",
        "softap disconnected",
        "softap connect failed",
    };

    switch(evt_type) {
        case RW_EVT_STA_GOT_IP:
            g_aws_wifi_beken.wifi_conn_state = true;
            xEventGroupClearBits(g_aws_wifi_beken.wifi_event_group, BEKEN_DISCONNECTED_BIT);
            xEventGroupSetBits(g_aws_wifi_beken.wifi_event_group, BEKEN_CONNECTED_BIT);
            break;
        case RW_EVT_STA_DISCONNECTED:
        case RW_EVT_STA_CONNECT_FAILED:
            g_aws_wifi_beken.wifi_conn_state = false;
            xEventGroupClearBits(g_aws_wifi_beken.wifi_event_group, BEKEN_CONNECTED_BIT);
            xEventGroupSetBits(g_aws_wifi_beken.wifi_event_group, BEKEN_DISCONNECTED_BIT);
            break;
        default:
            break;
    }
    os_printf("---- %s ----conn_state=%d\n", event_tags[evt_type], g_aws_wifi_beken.wifi_conn_state);

    return 0;
}


/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_On( void )
{
    if (1 == g_aws_wifi_beken.initialized)
    {
        return eWiFiSuccess;
    }

    g_aws_wifi_beken.device_mode = eWiFiModeStation;
    rtos_init_semaphore(&g_aws_wifi_beken.scan_done_sem, 1);
    rtos_init_mutex(&g_aws_wifi_beken.wifi_lock);

    g_aws_wifi_beken.wifi_event_group = xEventGroupCreate();
    if (NULL == g_aws_wifi_beken.wifi_event_group)
    {
        os_printf("%s:%d Failed to create eventgroup\n", __FUNCTION__, __LINE__);
        return eWiFiFailure;
    }

    os_memset(& g_aws_wifi_beken.ap_config, 0x0, sizeof( g_aws_wifi_beken.ap_config));

    g_aws_wifi_beken.ap_config.wifi_mode = SOFT_AP;
    g_aws_wifi_beken.ap_config.dhcp_mode = DHCP_SERVER;
    g_aws_wifi_beken.ap_config.wifi_retry_interval = 100;
    os_strcpy(g_aws_wifi_beken.ap_config.local_ip_addr, WLAN_DEFAULT_IP);
    os_strcpy(g_aws_wifi_beken.ap_config.net_mask, WLAN_DEFAULT_MASK);
    os_strcpy(g_aws_wifi_beken.ap_config.dns_server_ip_addr, WLAN_DEFAULT_IP);

    rw_evt_set_callback(RW_EVT_STA_CONNECTED, beken_rw_event_callback);
    rw_evt_set_callback(RW_EVT_STA_GOT_IP, beken_rw_event_callback);
    rw_evt_set_callback(RW_EVT_STA_DISCONNECTED, beken_rw_event_callback);
    rw_evt_set_callback(RW_EVT_STA_CONNECT_FAILED, beken_rw_event_callback);
    rw_evt_set_callback(RW_EVT_AP_CONNECTED, beken_rw_event_callback);
    rw_evt_set_callback(RW_EVT_AP_DISCONNECTED, beken_rw_event_callback);
    rw_evt_set_callback(RW_EVT_AP_CONNECT_FAILED, beken_rw_event_callback);

    g_aws_wifi_beken.wifi_conn_state = false;
    g_aws_wifi_beken.initialized = 1;
    os_printf("%s:%d conn_state=%d\n", __FUNCTION__, __LINE__, g_aws_wifi_beken.wifi_conn_state);

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Off( void )
{
    /* do nothing like esp32, since stupid unittest case */
#if 0
    if (0 == g_aws_wifi_beken.initialized)
    {
        return eWiFiSuccess;
    }

    rtos_deinit_semaphore(&g_aws_wifi_beken.scan_done_sem);
    rtos_deinit_mutex(&g_aws_wifi_beken.wifi_lock);
    vEventGroupDelete(g_aws_wifi_beken.wifi_event_group);
    g_aws_wifi_beken.initialized = 0;
#endif

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    network_InitTypeDef_st wNetConfig;
    OSStatus status;
    WIFIReturnCode_t returnCode = eWiFiFailure;

    if (NULL == pxNetworkParams)
    {
        os_printf("%s:%d BADPARAM\n", __FUNCTION__, __LINE__);
        return eWiFiFailure;
    }

    if ((NULL == pxNetworkParams->pcSSID)
    || (0 == pxNetworkParams->ucSSIDLength)
    || (sizeof(wNetConfig.wifi_ssid) - 1 < pxNetworkParams->ucSSIDLength)
    || (sizeof(wNetConfig.wifi_key) < pxNetworkParams->ucPasswordLength)
    || ((NULL == pxNetworkParams->pcPassword) && (pxNetworkParams->ucPasswordLength > 0)))
    {
        os_printf("%s:%d ucSSIDLength=%d,ucPasswordLength=%d invalid\n", __FUNCTION__, __LINE__, pxNetworkParams->ucSSIDLength, pxNetworkParams->ucPasswordLength);
        return eWiFiFailure;
    }

    os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    os_strncpy((char *)wNetConfig.wifi_ssid, pxNetworkParams->pcSSID, sizeof(wNetConfig.wifi_ssid));

    if (NULL != pxNetworkParams->pcPassword)
    {
        os_strncpy((char *)wNetConfig.wifi_key, pxNetworkParams->pcPassword, sizeof(wNetConfig.wifi_key));
    }

    wNetConfig.wifi_mode = STATION;
    wNetConfig.dhcp_mode = DHCP_CLIENT;
    wNetConfig.wifi_retry_interval = 100;

    rtos_lock_mutex(&g_aws_wifi_beken.wifi_lock);
    // Check if WiFi is already connected
    if (g_aws_wifi_beken.wifi_conn_state == true) {
        status = bk_wlan_stop(STATION);
        if (kNoErr == status)
        {
            // Wait for wifi disconnected event
            xEventGroupWaitBits(g_aws_wifi_beken.wifi_event_group, BEKEN_DISCONNECTED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        }
    }

    /**
         * clear event for case that:
         * a. user disconnect outside with function
         * b. beken_rw_event_callback(RW_EVT_STA_DISCONNECTED) is invoked
         * c. g_aws_wifi_beken.wifi_conn_state is set to false and BEKEN_DISCONNECTED_BIT is set
         * d. above xEventGroupWaitBits(BEKEN_DISCONNECTED_BIT) won't trigger
         * e. follow xEventGroupWaitBits(BEKEN_DISCONNECTED_BIT | BEKEN_DISCONNECTED_BIT) return directly
         */
    xEventGroupClearBits(g_aws_wifi_beken.wifi_event_group, BEKEN_CONNECTED_BIT | BEKEN_DISCONNECTED_BIT);

    status = bk_wlan_start(&wNetConfig);
    if (kNoErr == status)
    {
        // Wait for wifi connected or disconnected event
        xEventGroupWaitBits(g_aws_wifi_beken.wifi_event_group, BEKEN_CONNECTED_BIT | BEKEN_DISCONNECTED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if (g_aws_wifi_beken.wifi_conn_state == true)
        {
            returnCode = eWiFiSuccess;
        }
    }
    rtos_unlock_mutex(&g_aws_wifi_beken.wifi_lock);
    os_printf("%s: conn_state=%d returnCode=%d ssid:%.*s key:%.*s\n", __FUNCTION__, g_aws_wifi_beken.wifi_conn_state, returnCode, sizeof(wNetConfig.wifi_ssid), wNetConfig.wifi_ssid, sizeof(wNetConfig.wifi_key), wNetConfig.wifi_key);

    return returnCode;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Disconnect( void )
{
    OSStatus status;
    WIFIReturnCode_t returnCode = eWiFiFailure;

    rtos_lock_mutex(&g_aws_wifi_beken.wifi_lock);
    status = bk_wlan_stop(STATION);
    if (kNoErr == status)
    {
        // Wait for wifi disconnected event
        //xEventGroupWaitBits(g_aws_wifi_beken.wifi_event_group, BEKEN_DISCONNECTED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        returnCode = eWiFiSuccess;
    }
    rtos_unlock_mutex(&g_aws_wifi_beken.wifi_lock);

    return returnCode;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Reset( void )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t * pxBuffer,
                            uint8_t ucNumNetworks )
{
    struct scanu_rst_upload *scan_rst;
    OSStatus status;
    uint8_t scanu_num = ucNumNetworks;
    uint8_t index;

    if ((NULL == pxBuffer) || (0 == ucNumNetworks))
    {
        os_printf("%s:%d BADPARAM pxBuffer=0x%x,ucNumNetworks=%d\n", __FUNCTION__, __LINE__, pxBuffer, ucNumNetworks);
        return eWiFiFailure;
    }

    bk_wlan_scan_ap_reg_cb(beken_scan_ap_callback);
    bk_wlan_start_scan(NULL);
    status = rtos_get_semaphore(&g_aws_wifi_beken.scan_done_sem, BEKEN_WAIT_FOREVER);
    if (kNoErr != status)
    {
        os_printf("%s:%d timeout or error\n", __FUNCTION__, __LINE__);

        return eWiFiFailure;
    }

    scan_rst = sr_get_scan_results();
    if ((NULL == scan_rst) || (0 == scan_rst->scanu_num))
    {
        os_printf("%s:%d no result\n", __FUNCTION__, __LINE__);
        sr_release_scan_results(scan_rst);

        return eWiFiFailure;
    }

    if (scan_rst->scanu_num < ucNumNetworks)
    {
        scanu_num = scan_rst->scanu_num;
    }

#if 0
    os_printf("             SSID                      MAC            rssi   chn    security\n");
    os_printf("------------------------------- -----------------     ----   ---    ----\n");
#endif
    for (index = 0;index<scanu_num;index++)
    {
        struct sta_scan_res *res = scan_rst->res[index];

        os_strncpy(pxBuffer[index].cSSID, res->ssid, wificonfigMAX_SSID_LEN + 1);
        os_memcpy(pxBuffer[index].ucBSSID, res->bssid, wificonfigMAX_BSSID_LEN);
        switch (res->security)
        {
        case SECURITY_TYPE_NONE:
            pxBuffer[index].xSecurity = eWiFiSecurityOpen;
            break;

        case SECURITY_TYPE_WEP:
            pxBuffer[index].xSecurity = eWiFiSecurityWEP;
            break;

        case SECURITY_TYPE_WPA_TKIP:
        case SECURITY_TYPE_WPA_AES:
            pxBuffer[index].xSecurity = eWiFiSecurityWPA;
            break;

        case SECURITY_TYPE_WPA2_TKIP:
        case SECURITY_TYPE_WPA2_AES:
        case SECURITY_TYPE_WPA2_MIXED:
            pxBuffer[index].xSecurity = eWiFiSecurityWPA2;
            break;

        default:
            pxBuffer[index].xSecurity = eWiFiSecurityNotSupported;
            break;
        }
        pxBuffer[index].cRSSI = res->level;
        pxBuffer[index].cChannel = res->channel;
        if (0 == os_strlen(pxBuffer[index].cSSID))
        {
            pxBuffer[index].ucHidden = 1;
        }
        else
        {
            pxBuffer[index].ucHidden = 0;
        }
#if 0
        os_printf("%-32.32s", res->ssid);
        os_printf("%02x:%02x:%02x:%02x:%02x:%02x     ", 
                    res->bssid[0], res->bssid[1], res->bssid[2],
                    res->bssid[3], res->bssid[4], res->bssid[5]);
        os_printf("%4d    %2d    %d\n", res->level, res->channel, res->security);
#endif
    }

    sr_release_scan_results(scan_rst);

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetMode( WIFIDeviceMode_t xDeviceMode )
{
    if ((eWiFiModeStation == xDeviceMode)
    || (eWiFiModeAP == xDeviceMode))
    {
        g_aws_wifi_beken.device_mode = xDeviceMode;
    }
    else
    {
        os_printf("%s:%d BADPARAM xDeviceMode=%d\n", __FUNCTION__, __LINE__, xDeviceMode);
        return eWiFiNotSupported;
    }

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMode( WIFIDeviceMode_t *  pxDeviceMode)
{
    if (NULL == pxDeviceMode)
    {
        os_printf("%s:%d BADPARAM\n", __FUNCTION__, __LINE__);
        return eWiFiFailure;
    }

    *pxDeviceMode = g_aws_wifi_beken.device_mode;

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t * pusIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t * pxNetworkProfile,
                                  uint16_t usIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Ping( uint8_t * pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    OSStatus status;

    if (NULL == pucIPAddr)
    {
        os_printf("%s:%d BADPARAM\n", __FUNCTION__, __LINE__);
        return eWiFiFailure;
    }

    status = ping((struct in_addr *)pucIPAddr, usCount, 0, ulIntervalMS);

    return (kNoErr == status) ? eWiFiSuccess : eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetIP( uint8_t * pucIPAddr )
{
    struct wlan_ip_config addr;

    if (NULL == pucIPAddr)
    {
        os_printf("%s:%d BADPARAM\n", __FUNCTION__, __LINE__);
        return eWiFiFailure;
    }

    os_memset(&addr, 0, sizeof(struct wlan_ip_config));    

    if (eWiFiModeAP == g_aws_wifi_beken.device_mode)
    {
        net_get_if_addr(&addr, net_get_uap_handle());
    }
    else
    {
        net_get_if_addr(&addr, net_get_sta_handle());
    }

    memcpy((void *)pucIPAddr, (void *)&addr.ipv4.address, sizeof(addr.ipv4.address));

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
    if (NULL == pucMac)
    {
        os_printf("%s:%d BADPARAM\n", __FUNCTION__, __LINE__);
        return eWiFiFailure;
    }

    wifi_get_mac_address(pucMac, CONFIG_ROLE_STA);

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetHostIP( char * pcHost,
                                 uint8_t * pucIPAddr )
{
    struct addrinfo hint, *res = NULL;

    if ((NULL == pcHost) || (NULL == pucIPAddr))
    {
        os_printf("%s:%d BADPARAM pcHost=0x%x,pucIPAddr=0x%x\n", __FUNCTION__, __LINE__, pcHost, pucIPAddr);
        return eWiFiFailure;
    }

#if defined(AMAZON_FREERTOS_ENABLE_UNIT_TESTS)
    /* workaround for IDT: DNS of office would convert invalid host to a special IP */
    if (0 == strcmp(pcHost, "invalid"))
    {
        os_printf("%s:%d invalid\n", __FUNCTION__, __LINE__);
        return eWiFiFailure;
    }
#endif

    memset(&hint, 0, sizeof(hint));
    hint.ai_family     = AF_INET;
    hint.ai_socktype   = SOCK_DGRAM;
    hint.ai_flags      = 0;
    hint.ai_protocol   = 0;

    /* convert URL to IP */
    if (lwip_getaddrinfo(pcHost, NULL, &hint, &res) != 0)
    {
        os_printf("%s: unknown host %s\n", __FUNCTION__, pcHost);
        return eWiFiFailure;
    }
    else
    {
        os_printf("%s: host %s => %s\n", __FUNCTION__, pcHost, inet_ntoa(((struct sockaddr_in *)(res->ai_addr))->sin_addr));
    }
    memcpy((void *)pucIPAddr, (void *)&((struct sockaddr_in *)(res->ai_addr))->sin_addr, sizeof(struct in_addr));
    lwip_freeaddrinfo(res);

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartAP( void )
{
    network_InitTypeDef_st *ap_config;
    OSStatus status;
    uint8_t mac[6];

    ap_config = &g_aws_wifi_beken.ap_config;
    if (0 == os_strlen(ap_config->wifi_ssid))
    {
        wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);
        os_snprintf(ap_config->wifi_ssid, sizeof(ap_config->wifi_ssid),
                           "aws-%02x%02x%02x%02x%02x%02x",
                           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    rtos_lock_mutex(&g_aws_wifi_beken.wifi_lock);
    status = bk_wlan_start(ap_config);
    rtos_unlock_mutex(&g_aws_wifi_beken.wifi_lock);
    os_printf("%s: status=%d ssid:%.*s key:%.*s\n", __FUNCTION__, status, sizeof(ap_config->wifi_ssid), ap_config->wifi_ssid, sizeof(ap_config->wifi_key), ap_config->wifi_key);

    return (kNoErr == status) ? eWiFiSuccess : eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StopAP( void )
{
    OSStatus status;

    rtos_lock_mutex(&g_aws_wifi_beken.wifi_lock);
    status = bk_wlan_stop(SOFT_AP);
    rtos_unlock_mutex(&g_aws_wifi_beken.wifi_lock);

    return (kNoErr == status) ? eWiFiSuccess : eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConfigureAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    network_InitTypeDef_st *ap_config;
    if (NULL == pxNetworkParams)
    {
        return eWiFiFailure;
    }

    ap_config = &g_aws_wifi_beken.ap_config;
    if ((NULL != pxNetworkParams->pcSSID) && (0 != pxNetworkParams->ucSSIDLength))
    {
        os_strncpy(ap_config->wifi_ssid, pxNetworkParams->pcSSID, sizeof(ap_config->wifi_ssid));
    }

    if (NULL != pxNetworkParams->pcPassword)
    {
        os_strncpy(ap_config->wifi_key, pxNetworkParams->pcPassword, sizeof(ap_config->wifi_key));
    }
    else
    {
        os_memset(ap_config->wifi_key, 0, sizeof(ap_config->wifi_key));
    }

    if ((0 < pxNetworkParams->cChannel) && (pxNetworkParams->cChannel < 14))
    {
        bk_wlan_ap_set_default_channel(pxNetworkParams->cChannel);
    }

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetPMMode( WIFIPMMode_t xPMModeType,
                                 const void * pvOptionValue )
{
    g_aws_wifi_beken.pm_mode = xPMModeType;
    switch (xPMModeType)
    {
#if CFG_USE_MCU_PS
        case eWiFiPMNormal:
            /* disable cpu sleep */
            bk_wlan_mcu_ps_mode_disable();
            /* disable rf sleep */
            bk_wlan_dtim_rf_ps_disable_send_msg();
            /* pause rf timer */
            bk_wlan_dtim_rf_ps_timer_pause();
            break;

        case eWiFiPMLowPower:
            /* enable cpu sleep */
            bk_wlan_mcu_ps_mode_enable();
            /* enable rf sleep */
            bk_wlan_dtim_rf_ps_mode_enable();
            /* start rf timer */
            bk_wlan_dtim_rf_ps_timer_start();
            break;

        case eWiFiPMAlwaysOn:
            /* enable cpu sleep */
            bk_wlan_mcu_ps_mode_enable();
            /* disable rf sleep */
            bk_wlan_dtim_rf_ps_disable_send_msg();
            /* pause rf timer */
            bk_wlan_dtim_rf_ps_timer_pause();
            break;
#endif

        default:
            return eWiFiNotSupported;
    }

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetPMMode( WIFIPMMode_t * pxPMModeType,
                                 void * pvOptionValue )
{
#if CFG_USE_MCU_PS
    return g_aws_wifi_beken.pm_mode;
#else
    return eWiFiNotSupported;
#endif
}
/*-----------------------------------------------------------*/

BaseType_t WIFI_IsConnected(void)
{
	if (uap_ip_is_start())
	{
	    return pdTRUE;
	}
	if (sta_ip_is_start() && g_aws_wifi_beken.wifi_conn_state)
	{
	    return pdTRUE;
	}
	
	return pdFALSE;
}
