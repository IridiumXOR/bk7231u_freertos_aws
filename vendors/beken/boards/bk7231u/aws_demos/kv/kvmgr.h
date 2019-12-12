/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _key_value_h_
#define _key_value_h

#include <errno.h>
#include "BkDriverFlash.h"

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

/* The totally storage size for key-value store */
#define KV_TOTAL_SIZE   (16 * 1024)

#if defined(BK_FLASH_PARTITION)
/* The physical parition for key-value store */
#define KV_PTN    BK_PARTITION_NET_PARAM
#else
#define BK_KV_ADDRESS 0x1FA000
#endif

/* Key-value function return code description */
typedef enum {
    RES_OK              = 0,        /* Successed */
    RES_CONT            = -EAGAIN,  /* Loop continued */
    RES_NO_SPACE        = -ENOSPC,  /* The space is out of range */
    RES_INVALID_PARAM   = -EINVAL,  /* The parameter is invalid */
    RES_MALLOC_FAILED   = -ENOMEM,  /* Error related to malloc */
    RES_ITEM_NOT_FOUND  = -ENOENT,  /* Could not find the key-value item */
    RES_FLASH_READ_ERR  = -EIO,     /* The flash read operation failed */
    RES_FLASH_WRITE_ERR = -EIO,     /* The flash write operation failed */
    RES_FLASH_EARSE_ERR = -EIO      /* The flash earse operation failed */
} result_e;

/**
 * @brief init the kv module.
 *
 * @param[in] none.
 *
 * @note: the default KV size is @HASH_TABLE_MAX_SIZE, the path to store
 *        the kv file is @KVFILE_PATH.
 * @retval  0 on success, otherwise -1 will be returned
 */
int aos_kv_init();

/**
 * @brief deinit the kv module.
 *
 * @param[in] none.
 *
 * @note: all the KV in RAM will be released.
 * @retval none.
 */
void aos_kv_deinit();

int aos_kv_del(const char *key);
int aos_kv_set(const char *key, const void *val, int len, int sync);
int aos_kv_get(const char *key, void *buffer, int *buffer_len);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif


