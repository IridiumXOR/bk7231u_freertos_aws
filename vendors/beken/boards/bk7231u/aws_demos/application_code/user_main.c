/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_pub.h"
#include "aws_application_version.h"

/* Logging Task Defines. */
#define mainLOGGING_MESSAGE_QUEUE_LENGTH    ( 15 )
#define mainLOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE )

#ifdef CFG_ENABLE_USER_APP

#include "wlan_cli_pub.h"
#include <string.h>
#include "kv/kvmgr.h"
#include "aws_wifi.h"
#include "aws_system_init.h"
#include "aws_mqtt_agent.h"
#include "aws_logging_task.h"
#include "aws_dev_mode_key_provisioning.h"
#include "aws_clientcredential.h"

void prvWifiConnect( void )
{
    WIFINetworkParams_t xJoinAPParams;
    WIFIReturnCode_t xWifiStatus;

    xWifiStatus = WIFI_On();

    if( xWifiStatus == eWiFiSuccess )
    {
        configPRINTF( ( "WiFi module initialized. Connecting to AP...\r\n" ) );
    }
    else
    {
        configPRINTF( ( "WiFi module failed to initialize.\r\n" ) );

        while( 1 )
        {
        }
    }

    /* Setup parameters. */
    xJoinAPParams.pcSSID           = clientcredentialWIFI_SSID;
    xJoinAPParams.ucSSIDLength     = sizeof( clientcredentialWIFI_SSID );
    xJoinAPParams.pcPassword       = clientcredentialWIFI_PASSWORD;
    xJoinAPParams.ucPasswordLength = sizeof( clientcredentialWIFI_PASSWORD );
    xJoinAPParams.xSecurity        = clientcredentialWIFI_SECURITY;

    xWifiStatus = WIFI_ConnectAP( &( xJoinAPParams ) );

    if( xWifiStatus == eWiFiSuccess )
    {
        configPRINTF( ( "WiFi Connected to AP. Creating tasks which use network...\r\n" ) );
    }
    else
    {
        configPRINTF( ( "WiFi failed to connect to AP.\r\n" ) );

        while( 1 )
        {
            taskYIELD();
        }
    }
}

void user_main( beken_thread_arg_t args )
{
    xLoggingTaskInitialize(mainLOGGING_TASK_STACK_SIZE, tskIDLE_PRIORITY + 3, mainLOGGING_MESSAGE_QUEUE_LENGTH);

    /* cli init in app_start, so kv init should later than it */
    aos_kv_init();

    /* FIX ME: Perform any hardware initialization, that require the RTOS to be
     * running, here. */
    vDevModeKeyProvisioning();

    /* FIX ME: If your MCU is using Wi-Fi, delete surrounding compiler directives to
     * enable the unit tests and after MQTT, Bufferpool, and Secure Sockets libraries
     * have been imported into the project. If you are not using Wi-Fi, see the
     * vApplicationIPNetworkEventHook function. */
    if( SYSTEM_Init() == pdPASS )
    {
        /* Connect to the wifi before running real jobs. */
        prvWifiConnect();
    }

    rtos_delete_thread( NULL );
}
#endif // CFG_ENABLE_USER_APP
