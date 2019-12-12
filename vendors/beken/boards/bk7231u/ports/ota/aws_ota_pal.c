/*
 * Amazon FreeRTOS OTA PAL for Windows Simulator V1.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/* OTA PAL implementation for Windows platform. */

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "aws_crypto.h"
#include "aws_pkcs11.h"
#include "aws_ota_pal.h"
#include "aws_ota_agent_internal.h"
#include "aws_ota_codesigner_certificate.h"

#include "BkDriverFlash.h"
#include "flash.h"

/* definitions shared with the resident bootloader. */
#define AWS_OTA_IMAGE_MAGIC         "@BK"
#define AWS_OTA_IMAGE_MAGIC_SIZE    ( 3U )

#define AWS_OTA_FLAG_IMG_NEW               0xffU /* 11111111b A new image that hasn't yet been run. */
#define AWS_OTA_FLAG_IMG_PENDING_COMMIT    0xfeU /* 11111110b Image is pending commit and is ready for self test. */
#define AWS_OTA_FLAG_IMG_VALID             0xfcU /* 11111100b The image was accepted as valid by the self test code. */
#define AWS_OTA_FLAG_IMG_INVALID           0xf8U /* 11111000b The image was NOT accepted by the self test code. */

typedef struct
{
    const OTA_FileContext_t * pxCurOTAFile; /* Current OTA file to be processed. */
    uint32_t ulPartitionBegin;                 /* Begin address in the ota partition. */
    uint32_t ulPartitionEnd;                    /* End address in the ota partition. */
    uint32_t ulLowImageOffset;              /* Lowest offset/address in the application image. */
    uint32_t ulHighImageOffset;             /* Highest offset/address in the application image. */
} OTA_BekenContext_t;

typedef struct
{
    char cImgMagic[ AWS_OTA_IMAGE_MAGIC_SIZE ];
    uint8_t ucImgFlags;
} OTA_ImageDescriptor_t;

/* NOTE that this implementation supports only one OTA at a time since it uses a single static instance. */
static OTA_BekenContext_t xCurrentOTAContext;         /* current OTA operation in progress. */

/* Specify the OTA signature algorithm we support on this platform. */
const char cOTA_JSON_FileSignatureKey[ OTA_FILE_SIG_KEY_STR_MAX_LENGTH ] = "sig-sha256-ecdsa";

static OTA_Err_t prvPAL_CheckFileSignature( OTA_FileContext_t * const C );

/*-----------------------------------------------------------*/

static inline BaseType_t prvContextValidate( OTA_FileContext_t * C )
{
    return( ( C != NULL ) &&
            ( C == xCurrentOTAContext.pxCurOTAFile ) &&
            ( C->pucFile == ( uint8_t * ) &xCurrentOTAContext ) ); /*lint !e9034 This preserves the abstraction layer. */
}

static inline void prvContextClose( OTA_FileContext_t * C )
{
    if( NULL != C )
    {
        C->pucFile = NULL;
    }

    xCurrentOTAContext.pxCurOTAFile = NULL;

    bk_flash_enable_security(FLASH_UNPROTECT_LAST_BLOCK); // last or custom
}

/* Used to set the high bit of Windows error codes for a negative return value. */
#define OTA_PAL_INT16_NEGATIVE_MASK    ( 1 << 15 )

/* Size of buffer used in file operations on this platform (Windows). */
#define OTA_PAL_WIN_BUF_SIZE ( ( size_t ) 4096UL )

/* Attempt to create a new receive file for the file chunks as they come in. */

int prvPAL_EraseBlock(uint32_t addr, uint32_t size)
{
    uint32_t _size = size;
    uint32_t _addr;
    uint8_t buffer[128];

    /* Calculate the start address of the flash sector(4kbytes) */
    _addr = addr & 0x00FFF000;

    do{
        flash_ctrl(CMD_FLASH_ERASE_SECTOR, &_addr);
        _addr += 4096;
        if (_size < 4096)
            _size = 0;
        else
            _size -= 4096;

        rtos_delay_milliseconds(5); // delay 5ms every 4K

    } while (_size);

    return _addr - addr; // return true erase size
}

