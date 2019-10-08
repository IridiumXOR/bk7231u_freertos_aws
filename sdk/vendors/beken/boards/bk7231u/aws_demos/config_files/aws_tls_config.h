#ifndef MBEDTLS_USER_CONFIG_H
#define MBEDTLS_USER_CONFIG_H

#if defined(__ARM_FEATURE_DSP)
#undef __ARM_FEATURE_DSP
#endif
#define __ARM_FEATURE_DSP 0

#if defined(MBEDTLS_SSL_MAX_CONTENT_LEN)
#undef MBEDTLS_SSL_MAX_CONTENT_LEN
#endif
#define MBEDTLS_SSL_MAX_CONTENT_LEN             8192

//#define MBEDTLS_DEBUG_C

#endif /* MBEDTLS_USER_CONFIG_H */
