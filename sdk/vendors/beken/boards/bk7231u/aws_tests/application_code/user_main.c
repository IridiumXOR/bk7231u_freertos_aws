/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_pub.h"
#include "wdt_pub.h"
#include "aws_test_runner_config.h"

/* Logging Task Defines. */
#define mainLOGGING_MESSAGE_QUEUE_LENGTH    ( 15 )
#define mainLOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE )

/* Unit test defines. */
#define mainTEST_RUNNER_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 7 )

#ifdef CFG_ENABLE_USER_APP

#include "wlan_cli_pub.h"
#include "unity_fixture.h"
#include <string.h>
#include "kv/kvmgr.h"
#include "iot_init.h"
#include "aws_wifi.h"
#include "aws_system_init.h"
#include "aws_mqtt_agent.h"
#include "aws_logging_task.h"
#include "aws_dev_mode_key_provisioning.h"
#include "aws_test_runner.h"


static void RunAllTests(void)
{
}

void handle_test_task( void * pvParameters )
{
    extern void list_free_nodes();
    list_free_nodes();
    UnityMain(0, (const char **)NULL, pvParameters);
    list_free_nodes();
    rtos_delete_thread( NULL );
}

void handle_test_main(char *pwbuf, int blen, int argc, char **argv)
{
    void (*test_proc)();
    if ((argc < 2) || (0 == strcmp(argv[1], "all")))
    {
        test_proc = RunAllTests;
    }
    else if (0 == strcmp(argv[1], "help"))
    {
        configPRINTF( ( "%s: all|mqtt|crypto|wifi|tcp|system|ota|ggd|shadow|stress|help\r\n", argv[0] ) );
        return;
    }
#if 0
    else if (0 == strcmp(argv[1], "json"))
    {
        handle_json();
        return;
    }
#endif
    /* Create the task to run unit tests. */
    xTaskCreate( handle_test_task,
                 "UnityMain_task",
                 mainTEST_RUNNER_TASK_STACK_SIZE,
                 test_proc,
                 tskIDLE_PRIORITY + 1,
                 NULL );
}

static struct cli_command test_cmd[] = {
    {
        .name = "test",
        .help = "test [all | mqtt | crypto | network | system]",
        .function = handle_test_main,
    },
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

    //bk_wdt_start(10000);

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

#if 1
        /* Create the task to run unit tests. */
        xTaskCreate( TEST_RUNNER_RunTests_task,
                     "RunTests_task",
                     mainTEST_RUNNER_TASK_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 1,
                     NULL );
#endif
    }

    rtos_delete_thread( NULL );
}
#endif // CFG_ENABLE_USER_APP