OTA_Err_t prvPAL_CreateFileForRx( OTA_FileContext_t * const C )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_CreateFileForRx" );

    OTA_Err_t eResult = kOTA_Err_Uninitialized; /* For MISRA mandatory. */

    if( C != NULL )
    {
        bk_logic_partition_t *pt = bk_flash_get_info(BK_PARTITION_OTA);
        if( pt != NULL )
        {
            if (C->ulFileSize + sizeof(OTA_ImageDescriptor_t) <= pt->partition_length)
            {
                bk_flash_enable_security(FLASH_PROTECT_HALF); // half or custom

                /* since AWS write data with OTA_FILE_BLOCK_SIZE(1024), other than 4096,
                    we should erase flash first */
                prvPAL_EraseBlock(pt->partition_start_addr, pt->partition_length);
                xCurrentOTAContext.pxCurOTAFile = C;
                xCurrentOTAContext.ulPartitionBegin = pt->partition_start_addr;
                xCurrentOTAContext.ulPartitionEnd = pt->partition_start_addr + pt->partition_length;
                xCurrentOTAContext.ulHighImageOffset = 0;
                xCurrentOTAContext.ulLowImageOffset = xCurrentOTAContext.ulPartitionEnd;
                C->pucFile = (uint8_t *)&xCurrentOTAContext;
                eResult = kOTA_Err_None;
                OTA_LOG_L1( "[%s] Receive file created.\r\n", OTA_METHOD_NAME );
            }
            else
            {
                eResult = kOTA_Err_RxFileTooLarge;
                OTA_LOG_L1( "[%s] ERROR - Invalid patition table.\r\n", OTA_METHOD_NAME );
            }
        }
        else
        {
            eResult = kOTA_Err_RxFileCreateFailed;
            OTA_LOG_L1( "[%s] ERROR - Invalid patition table.\r\n", OTA_METHOD_NAME );
        }
    }
    else
    {
        eResult = kOTA_Err_RxFileCreateFailed;
        OTA_LOG_L1( "[%s] ERROR - Invalid context provided.\r\n", OTA_METHOD_NAME );
    }

    return eResult; /*lint !e480 !e481 Exiting function without calling fclose.
                     * Context file handle state is managed by this API. */
}


/* Abort receiving the specified OTA update by closing the file. */

OTA_Err_t prvPAL_Abort( OTA_FileContext_t * const C )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_Abort" );

    /* Set default return status to uninitialized. */
    OTA_Err_t eResult = kOTA_Err_Uninitialized;

    if( prvContextValidate( C ) == pdTRUE )
    {
        /* Close the OTA update file if it's open. */
        prvContextClose(C);
        OTA_LOG_L1( "[%s] OK\r\n", OTA_METHOD_NAME );
        eResult = kOTA_Err_None;
    }
    else /* Context was not valid. */
    {
        OTA_LOG_L1( "[%s] ERROR - Invalid context.\r\n", OTA_METHOD_NAME );
        eResult = kOTA_Err_None;
    }

    return eResult;
}

/* Write a block of data to the specified file. */
int16_t prvPAL_WriteBlock( OTA_FileContext_t * const C,
                           uint32_t ulOffset,
                           uint8_t * const pacData,
                           uint32_t ulBlockSize )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_WriteBlock" );

    int32_t lResult = 0;

    if( prvContextValidate( C ) == pdFALSE )
    {
        /* Invalid context or file pointer provided. */
        OTA_LOG_L1( "[%s] ERROR - Invalid context.\r\n", OTA_METHOD_NAME );
        lResult = -1; /*TODO: Need a negative error code from the PAL here. */
    }
    else if( ( ulOffset + ulBlockSize ) > xCurrentOTAContext.ulPartitionEnd )
    {
        /* invalid address. */
        OTA_LOG_L1( "[%s] ERROR - Invalid ulOffset=%d ulBlockSize=%d.\r\n", OTA_METHOD_NAME, ulOffset, ulBlockSize );
        lResult = -1;
    }
    else /* Update the image offsets. */
    {
        if( ulOffset < xCurrentOTAContext.ulLowImageOffset )
        {
            xCurrentOTAContext.ulLowImageOffset = ulOffset;
        }

        if( ( ulOffset + ulBlockSize ) > xCurrentOTAContext.ulHighImageOffset )
        {
            xCurrentOTAContext.ulHighImageOffset = ulOffset + ulBlockSize;
        }

        lResult = flash_write( pacData, ulBlockSize, xCurrentOTAContext.ulPartitionBegin + ulOffset ); /*lint !e586 !e713 !e9034
                                                            * C standard library call is being used for portability. */

        if( FLASH_SUCCESS == lResult )
        {
#if 1
            /* read back for verify. */
            uint8_t *pacVerifyBuffer = pvPortMalloc( ulBlockSize );
            if (NULL != pacVerifyBuffer)
            {
                lResult = flash_read(pacVerifyBuffer, ulBlockSize, xCurrentOTAContext.ulPartitionBegin + ulOffset);

                if (FLASH_SUCCESS != lResult)
                {
                    OTA_LOG_L1( "[%s] ERROR - read failed\r\n", OTA_METHOD_NAME );
                    lResult = -1;
                }
                else if (memcmp(pacVerifyBuffer, pacData, ulBlockSize) != 0)
                {
                    OTA_LOG_L1( "[%s] ERROR - verify failed\r\n", OTA_METHOD_NAME );
                    lResult = -1;
                }
                vPortFree( pacVerifyBuffer );
            }
#endif

            if (FLASH_SUCCESS == lResult)
            {
                lResult = ulBlockSize; /*lint !e586 !e713 !e9034
                                                                      * C standard library call is being used for portability. */
            }
        }
        else
        {
            OTA_LOG_L1( "[%s] ERROR - write failed\r\n", OTA_METHOD_NAME );
            lResult = -1; /*TODO: Need a negative error code from the PAL here. */
        }
    }

    return ( int16_t ) lResult;
}

/* Close the specified file. This shall authenticate the file if it is marked as secure. */

OTA_Err_t prvPAL_CloseFile( OTA_FileContext_t * const C )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_CloseFile" );

    OTA_Err_t eResult = kOTA_Err_None;
    int32_t lWindowsError = 0;

    if( prvContextValidate( C ) == pdTRUE )
    {
        if( ( C->pxSignature != NULL ) &&
            ( xCurrentOTAContext.ulHighImageOffset > xCurrentOTAContext.ulLowImageOffset ) )
        {
            /* Verify the file signature, close the file and return the signature verification result. */
            eResult = prvPAL_CheckFileSignature( C );
        }
        else
        {
            OTA_LOG_L1( "[%s] ERROR - NULL OTA Signature structure.\r\n", OTA_METHOD_NAME );
            eResult = kOTA_Err_SignatureCheckFailed;
        }

        if( eResult == kOTA_Err_None )
        {
            OTA_ImageDescriptor_t imageDesc;

            memset((void *)&imageDesc, 0x0, sizeof(imageDesc));
            memcpy( imageDesc.cImgMagic, AWS_OTA_IMAGE_MAGIC, sizeof( imageDesc.cImgMagic ) );
            imageDesc.ucImgFlags = AWS_OTA_FLAG_IMG_PENDING_COMMIT;
            flash_write( (void *)&imageDesc, sizeof(imageDesc), xCurrentOTAContext.ulPartitionEnd - sizeof(imageDesc) );

            OTA_LOG_L1( "[%s] %s signature verification passed.\r\n", OTA_METHOD_NAME, cOTA_JSON_FileSignatureKey );
        }
        else
        {
            OTA_LOG_L1( "[%s] ERROR - Failed to pass %s signature verification: %d.\r\n", OTA_METHOD_NAME,
                        cOTA_JSON_FileSignatureKey, eResult );

            /* If we fail to verify the file signature that means the image is not valid. We need to set the image state to aborted. */
            prvPAL_SetPlatformImageState( eOTA_ImageState_Aborted );

        }

        prvContextClose(C);
    }
    else /* Invalid OTA Context. */
    {
        /* FIXME: Invalid error code for a null file context and file handle. */
        OTA_LOG_L1( "[%s] ERROR - Invalid context.\r\n", OTA_METHOD_NAME );
        eResult = kOTA_Err_FileClose;
    }

    return eResult;
}

