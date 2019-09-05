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

#if 1//( testrunnerFULL_MQTTv4_ENABLED == 1 )
static void RunMQTTValidateTests(void)
{
    RUN_TEST_GROUP( MQTT_Unit_Validate );
}

static void RunMQTTSubscriptionTests(void)
{
    RUN_TEST_GROUP( MQTT_Unit_Subscription );
}

static void RunMQTTReceiveTests(void)
{
    RUN_TEST_GROUP( MQTT_Unit_Receive );
}

static void RunMQTTAPITests(void)
{
    RUN_TEST_GROUP( MQTT_Unit_API );
}

static void RunMQTTSystemTests(void)
{
    RUN_TEST_GROUP( MQTT_System );
}
#endif /* if ( testrunnerFULL_MQTTv4_ENABLED == 1 ) */

static void RunMQTTTests(void)
{
#if ( testrunnerFULL_MQTT_AGENT_ENABLED == 1 )
    RUN_TEST_GROUP( Full_MQTT_Agent );
#endif
#if ( testrunnerFULL_MQTT_ALPN_ENABLED == 1 )
    RUN_TEST_GROUP( Full_MQTT_Agent_ALPN );
#endif
#if 0//( testrunnerFULL_MQTT_STRESS_TEST_ENABLED == 1 )
    RUN_TEST_GROUP( Full_MQTT_Agent_Stress_Tests );
#endif
}

static void RunGGDTests(void)
{
#if ( testrunnerFULL_GGD_ENABLED == 1 )
    RUN_TEST_GROUP( Full_GGD );
#endif

#if ( testrunnerFULL_GGD_HELPER_ENABLED == 1 )
    RUN_TEST_GROUP( Full_GGD_Helper );
#endif
}

static void RunShadowTests(void)
{
#if ( testrunnerFULL_SHADOW_ENABLED == 1 )
    RUN_TEST_GROUP( Full_Shadow );
#endif

#if ( testrunnerFULL_SHADOWv4_ENABLED == 1 )
    RUN_TEST_GROUP( Shadow_Unit_Parser );
    RUN_TEST_GROUP( Shadow_Unit_API );
    RUN_TEST_GROUP( Shadow_System );
#endif /* if ( testrunnerFULL_SHADOWv4_ENABLED == 1 ) */
}

static void RunMQTTStressTests(void)
{
    //RUN_TEST_GROUP(Full_MQTT_Agent_Stress_Tests);
}

static void RunCRYPTOTests(void)
{
    RUN_TEST_GROUP(Full_PKCS11_CryptoOperation);
    RUN_TEST_GROUP(Full_PKCS11_GeneralPurpose);
    RUN_TEST_GROUP(Full_CRYPTO);
    RUN_TEST_GROUP(Full_TLS);
}

static void RunWiFiTests(void)
{
    RUN_TEST_GROUP(Full_WiFi);
}

static void RunTCPTests(void)
{
    RUN_TEST_GROUP(Full_TCP);
#if ( testrunnerFULL_FREERTOS_TCP_ENABLED == 1 )
    RUN_TEST_GROUP( Full_FREERTOS_TCP );
#endif
}

static void RunOTATests(void)
{
#if ( testrunnerFULL_OTA_PAL_ENABLED == 1 )
        RUN_TEST_GROUP( Full_OTA_PAL );
#endif
#if ( testrunnerFULL_OTA_AGENT_ENABLED == 1 )
    RUN_TEST_GROUP( Full_OTA_AGENT );
#endif
#if ( testrunnerFULL_OTA_CBOR_ENABLED == 1 )
    RUN_TEST_GROUP( Full_OTA_CBOR );
    RUN_TEST_GROUP( Quarantine_OTA_CBOR );
#endif
}

static void RunSystemTests(void)
{
#if ( testrunnerFULL_DEFENDER_ENABLED == 1 )
    RUN_TEST_GROUP(Full_DEFENDER);
#endif
#if ( testrunnerFULL_POSIX_ENABLED == 1 )
    RUN_TEST_GROUP( Full_POSIX_CLOCK );
    RUN_TEST_GROUP( Full_POSIX_MQUEUE );
    RUN_TEST_GROUP( Full_POSIX_PTHREAD );
    RUN_TEST_GROUP( Full_POSIX_SEMAPHORE );
    RUN_TEST_GROUP( Full_POSIX_TIMER );
    RUN_TEST_GROUP( Full_POSIX_UTILS );
    RUN_TEST_GROUP( Full_POSIX_STRESS );
#endif
    RUN_TEST_GROUP(Full_MemoryLeak);
}

static void RunAllTests(void)
{
    RunMQTTTests();
    RunCRYPTOTests();
    RunWiFiTests();
    RunTCPTests();
    RunSystemTests();
    RunOTATests();
    RunGGDTests();
    RunShadowTests();
}

