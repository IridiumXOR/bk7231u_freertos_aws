/*
 * Amazon FreeRTOS V201906.00 Major
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

#ifndef BK_CLIENT_CREDENTIAL_KEYS_H
#define BK_CLIENT_CREDENTIAL_KEYS_H

/*
 * @brief PEM-encoded client certificate.
 *
 * @todo If you are running one of the Amazon FreeRTOS demo projects, set this
 * to the certificate that will be used for TLS client authentication.
 *
 * @note Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----\n"
 */
/* 20902cfd72-certificate.pem.crt */
#define BK_CLIENT_CERTIFICATE_PEM "-----BEGIN CERTIFICATE-----\n"\
"MIIDWTCCAkGgAwIBAgIUfWVojXhTxiPnQmf+vMs4vzJ1U9IwDQYJKoZIhvcNAQEL\n"\
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"\
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE5MDIyODA2NDc1\n"\
"MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"\
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANTnW9rOYAb/6fHq4paD\n"\
"I72BNUi7IjJY+UeGGEzb6kQvxL8UpUYyy47upw6mevZa6S/drRloXLeuDJqCK4qN\n"\
"JubH42ewOanJdalB5EEos8PE1K2e+3BdlHB3niGVt91zAbgnoKm9c2h7k4H+jJdC\n"\
"2hCZuj+yciWhl4LXYDaE6Ll+UNTq4GXlVKQL5ydVF64OjOJ/VjMIDgF8CtpG+/bl\n"\
"RXIQ+U621pjpHJrFCMgBMjY6W3ubT/qrttSsI26XDVuTeBJ72v5d6TifSrDh11Z1\n"\
"CEV89dY1KydFZmcYWn9TTKKfd8hDWoZXlsDHX+ol1qbIoQ+ItIqSnwzIrw6SO0Bn\n"\
"kaUCAwEAAaNgMF4wHwYDVR0jBBgwFoAUqr/xZ/86QdRxNKLcH4iIpDio0LowHQYD\n"\
"VR0OBBYEFCkTqOjGwiKz+mtRaxwM4F5sg0SBMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"\
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCKQyWkl5eEqlYEb2BYgXxu9IXr\n"\
"dlcGplJFK2K0aUQem4uIN7IqfV58j5PTMoeIleC1L+4Y6P4MF46H5ZAXSqLKp2Ll\n"\
"SnLg/VRYD5csgztmw6YLjEFYstGSvNOgW6Y9sF+Mzldt9iI/JcH8qYg1Kd6BfehE\n"\
"DVwqchh+bz/aluxtpCNj0hz7ozsaTjhfCTKlXIiwr8V6Jx15fsS9KI2Gif4cjsbb\n"\
"fScZr7hW1A7TEnBMzQ/IyWqmMDebpDn9OhvLPOiCKN+/33KvjaHLuG4TTbx4mniJ\n"\
"v+I3F9GEr2IPz7lOwcKC5MsnuYEl6mmeob4SSTOYadlFxDPm7iH5bJQNaYCz\n"\
"-----END CERTIFICATE-----\n"

/* 20902cfd72-public.pem.key */
#define BK_CLIENT_PUBLIC_KEY_PEM "-----BEGIN PUBLIC KEY-----\n"\
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1Odb2s5gBv/p8eriloMj\n"\
"vYE1SLsiMlj5R4YYTNvqRC/EvxSlRjLLju6nDqZ69lrpL92tGWhct64MmoIrio0m\n"\
"5sfjZ7A5qcl1qUHkQSizw8TUrZ77cF2UcHeeIZW33XMBuCegqb1zaHuTgf6Ml0La\n"\
"EJm6P7JyJaGXgtdgNoTouX5Q1OrgZeVUpAvnJ1UXrg6M4n9WMwgOAXwK2kb79uVF\n"\
"chD5TrbWmOkcmsUIyAEyNjpbe5tP+qu21KwjbpcNW5N4Enva/l3pOJ9KsOHXVnUI\n"\
"RXz11jUrJ0VmZxhaf1NMop93yENahleWwMdf6iXWpsihD4i0ipKfDMivDpI7QGeR\n"\
"pQIDAQAB\n"\
"-----END PUBLIC KEY-----\n"

/*
 * @brief PEM-encoded issuer certificate for AWS IoT Just In Time Registration (JITR).
 *
 * @todo If you are using AWS IoT Just in Time Registration (JITR), set this to
 * the issuer (Certificate Authority) certificate of the client certificate above.
 *
 * @note This setting is required by JITR because the issuer is used by the AWS
 * IoT gateway for routing the device's initial request. (The device client
 * certificate must always be sent as well.) For more information about JITR, see:
 *  https://docs.aws.amazon.com/iot/latest/developerguide/jit-provisioning.html,
 *  https://aws.amazon.com/blogs/iot/just-in-time-registration-of-device-certificates-on-aws-iot/.
 *
 * If you're not using JITR, set below to NULL.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----\n"
 */