static CK_RV prvGetCertificateHandle( CK_FUNCTION_LIST_PTR pxFunctionList,
                                      CK_SESSION_HANDLE xSession,
                                      const char * pcLabelName,
                                      CK_OBJECT_HANDLE_PTR pxCertHandle )
{
    CK_ATTRIBUTE xTemplate;
    CK_RV xResult = CKR_OK;
    CK_ULONG ulCount = 0;
    CK_BBOOL xFindInit = CK_FALSE;

    /* Get the certificate handle. */
    if( 0 == xResult )
    {
        xTemplate.type = CKA_LABEL;
        xTemplate.ulValueLen = strlen( pcLabelName ) + 1;
        xTemplate.pValue = ( char * ) pcLabelName;
        xResult = pxFunctionList->C_FindObjectsInit( xSession, &xTemplate, 1 );
    }

    if( 0 == xResult )
    {
        xFindInit = CK_TRUE;
        xResult = pxFunctionList->C_FindObjects( xSession,
                                                 ( CK_OBJECT_HANDLE_PTR ) pxCertHandle,
                                                 1,
                                                 &ulCount );
    }

    if( CK_TRUE == xFindInit )
    {
        xResult = pxFunctionList->C_FindObjectsFinal( xSession );
    }

    return xResult;
}

/* Note that this function mallocs a buffer for the certificate to reside in,
 * and it is the responsibility of the caller to free the buffer. */
static CK_RV prvGetCertificate( const char * pcLabelName,
                                uint8_t ** ppucData,
                                uint32_t * pulDataSize )
{
    /* Find the certificate */
    CK_OBJECT_HANDLE xHandle;
    CK_RV xResult;
    CK_FUNCTION_LIST_PTR xFunctionList;
    CK_SLOT_ID xSlotId;
    CK_ULONG xCount = 1;
    CK_SESSION_HANDLE xSession;
    CK_ATTRIBUTE xTemplate = { 0 };
    uint8_t * pucCert = NULL;
    CK_BBOOL xSessionOpen = CK_FALSE;

    xResult = C_GetFunctionList( &xFunctionList );

    if( CKR_OK == xResult )
    {
        xResult = xFunctionList->C_Initialize( NULL );
    }

    if( ( CKR_OK == xResult ) || ( CKR_CRYPTOKI_ALREADY_INITIALIZED == xResult ) )
    {
        xResult = xFunctionList->C_GetSlotList( CK_TRUE, &xSlotId, &xCount );
    }

    if( CKR_OK == xResult )
    {
        xResult = xFunctionList->C_OpenSession( xSlotId, CKF_SERIAL_SESSION, NULL, NULL, &xSession );
    }

    if( CKR_OK == xResult )
    {
        xSessionOpen = CK_TRUE;
        xResult = prvGetCertificateHandle( xFunctionList, xSession, pcLabelName, &xHandle );
    }

    if( ( xHandle != 0 ) && ( xResult == CKR_OK ) ) /* 0 is an invalid handle */
    {
        /* Get the length of the certificate */
        xTemplate.type = CKA_VALUE;
        xTemplate.pValue = NULL;
        xResult = xFunctionList->C_GetAttributeValue( xSession, xHandle, &xTemplate, xCount );

        if( xResult == CKR_OK )
        {
            pucCert = pvPortMalloc( xTemplate.ulValueLen );
        }

        if( ( xResult == CKR_OK ) && ( pucCert == NULL ) )
        {
            xResult = CKR_HOST_MEMORY;
        }

        if( xResult == CKR_OK )
        {
            xTemplate.pValue = pucCert;
            xResult = xFunctionList->C_GetAttributeValue( xSession, xHandle, &xTemplate, xCount );

            if( xResult == CKR_OK )
            {
                *ppucData = pucCert;
                *pulDataSize = xTemplate.ulValueLen;
            }
            else
            {
                vPortFree( pucCert );
            }
        }
    }
    else /* Certificate was not found. */
    {
        *ppucData = NULL;
        *pulDataSize = 0;
    }

    if( xSessionOpen == CK_TRUE )
    {
        ( void ) xFunctionList->C_CloseSession( xSession );
    }

    return xResult;
}

