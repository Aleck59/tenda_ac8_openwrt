#include "lk.h"
#if defined(CONFIG_RTL8192CD) || defined(CONFIG_RTL8192CD_MODULE)
#include "../8192cd.h"
#include "../8192cd_headers.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif

#ifdef __OSK__
        #ifdef calloc
                #undef calloc
        #endif
#endif
//#ifndef __OSK__
void *calloc(size_t n, size_t size)
{
	return kzalloc(n * size, GFP_ATOMIC);
}
//#endif
#ifdef __OSK__
__DBG_NOMIPS16 __IRAM_SYS_LOW extern void free(void *ptr);
#else
void free(void *ptr)
{
	kfree(ptr);
}
#endif

#if defined(MBEDTLS_SELF_TEST) && defined(MBEDTLS_PKCS1_V15)
#ifdef __OSK__
extern int rand(void);
#else
int rand(void)
{
#if defined(CONFIG_PCI_HCI)
	return get_random_int();
#elif defined(CONFIG_SDIO_HCI) || defined(CONFIG_USB_HCI)
	unsigned char random;
#ifdef __ECOS
	{
		unsigned char random_buf[4];
		get_random_bytes(random_buf, 4);
		random = random_buf[3];
	}
#else
	get_random_bytes(&random, 1);
#endif
	return (random % 5);
#endif
}
#endif
#endif