#include "jsmn.h"
#include "aws_greengrass_discovery.h"

#if 0
#define ggdJSON_FILE                       "{\"GGGroups\":[{\"GGGroupId\":\"myGroupID\",\"Cores\":[{\"thingArn\":\"myGreenGrassCoreArn\",\"Connectivity\":[{\"Id\":\"AUTOIP_10.60.212.138_0\",\"HostAddress\":\"44.44.44.44\",\"PortNumber\":1234,\"Metadata\":\"\"},{\"Id\":\"AUTOIP_127.0.0.1_1\",\"HostAddress\":\"127.0.0.1\",\"PortNumber\":8883,\"Metadata\":\"\"},{\"Id\":\"AUTOIP_192.168.2.2_2\",\"HostAddress\":\"01.23.456.789\",\"PortNumber\":4321,\"Metadata\":\"\"},{\"Id\":\"AUTOIP_::1_3\",\"HostAddress\":\"::1\",\"PortNumber\":8883,\"Metadata\":\"\"},{\"Id\":\"AUTOIP_fe80::bfda:8f62:7b4b:f358_4\",\"HostAddress\":\"fe80::bfda:8f62:7b4b:f358\",\"PortNumber\":8883,\"Metadata\":\"\"},{\"Id\":\"AUTOIP_fe80::e234:cff9:f53f:6216_5\",\"HostAddress\":\"fe80::e234:cff9:f53f:6216\",\"PortNumber\":8883,\"Metadata\":\"\"}]}],\"CAs\":[\"-----BEGIN CERTIFICATE-----\\nMIIEFTCCAv2gAwIBAgIVAPRru+NqCDr0r6oD6PnTG05rWuY+MA0GCSqGSIb3DQEB\\nCwUAMIGoMQswCQYDVQQGEwJVUzEYMBYGA1UECgwPQW1hem9uLmNvbSBJbmMuMRww\\nGgYDVQQLDBNBbWF6b24gV2ViIFNlcnZpY2VzMRMwEQYDVQQIDApXYXNoaW5ndG9u\\nMRAwDgYDVQQHDAdTZWF0dGxlMTowOAYDVQQDDDE5NDI5MjczNzY5NjU6ZDk3ZmZl\\nZmUtNTI4MS00ZWM5LTk4NDYtYjNlZTQxMDRjMjAxMCAXDTE3MDcwNjIwMDczOFoY\\nDzIwOTcwNzA2MjAwNzM3WjCBqDELMAkGA1UEBhMCVVMxGDAWBgNVBAoMD0FtYXpv\\nbi5jb20gSW5jLjEcMBoGA1UECwwTQW1hem9uIFdlYiBTZXJ2aWNlczETMBEGA1UE\\nCAwKV2FzaGluZ3RvbjEQMA4GA1UEBwwHU2VhdHRsZTE6MDgGA1UEAwwxOTQyOTI3\\nMzc2OTY1OmQ5N2ZmZWZlLTUyODEtNGVjOS05ODQ2LWIzZWU0MTA0YzIwMTCCASIw\\nDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKxzJpXU2DZDEglh/FT01epAWby6\\np4Ymw76icyMzBUJzafibABJ3cTyjDQE6ZqbSl1ryBxGwQBsveIgj8SVVtv927wk7\\nlncgD+EghfTZgSfscND653AJeVFQlCeHipZI32wzXyPmwglFrWp9vsrY/8BO1Kjk\\nSAs4o8fDVVMAaZCJDMuc5csc3CQ2OJYLOl+SZisGNM1h0xHpWieM38KDDrp99x8Q\\nTwDmgaMjtdIJR7Y9Nzm0N78gTf3gTazEO9iUKojVCNubxK/lQ6KjJ0JcvsljPpVp\\nuzjOmn91xmNoHEQCboa7YoYNNbdAbftGeUl16wFdTgbuUS9vakk5idVoC2ECAwEA\\nAaMyMDAwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUmcz4OlH9+mlpnTKG3taI\\nw+6FSk0wDQYJKoZIhvcNAQELBQADggEBACeiQ6MxiktsU0sLNmP1cNbiuBuutjoq\\nymk476Bhr4E2WSE0B9W1TFOSLIYx9oN63T3lXzsGHP/MznueIbqbwFf/o5aXI7th\\n+J+i9LgBrViNvzkze7G0GiPuEQ7ox4XnPBJAFtTZxa8gXL95QfcypERpQs28lg7W\\nQpdNhiBN+c4o1aSOzJ474sjXnjtI1G2jRTKucm0buYYeAeVT7kpBq9YL7gGfOcyj\\nsPxQEgyQV2Mk+b1q7lYDS4tnzoRkUfNLgAtDKSh8S8iVhAR6wRR2G3aMySKrOxbg\\nalghO3OqfeuTwIj9w17JTAyYAME22RJQ6oxEJ8rHp/9PaYnOmiSkP7M=\\n-----END CERTIFICATE-----\\n\"]}]}"
#define ggdTestJSON_MAX_TOKENS             128       /* Size of the array used by jsmn to store the tokens. */
extern char cBuffer[ testrunnerBUFFER_SIZE ];
void handle_json()
{
    static const char cJSON_FILE[] = ggdJSON_FILE;
    static jsmntok_t pxTok[ ggdTestJSON_MAX_TOKENS ];
    static char *jsmnTypes[] = 
    {
        "Unknown",
        "Object",
        "Array",
        "String",
        "Primitive"
    };
    jsmn_parser xParser;
    int32_t lNbTokens;
    uint32_t ulJSONFileSize = strlen( cJSON_FILE );
    jsmntok_t *tokenPtr;
    int32_t tokenIndex;
    int32_t index;

    jsmn_init( &xParser );
    memcpy( cBuffer, cJSON_FILE, ulJSONFileSize );

    /* From jsmn, parse the JSON file. */
    lNbTokens = ( int32_t ) jsmn_parse( &xParser,
                                        cBuffer, /*lint !e971 can use char without signed/unsigned. */
                                        ( size_t ) ulJSONFileSize,
                                        pxTok,
                                        ( unsigned int ) ggdTestJSON_MAX_TOKENS ); /*lint !e961 redundant casting only when int = int32_t. */
    for (tokenPtr = pxTok, tokenIndex = 0; tokenIndex < lNbTokens; tokenPtr++, tokenIndex++)
    {
        bk_printf("[%d]%s:[%d~%d=%d]\n", tokenIndex, jsmnTypes[tokenPtr->type], tokenPtr->start, tokenPtr->end, tokenPtr->end - tokenPtr->start);
        for (index = tokenPtr->start; index < tokenPtr->end; index++)
        {
            bk_send_byte(1, cJSON_FILE[index]);
        }
        bk_send_byte(1, '\n');
    }

    {
        HostParameters_t xHostParameters;
        GGD_HostAddressData_t xHostAddressData;
        BaseType_t xStatus;

        xHostParameters.pcCoreAddress = ( char * ) "myGreenGrassCoreArn";
        xHostParameters.pcGroupName = ( char * ) "myGroupID";
        xHostParameters.ucInterface = 3;
        xStatus = GGD_GetIPandCertificateFromJSON( cBuffer, /*lint !e971 can use char without signed/unsigned. */
                                                   ulJSONFileSize,
                                                   &xHostParameters,
                                                   &xHostAddressData,
                                                   pdFALSE );
        bk_printf("xStatus=%d,%d,%d:%.*s\n", xStatus, xHostAddressData.ulCertificateSize, xHostAddressData.usPort, 15, xHostAddressData.pcHostAddress);
    }
}
#endif

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
    else if (0 == strcmp(argv[1], "mqttval"))
    {
        test_proc = RunMQTTValidateTests;
    }
    else if (0 == strcmp(argv[1], "mqttsub"))
    {
        test_proc = RunMQTTSubscriptionTests;
    }
    else if (0 == strcmp(argv[1], "mqttrcev"))
    {
        test_proc = RunMQTTReceiveTests;
    }
    else if (0 == strcmp(argv[1], "mqttapi"))
    {
        test_proc = RunMQTTAPITests;
    }
    else if (0 == strcmp(argv[1], "mqttsys"))
    {
        test_proc = RunMQTTSystemTests;
    }
    else if (0 == strcmp(argv[1], "mqtt"))
    {
        test_proc = RunMQTTTests;
    }
    else if (0 == strcmp(argv[1], "crypto"))
    {
        test_proc = RunCRYPTOTests;
    }
    else if (0 == strcmp(argv[1], "wifi"))
    {
        test_proc = RunWiFiTests;
    }
    else if (0 == strcmp(argv[1], "tcp"))
    {
        test_proc = RunTCPTests;
    }
    else if (0 == strcmp(argv[1], "system"))
    {
        test_proc = RunSystemTests;
    }
    else if (0 == strcmp(argv[1], "ota"))
    {
        test_proc = RunOTATests;
    }
    else if (0 == strcmp(argv[1], "ggd"))
    {
        test_proc = RunGGDTests;
    }
    else if (0 == strcmp(argv[1], "shadow"))
    {
        test_proc = RunShadowTests;
    }
    else if (0 == strcmp(argv[1], "stress"))
    {
        test_proc = RunMQTTStressTests;
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
        .name = "test",
        .help = "test [all | mqtt | crypto | network | system]",
        .function = handle_test_main,
    },
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