/* Read the specified signer certificate from the filesystem into a local buffer. The
 * allocated memory becomes the property of the caller who is responsible for freeing it.
 */
static uint8_t * prvPAL_ReadAndAssumeCertificate( const uint8_t * const pucCertName,
                                      uint32_t * const ulSignerCertSize )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_ReadAndAssumeCertificate" );

    uint8_t * pucCertData;
    uint32_t ulCertSize;
    uint8_t * pucSignerCert = NULL;
    CK_RV xResult;

    xResult = prvGetCertificate( ( const char * ) pucCertName, &pucSignerCert, ulSignerCertSize );

    if( ( xResult == CKR_OK ) && ( pucSignerCert != NULL ) )
    {
        OTA_LOG_L1( "[%s] Using cert with label: %s OK\r\n", OTA_METHOD_NAME, ( const char * ) pucCertName );
    }
    else
    {
        OTA_LOG_L1( "[%s] No such certificate file: %s. Using aws_ota_codesigner_certificate.h.\r\n", OTA_METHOD_NAME,
                    ( const char * ) pucCertName );

        /* Allocate memory for the signer certificate plus a terminating zero so we can copy it and return to the caller. */
        ulCertSize = sizeof( signingcredentialSIGNING_CERTIFICATE_PEM );
        pucSignerCert = pvPortMalloc( ulCertSize + 1 );                       /*lint !e9029 !e9079 !e838 malloc proto requires void*. */
        pucCertData = ( uint8_t * ) signingcredentialSIGNING_CERTIFICATE_PEM; /*lint !e9005 we don't modify the cert but it could be set by PKCS11 so it's not const. */

        if( pucSignerCert != NULL )
        {
            memcpy( pucSignerCert, pucCertData, ulCertSize );
            *ulSignerCertSize = ulCertSize;
        }
        else
        {
            OTA_LOG_L1( "[%s] Error: No memory for certificate of size %d!\r\n", OTA_METHOD_NAME, ulCertSize );
        }
    }

    return pucSignerCert;
}

/* Verify the signature of the specified file. */

