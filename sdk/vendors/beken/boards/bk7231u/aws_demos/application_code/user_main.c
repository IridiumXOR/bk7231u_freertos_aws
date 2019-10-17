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

void handle_mqtt_main(char *pwbuf, int blen, int argc, char **argv)
{
    MQTTAgentReturnCode_t xReturned = eMQTTAgentFailure;
    StaticSemaphore_t xSemaphore = { 0 };
    MQTTAgentHandle_t xMQTTHandle = NULL;
    MQTTAgentSubscribeParams_t xSubscribeParams;
    MQTTAgentPublishParams_t xPublishParameters;
    BaseType_t xMQTTAgentCreated = pdFALSE;
    BaseType_t xUseAlpn = pdFALSE;
    MQTTAgentConnectParams_t xConnectParameters;

    IotSdk_Init();
    if (argc > 1)
    {
        if (0 == strcmp(argv[1], "alpn"))
        {
            xUseAlpn = pdTRUE;
        }
    }

    memset( &xConnectParameters, 0x0, sizeof( MQTTAgentConnectParams_t ) );

    /* Fill in the MQTTAgentConnectParams_t member that is not const. */
    xConnectParameters.pcURL = clientcredentialMQTT_BROKER_ENDPOINT;
    xConnectParameters.xFlags = mqttagentREQUIRE_TLS;
    xConnectParameters.xURLIsIPAddress = pdFALSE;
    xConnectParameters.usPort = clientcredentialMQTT_BROKER_PORT;
    xConnectParameters.pucClientId = ( const uint8_t * ) clientcredentialIOT_THING_NAME;
    xConnectParameters.usClientIdLength = ( uint16_t ) strlen(
        ( char * ) xConnectParameters.pucClientId );
    xConnectParameters.xSecuredConnection = pdTRUE;
    xConnectParameters.pvUserData = NULL;
    xConnectParameters.pxCallback = NULL;
    xConnectParameters.pcCertificate = NULL;
    xConnectParameters.ulCertificateSize =  0;

    /* Switch ports if requested. */
    if( xUseAlpn )
    {
        xConnectParameters.xFlags |= mqttagentUSE_AWS_IOT_ALPN_443;
    }

    {
        /* The MQTT client object must be created before it can be used. */
        xReturned = MQTT_AGENT_Create( &xMQTTHandle );
        xMQTTAgentCreated = pdTRUE;

        /* Connect to the broker. */
        xReturned = MQTT_AGENT_Connect( xMQTTHandle,
                                        &xConnectParameters,
                                        pdMS_TO_TICKS( 10000UL ) );

        /* Setup the publish parameters. */
        memset( &( xPublishParameters ), 0x00, sizeof( xPublishParameters ) );
        xPublishParameters.pucTopic = ( ( const uint8_t * ) "freertos/tests/echo" );
        xPublishParameters.pvData = "Hello from cli test.";
        xPublishParameters.usTopicLength = ( uint16_t ) strlen( ( const char * ) xPublishParameters.pucTopic );
        xPublishParameters.ulDataLength = ( uint32_t ) strlen( xPublishParameters.pvData );
        xPublishParameters.xQoS = eMQTTQoS1;

        /* Publish the message. */
        xReturned = MQTT_AGENT_Publish( xMQTTHandle,
                                        &( xPublishParameters ),
                                        pdMS_TO_TICKS( 10000UL ) );

        /* Disconnect the client. */
        xReturned = MQTT_AGENT_Disconnect( xMQTTHandle, pdMS_TO_TICKS( 10000UL ) );
    }

    /*Don't forget to reset the flag, since connect parameters are global, all test afterwards would use ALPN. */
    xConnectParameters.xFlags &= ~mqttagentUSE_AWS_IOT_ALPN_443;

    if( xMQTTAgentCreated == pdTRUE )
    {
        /* Delete the MQTT client. */
        xReturned = MQTT_AGENT_Delete( xMQTTHandle );
    }

    IotSdk_Cleanup();
}

void handle_ota_main(char *pwbuf, int blen, int argc, char **argv)
{
    //vStartOTAUpdateDemoTask();
}

static struct cli_command test_cmd[] = {
    {
        .name = "mqtt",
        .help = "mqtt [sub | pub] [-t topic] [-m message]",
        .function = handle_mqtt_main,
    },
    {
        .name = "ota",
        .help = "ota",
        .function = handle_ota_main,
    }
};

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
    cli_register_commands(test_cmd, sizeof(test_cmd) / sizeof(test_cmd[0]));

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
