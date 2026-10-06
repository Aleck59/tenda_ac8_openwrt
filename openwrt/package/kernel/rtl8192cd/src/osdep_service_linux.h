#ifndef _LINUX_VER_H_
#define _LINUX_VER_H_

#ifdef __KERNEL__

#include <linux/version.h>

/*-----------------------------------------------------------------------------
                        Linux kernel version macros
------------------------------------------------------------------------------*/

#if (LINUX_VERSION_CODE >= 0x020415) // linux 2.4.21
	#define __LINUX_2_4_21__
#endif

#if (LINUX_VERSION_CODE > 0x020600)  // linux 2.6
	#define __LINUX_2_6__
#endif

#if (LINUX_VERSION_CODE >= 0x02060C) // linux 2.6.12
	#define __LINUX_2_6_12__
#endif

#if (LINUX_VERSION_CODE >= 0x02060D) // linux 2.6.13
	#define __LINUX_2_6_13__
#endif

#if (LINUX_VERSION_CODE >= 0x020612) // linux 2.6.18
	#define __LINUX_2_6_18__
#endif

#if (LINUX_VERSION_CODE >= 0x020613) // linux 2.6.19
	#define __LINUX_2_6_19__
#endif

#if (LINUX_VERSION_CODE >= 0x020614) // linux 2.6.20
	#define __LINUX_2_6_20__
#endif

#if (LINUX_VERSION_CODE >= 0x020615) // linux 2.6.21
	#define __LINUX_2_6_21__
#endif

#if (LINUX_VERSION_CODE >= 0x020616) // linux 2.6.22
	#define __LINUX_2_6_22__
#endif

#if (LINUX_VERSION_CODE >= 0x020617) // linux 2.6.23
	#define __LINUX_2_6_23__
#endif

#if (LINUX_VERSION_CODE >= 0x020618) // linux 2.6.24
	#define __LINUX_2_6_24__
#endif

#if (LINUX_VERSION_CODE >= 0x020619) // linux 2.6.25
	#define __LINUX_2_6_25__
#endif

#if (LINUX_VERSION_CODE >= 0x02061A) // linux 2.6.26
	#define __LINUX_2_6_26__
#endif

#if (LINUX_VERSION_CODE >= 0x02061B) // linux 2.6.27
	#define __LINUX_2_6_27__
#endif

#if (LINUX_VERSION_CODE >= 0x02061D) // linux 2.6.29
	#define __LINUX_2_6_29__
#endif

#if (LINUX_VERSION_CODE >= 0x02061E) // linux 2.6.30
	#define __LINUX_2_6_30__
#endif

#if (LINUX_VERSION_CODE >= 0x02061F) // linux 2.6.31
	#define __LINUX_2_6_31__
#endif

#if (LINUX_VERSION_CODE >= 0x020620) // linux 2.6.32
	#define __LINUX_2_6_32__
#endif

#if (LINUX_VERSION_CODE >= 0x020621) // linux 2.6.33
	#define __LINUX_2_6_33__
#endif

#if (LINUX_VERSION_CODE >= 0x020623) // linux 2.6.35
	#define __LINUX_2_6_35__
#endif

#if (LINUX_VERSION_CODE >= 0x020624) // linux 2.6.36
	#define __LINUX_2_6_36__
#endif

#if (LINUX_VERSION_CODE >= 0x020625) // linux 2.6.37
	#define __LINUX_2_6_37__
#endif

#if (LINUX_VERSION_CODE >= 0x020627) // linux 2.6.39
	#define __LINUX_2_6_39__
#endif

#if (LINUX_VERSION_CODE >= 0x020628) // linux 2.6.40
	#define __LINUX_2_6_40__
#endif

#if (LINUX_VERSION_CODE >= 0x02062A) // linux 2.6.42
	#define __LINUX_2_6_42__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0))
	#define __LINUX_3_0__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
	#define __LINUX_3_2__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
	#define __LINUX_3_3__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
	#define __LINUX_3_4__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
	#define __LINUX_3_6__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
	#define __LINUX_3_7__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0))
	#define __LINUX_3_8__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
	#define __LINUX_3_9__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	#define __LINUX_3_10__
#endif

#if (LINUX_VERSION_CODE == KERNEL_VERSION(3,10,49))
	#define __LINUX_3_10_49_ONLY__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0))
	#define __LINUX_3_13__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
	#define __LINUX_3_14__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0))
	#define __LINUX_3_15__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
	#define __LINUX_3_16__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0))
	#define __LINUX_3_18__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,21))
	#define __LINUX_3_18_21__
#endif

#if (LINUX_VERSION_CODE == KERNEL_VERSION(3,18,21))
	#define __LINUX_3_18_21_ONLY__
#endif

#if (LINUX_VERSION_CODE == KERNEL_VERSION(3,18,23))
	#define __LINUX_3_18_23_ONLY__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,20,0))
	#define __LINUX_3_20__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
	#define __LINUX_4_0__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	#define __LINUX_4_1__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,2,0))
	#define __LINUX_4_2__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	#define __LINUX_4_4__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,140))
	#define __LINUX_4_4_140__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,211))
	#define __LINUX_4_4_211__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0))
	#define __LINUX_4_5__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0))
	#define __LINUX_4_6__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,8,0))
	#define __LINUX_4_8__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0))
	#define __LINUX_4_11__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0))
	#define __LINUX_4_12__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
	#define __LINUX_4_14__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0))
	#define __LINUX_4_15__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,16,0))
	#define __LINUX_4_16__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
	#define __LINUX_4_20__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0))
	#define __LINUX_5_0__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,5,0))
	#define __LINUX_5_5__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
	#define __LINUX_5_6__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0))
	#define __LINUX_5_8__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	#define __LINUX_5_10__
#endif

#endif //  __KERNEL__
#endif // _LINUX_VER_H_