static OTA_Err_t prvPAL_CheckFileSignature( OTA_FileContext_t * const C )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_CheckFileSignature" );

    OTA_Err_t eResult = kOTA_Err_None;
    uint32_t ulSignerCertSize;
    uint8_t * pucSignerCert = NULL;
    void * pvSigVerifyContext;

    if( prvContextValidate( C ) == pdTRUE )
    {
        /* Verify an ECDSA-SHA256 signature. */
        if( CRYPTO_SignatureVerificationStart( &pvSigVerifyContext, cryptoASYMMETRIC_ALGORITHM_ECDSA,
                                               cryptoHASH_ALGORITHM_SHA256 ) == pdFALSE )
        {
            eResult = kOTA_Err_SignatureCheckFailed;
        }
        else
        {
            OTA_LOG_L1( "[%s] Started %s signature verification, file: %s\r\n", OTA_METHOD_NAME,
                        cOTA_JSON_FileSignatureKey, ( const char * ) C->pucCertFilepath );
            pucSignerCert = prvPAL_ReadAndAssumeCertificate( ( const uint8_t * const ) C->pucCertFilepath, &ulSignerCertSize );

            if( pucSignerCert == NULL )
            {
                eResult = kOTA_Err_BadSignerCert;
            }
            else
            {
                uint32_t fw_start_addr, fw_end_addr, index_addr;
                uint8_t buf[128];

                /* TODO: should skip the header */
                fw_start_addr = xCurrentOTAContext.ulPartitionBegin;
                fw_end_addr = fw_start_addr + xCurrentOTAContext.ulHighImageOffset - xCurrentOTAContext.ulLowImageOffset;

                /* calculate CRC32 */
                for (index_addr = fw_start_addr; index_addr <= fw_end_addr - sizeof(buf); index_addr += sizeof(buf))
                {
                    flash_read(buf, sizeof(buf), index_addr);
                    CRYPTO_SignatureVerificationUpdate( pvSigVerifyContext, (const uint8_t *)buf, sizeof(buf));
                }

                /* align process */
                if (index_addr != fw_end_addr - sizeof(buf))
                {
                    flash_read(buf, fw_end_addr - index_addr, index_addr);
                    CRYPTO_SignatureVerificationUpdate( pvSigVerifyContext, (const uint8_t *)buf, fw_end_addr - index_addr);
                }

                if( CRYPTO_SignatureVerificationFinal( pvSigVerifyContext, ( char * ) pucSignerCert, ulSignerCertSize,
                                                       C->pxSignature->ucData, C->pxSignature->usSize ) == pdFALSE )
                {
                    eResult = kOTA_Err_SignatureCheckFailed;

                    OTA_LOG_L1( "[%s] Error: Failed to verification signature !\r\n", OTA_METHOD_NAME );
                }
                else
                {
                    eResult = kOTA_Err_None;
                }
            }
        }

        /* Free the signer certificate that we now own after prvPAL_ReadAndAssumeCertificate(). */
        if( pucSignerCert != NULL )
        {
            vPortFree( pucSignerCert );
        }
    }
    else
    {
        /* FIXME: Invalid error code for a NULL file context. */
        OTA_LOG_L1( "[%s] ERROR - Invalid OTA file context.\r\n", OTA_METHOD_NAME );
        /* Invalid OTA context or file pointer. */
        eResult = kOTA_Err_NullFilePtr;
    }

    return eResult;
}

/*-----------------------------------------------------------*/

OTA_Err_t prvPAL_ResetDevice( void )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_ResetDevice" );

    OTA_LOG_L1( "[%s] Resetting the device.\r\n", OTA_METHOD_NAME );

    /* Short delay for debug log output before reset. */
    rtos_delay_milliseconds( 500UL );

    extern void reboot(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
    reboot(NULL, 0, 0, NULL);

    /* We shouldn't actually get here if the board supports the auto reset.
     * But, it doesn't hurt anything if we do although someone will need to
     * reset the device for the new image to boot. */
    return kOTA_Err_None;
}

/*-----------------------------------------------------------*/

OTA_Err_t prvPAL_ActivateNewImage( void )
{
    DEFINE_OTA_METHOD_NAME("prvPAL_ActivateNewImage");

    OTA_LOG_L1( "[%s] Activating the new MCU image.\r\n", OTA_METHOD_NAME );
    return prvPAL_ResetDevice();
}


/*
 * Set the final state of the last transferred (final) OTA file (or bundle).
 * On Windows, the state of the OTA image is stored in PlaformImageState.txt.
 */