#define BK_JITR_DEVICE_CERTIFICATE_AUTHORITY_PEM    ""

/*
 * @brief PEM-encoded client private key.
 *
 * @todo If you are running one of the Amazon FreeRTOS demo projects, set this
 * to the private key that will be used for TLS client authentication.
 *
 * @note Must include the PEM header and footer:
 * "-----BEGIN RSA PRIVATE KEY-----\n"\
 * "...base64 data...\n"\
 * "-----END RSA PRIVATE KEY-----\n"
 */
/* 20902cfd72-private.pem.key */
#define BK_CLIENT_PRIVATE_KEY_PEM "-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEpAIBAAKCAQEA1Odb2s5gBv/p8eriloMjvYE1SLsiMlj5R4YYTNvqRC/EvxSl\n"\
"RjLLju6nDqZ69lrpL92tGWhct64MmoIrio0m5sfjZ7A5qcl1qUHkQSizw8TUrZ77\n"\
"cF2UcHeeIZW33XMBuCegqb1zaHuTgf6Ml0LaEJm6P7JyJaGXgtdgNoTouX5Q1Org\n"\
"ZeVUpAvnJ1UXrg6M4n9WMwgOAXwK2kb79uVFchD5TrbWmOkcmsUIyAEyNjpbe5tP\n"\
"+qu21KwjbpcNW5N4Enva/l3pOJ9KsOHXVnUIRXz11jUrJ0VmZxhaf1NMop93yENa\n"\
"hleWwMdf6iXWpsihD4i0ipKfDMivDpI7QGeRpQIDAQABAoIBAQCoGINNnuPyotvz\n"\
"RaDhdWkiloGbiyTU26r17coB5SBx9cVBmOtlIXXWxFbiGk+7csvqHvwss1mBLMqG\n"\
"s3/tRMUSMYA6vcjJZWag3IV7bMeCKkZBTuT3MuugYNFZcvxYvBT/cUpIumwEco1k\n"\
"dbZLN100/isvX5UAYTVe+O3eHdIhoUlIg1Xol9IVbnaW+XioxkWy9xRxBT7a2ONZ\n"\
"lrRrp5s6EYhyXEdIgE4xE96+PlKJrbm2WjAADQUuCD3gNF+ylRk31kaoloaegm6h\n"\
"Dpvh78mIxxjyxfUZtZaNu+XzySH1uTB/lfi6tet+BTNOwIaDoVmh3udyIK3DJR4n\n"\
"aVz957QBAoGBAOoJwnQ1msWkbfHR4Ox7fQkPkRzba4oA/WWnFtcXDsUFhsh7STEw\n"\
"CkPWs3NYqwcpXKo4rPuPMPVnQUoUihq+T024lLvMuWZpd0DeI7w4YbhB3kYJX/1U\n"\
"Hj5apCN/fj0fRfo4ecVp89Tb+C6/nDiwt3rpHlFHhBYssfY2Wg2jRz0NAoGBAOjh\n"\
"5KuddnBgqcXPwQisUfU2h0feDfN1Drk3BGEGAxz9+IadayT9flGifr6qj98EodTc\n"\
"4DVCfWvNSqbOe3Q+A6vzE+7pnzYUS+lZzSlIw3auNEnfrY+CMxuu9INjp/2g+qCV\n"\
"7giZtHgE3YNkisdC+HcI2Nj3VzP/gCfDcU8kJfD5AoGBAMrPGktKCI0tKHldvuQv\n"\
"PvMTIlU3b8FviicE3K4XtRzW5S3maE7PqpWPAIL8W3khRsPbyUtVkr+WcuWXVvZF\n"\
"5MMdKQZ1KlStIEJ+PcllsojRy6Q1i5Ejy/GM6qA4Y1TkPOfQ+PUyE7JpzG/2a5JU\n"\
"0SsZyMP2jWgJ403RW8hlrd4hAoGAd4m8PvsMmJKFkqwZgcIyL5RVzGYG8zja3eeH\n"\
"r+XOI0uaDj8viEU2WeD2/he+0dMm3oSh8bS3fGZcM1M5u2k5qUGUscXpm3C/poAZ\n"\
"918KNhklbeYKyOckJMmhaO/2gxHmlBdhn7iGEjUtHwy0z6NotnEsHfKYKHC177M6\n"\
"rkz3zMECgYBL6qhlOZbTMQ4Fr7cwjWqfqHDrDdWVDfN5HVjZPIvrnx0FQxJ6y+47\n"\
"fxA5mxfAdmaRqe+MYHywlkk3raEfTBfWfrFWKLRfk09aZUMOr4bYjtW33jwYUcQD\n"\
"GQLynsfU384TOHdreiwKh77iLYRcTDQSTSs84BiPdCt3ikEW6emlEg==\n"\
"-----END RSA PRIVATE KEY-----\n"

#endif /* BK_CLIENT_CREDENTIAL_KEYS_H */
