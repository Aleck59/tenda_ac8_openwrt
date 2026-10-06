#ifndef __MBEDTLS_LK_H__
#define __MBEDTLS_LK_H__

#ifndef CHAR_BIT
#define CHAR_BIT 8 /* Normally in <limits.h> */
#endif

#include "config.h"

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/string.h>

#include <linux/vmalloc.h>
#endif

extern void *calloc(size_t n, size_t size);

#ifdef __OSK__
__DBG_NOMIPS16 __IRAM_SYS_LOW
#endif
extern void free(void *ptr);

#if defined(MBEDTLS_SELF_TEST) && defined(MBEDTLS_PKCS1_V15)
#ifdef __KERNEL__
#include <linux/random.h>
#endif

extern int rand(void);

#endif /* MBEDTLS_SELF_TEST && MBEDTLS_PKCS1_V15 */

#endif /* __MBEDTLS_LK_H__ */