OTA_Err_t prvPAL_SetPlatformImageState( OTA_ImageState_t eState )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_SetPlatformImageState" );

    OTA_ImageDescriptor_t imageDesc;
    OTA_Err_t eResult = kOTA_Err_Uninitialized;
    int32_t lResult = 0;

    if (NULL == xCurrentOTAContext.pxCurOTAFile)
    {
        OTA_LOG_L1( "[%s] Uninitialized.\r\n", OTA_METHOD_NAME );
        return eResult;
    }

    lResult = flash_read( (void *)&imageDesc, sizeof(imageDesc), xCurrentOTAContext.ulPartitionEnd - sizeof(imageDesc) );

    /* This should be an image launched in self test mode! */
    if( imageDesc.ucImgFlags == AWS_OTA_FLAG_IMG_PENDING_COMMIT )
    {
        if( eState == eOTA_ImageState_Accepted )
        {
            /* Mark the image as valid */
            imageDesc.ucImgFlags = AWS_OTA_FLAG_IMG_VALID;

            lResult = flash_write( (void *)&imageDesc, sizeof(imageDesc), xCurrentOTAContext.ulPartitionEnd - sizeof(imageDesc) );
            if( lResult == FLASH_SUCCESS )
            {
                OTA_LOG_L1( "[%s] Accepted and committed final image.\r\n", OTA_METHOD_NAME );

                eResult = kOTA_Err_None;
            }
            else
            {
                OTA_LOG_L1( "[%s] Accepted final image but commit failed.\r\n", OTA_METHOD_NAME );
                eResult = ( uint32_t ) kOTA_Err_CommitFailed;
            }
        }
        else if( eState == eOTA_ImageState_Rejected )
        {
            /* Mark the image as invalid */
            imageDesc.ucImgFlags = AWS_OTA_FLAG_IMG_INVALID;

            lResult = flash_write( (void *)&imageDesc, sizeof(imageDesc), xCurrentOTAContext.ulPartitionEnd - sizeof(imageDesc) );
            if( lResult == FLASH_SUCCESS )
            {
                OTA_LOG_L1( "[%s] Rejected image.\r\n", OTA_METHOD_NAME );

                eResult = kOTA_Err_None;
            }
            else
            {
                OTA_LOG_L1( "[%s] Rejected final image but commit failed.\r\n", OTA_METHOD_NAME );
                eResult = ( uint32_t ) kOTA_Err_RejectFailed;
            }
        }
        else if( eState == eOTA_ImageState_Aborted )
        {
            /* Mark the image as invalid */
            imageDesc.ucImgFlags = AWS_OTA_FLAG_IMG_INVALID;

            lResult = flash_write( (void *)&imageDesc, sizeof(imageDesc), xCurrentOTAContext.ulPartitionEnd - sizeof(imageDesc) );
            if( lResult == FLASH_SUCCESS )
            {
                OTA_LOG_L1( "[%s] Aborted image.\r\n", OTA_METHOD_NAME );

                eResult = kOTA_Err_None;
            }
            else
            {
                OTA_LOG_L1( "[%s] Aborted final image but commit failed.\r\n", OTA_METHOD_NAME );
                eResult = ( uint32_t ) kOTA_Err_AbortFailed;
            }
        }
        else if( eState == eOTA_ImageState_Testing )
        {
            eResult = kOTA_Err_None;
        }
        else
        {
            OTA_LOG_L1( "[%s] Unknown state received %d.\r\n", OTA_METHOD_NAME, ( int32_t ) eState );
            eResult = kOTA_Err_BadImageState;
        }
    }
    else
    {
        /* Not in self-test mode so get the descriptor for image in upper bank. */

        if( eState == eOTA_ImageState_Accepted )
        {
            /* We are not in self-test mode so can not set the image in upper bank as valid.  */
            OTA_LOG_L1( "[%s] Not in commit pending so can not mark image valid.\r\n", OTA_METHOD_NAME );
            eResult = ( uint32_t ) kOTA_Err_CommitFailed;
        }
        else if( eState == eOTA_ImageState_Rejected )
        {
            OTA_LOG_L1( "[%s] Rejected image.\r\n", OTA_METHOD_NAME );

            /* The OTA on program image bank (upper bank) is rejected so erase the bank.  */
            lResult = prvPAL_EraseBlock(xCurrentOTAContext.ulPartitionBegin, xCurrentOTAContext.ulPartitionEnd - xCurrentOTAContext.ulPartitionBegin);
            if(lResult  != xCurrentOTAContext.ulPartitionEnd - xCurrentOTAContext.ulPartitionBegin )
            {
                OTA_LOG_L1( "[%s] Error: Failed to erase the flash! (%d).\r\n", OTA_METHOD_NAME );
                eResult = ( uint32_t ) kOTA_Err_RejectFailed;
            }
            else
            {
                eResult = kOTA_Err_None;
            }
        }
        else if( eState == eOTA_ImageState_Aborted )
        {
            OTA_LOG_L1( "[%s] Aborted image.\r\n", OTA_METHOD_NAME );

            /* The OTA on program image bank (upper bank) is aborted so erase the bank.  */
            lResult = prvPAL_EraseBlock(xCurrentOTAContext.ulPartitionBegin, xCurrentOTAContext.ulPartitionEnd - xCurrentOTAContext.ulPartitionBegin);
            if(lResult  != xCurrentOTAContext.ulPartitionEnd - xCurrentOTAContext.ulPartitionBegin )
            {
                OTA_LOG_L1( "[%s] Error: Failed to erase the flash! (%d).\r\n", OTA_METHOD_NAME );
                eResult = ( uint32_t ) kOTA_Err_AbortFailed;
            }
            else
            {
                eResult = kOTA_Err_None;
            }
        }
        else if( eState == eOTA_ImageState_Testing )
        {
            eResult = kOTA_Err_None;
        }
        else
        {
            OTA_LOG_L1( "[%s] ERROR - Invalid image state provided.\r\n", OTA_METHOD_NAME );
            eResult = kOTA_Err_BadImageState;
        }
    }

    return eResult;
}

/* Get the state of the currently running image.
 *
 * On Windows, this is simulated by looking for and reading the state from
 * the PlatformImageState.txt file in the current working directory.
 *
 * We read this at OTA_Init time so we can tell if the MCU image is in self
 * test mode. If it is, we expect a successful connection to the OTA services
 * within a reasonable amount of time. If we don't satisfy that requirement,
 * we assume there is something wrong with the firmware and reset the device,
 * causing it to rollback to the previous code. On Windows, this is not
 * fully simulated as there is no easy way to reset the simulated device.
 */
OTA_PAL_ImageState_t prvPAL_GetPlatformImageState( void )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_GetPlatformImageState" );

    OTA_ImageDescriptor_t imageDesc;
    OTA_PAL_ImageState_t eImageState = eOTA_PAL_ImageState_Invalid;
    int32_t lResult = 0;

    if (NULL == xCurrentOTAContext.pxCurOTAFile)
    {
        OTA_LOG_L1( "[%s] Uninitialized.\r\n", OTA_METHOD_NAME );
        return eImageState;
    }

    lResult = flash_read( (void *)&imageDesc, sizeof(imageDesc), xCurrentOTAContext.ulPartitionEnd - sizeof(imageDesc) );

    /**
     *  Check if valid magic code is present for the application image in lower bank.
     */
    if( memcmp( imageDesc.cImgMagic,
                AWS_OTA_IMAGE_MAGIC,
                AWS_OTA_IMAGE_MAGIC_SIZE ) == 0 )
    {
        switch( imageDesc.ucImgFlags )
        {
            case AWS_OTA_FLAG_IMG_PENDING_COMMIT:
                /* Pending Commit means we're in the Self Test phase. */
                eImageState = eOTA_PAL_ImageState_PendingCommit;
                break;

            case AWS_OTA_FLAG_IMG_VALID:
            case AWS_OTA_FLAG_IMG_NEW:
                eImageState = eOTA_PAL_ImageState_Valid;
                break;

            default:
                eImageState = eOTA_PAL_ImageState_Invalid;
                break;
        }
    }

    return eImageState;
}

/*-----------------------------------------------------------*/

/* Provide access to private members for testing. */
#ifdef AMAZON_FREERTOS_ENABLE_UNIT_TESTS
#include "aws_ota_pal_test_access_define.h"
#endif
