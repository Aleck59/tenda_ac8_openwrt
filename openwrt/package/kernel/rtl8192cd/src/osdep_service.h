/*
 *  Header file defines some common inline funtions
 *
 *  Copyright (c) 2017 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef __OSDEP_SERVICE_H_
#define __OSDEP_SERVICE_H_

#include "./typedef.h"

#ifdef __KERNEL__
#include "./osdep_service_linux.h"

#include <linux/timer.h>
#include <linux/module.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 18, 0)
#define from_timer timer_container_of
#define del_timer timer_delete
#define del_timer_sync timer_delete_sync
#define PCI_DMA_TODEVICE DMA_TO_DEVICE
#define PCI_DMA_FROMDEVICE DMA_FROM_DEVICE
#endif
#ifndef __LINUX_2_6_26__
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif

#ifdef CONFIG_USB_HCI
#include <linux/usb.h>
#ifndef __LINUX_2_6_21__
#include <linux/usb_ch9.h>
#else
#include <linux/usb/ch9.h>
#endif
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#endif // CONFIG_SDIO_HCI

#ifdef CONFIG_PCI_HCI
#include <linux/pci.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
/* The pci_* DMA wrappers were removed in Linux 5.18 */
#include <linux/dma-mapping.h>
#ifndef PCI_DMA_BIDIRECTIONAL
#define PCI_DMA_BIDIRECTIONAL	DMA_BIDIRECTIONAL
#endif
#ifndef PCI_DMA_NONE
#define PCI_DMA_NONE		DMA_NONE
#endif
#define pci_map_single(pdev, ptr, size, dir) \
	dma_map_single(&(pdev)->dev, (ptr), (size), (enum dma_data_direction)(dir))
#define pci_unmap_single(pdev, addr, size, dir) \
	dma_unmap_single(&(pdev)->dev, (addr), (size), (enum dma_data_direction)(dir))
#define pci_map_page(pdev, page, off, size, dir) \
	dma_map_page(&(pdev)->dev, (page), (off), (size), (enum dma_data_direction)(dir))
#define pci_unmap_page(pdev, addr, size, dir) \
	dma_unmap_page(&(pdev)->dev, (addr), (size), (enum dma_data_direction)(dir))
#define pci_dma_sync_single_for_cpu(pdev, addr, size, dir) \
	dma_sync_single_for_cpu(&(pdev)->dev, (addr), (size), (enum dma_data_direction)(dir))
#define pci_dma_sync_single_for_device(pdev, addr, size, dir) \
	dma_sync_single_for_device(&(pdev)->dev, (addr), (size), (enum dma_data_direction)(dir))
#define pci_dma_mapping_error(pdev, addr) \
	dma_mapping_error(&(pdev)->dev, (addr))
#define pci_alloc_consistent(pdev, size, handle) \
	dma_alloc_coherent(&(pdev)->dev, (size), (handle), \
			   in_atomic() ? GFP_ATOMIC : GFP_KERNEL)
#define pci_free_consistent(pdev, size, vaddr, handle) \
	dma_free_coherent(&(pdev)->dev, (size), (vaddr), (handle))
#define pci_set_dma_mask(pdev, mask) \
	dma_set_mask(&(pdev)->dev, (mask))
#define pci_set_consistent_dma_mask(pdev, mask) \
	dma_set_coherent_mask(&(pdev)->dev, (mask))
#endif
#endif

#if defined(CONFIG_RTL_8197F_WRT) && defined(CONFIG_DMA_NONCOHERENT)
/* the MIPS _dma_cache_* hooks are not exported, see rtl8197f_compat.c */
#include <asm/io.h>
void rtl8192cd_dma_cache_op(unsigned long start, unsigned long size, int dir);
#undef dma_cache_wback_inv
#undef dma_cache_wback
#undef dma_cache_inv
#define dma_cache_wback_inv(s, z) \
	rtl8192cd_dma_cache_op((unsigned long)(s), (z), DMA_BIDIRECTIONAL)
#define dma_cache_wback(s, z) \
	rtl8192cd_dma_cache_op((unsigned long)(s), (z), DMA_TO_DEVICE)
#define dma_cache_inv(s, z) \
	rtl8192cd_dma_cache_op((unsigned long)(s), (z), DMA_FROM_DEVICE)
#define _dma_cache_wback_inv(s, z)	dma_cache_wback_inv(s, z)
#define _dma_cache_wback(s, z)		dma_cache_wback(s, z)
#define _dma_cache_inv(s, z)		dma_cache_inv(s, z)
#endif
void rtl8192cd_dma_init(void);
void rtl8192cd_dma_exit(void);

#include <linux/spinlock.h>
#include <linux/circ_buf.h>
#endif // __KERNEL__

typedef struct semaphore _sema;
typedef spinlock_t	_lock;
#ifdef __LINUX_2_6_37__
typedef struct mutex 		_mutex;
#else
typedef struct semaphore	_mutex;
#endif

typedef	struct sk_buff	_pkt;
typedef struct list_head _list;
struct __queue {
	_list queue;
	int qlen;
	_lock lock;
};
typedef struct __queue _queue;
typedef unsigned long _irqL;
typedef struct net_device * _nic_hdl;

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)	((int)&(((type *)0)->field))
#endif

#ifndef nop
#define nop()	asm volatile ("nop")
#endif

#define _RND(sz, r) ((((sz)+(r)-1)/(r))*(r))

__inline static u32 _RND4(u32 sz)
{
	return ((sz + 3) & ~3);
}

__inline static u32 _RND8(u32 sz)
{
	return ((sz + 7) & ~7);
}

__inline static u32 _RND128(u32 sz)
{
	return ((sz + 127) & ~127);
}

__inline static u32 _RND256(u32 sz)
{
	return ((sz + 255) & ~255);
}

__inline static u32 _RND512(u32 sz)
{
	return ((sz + 511) & ~511);
}

#define LIST_CONTAINOR(ptr, type, member) \
        ((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))


__inline static void _rtw_mutex_init(_mutex *pmutex)
{
#ifdef __LINUX_2_6_37__
	mutex_init(pmutex);
#else
	init_MUTEX(pmutex);
#endif
}

__inline static void _rtw_mutex_free(_mutex *pmutex)
{
#ifdef __LINUX_2_6_37__
	mutex_destroy(pmutex);
#endif
}

__inline static void _enter_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
#ifdef __LINUX_2_6_37__
	mutex_lock(pmutex);
#else
	down(pmutex);
#endif
}

__inline static void _exit_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
#ifdef __LINUX_2_6_37__
	mutex_unlock(pmutex);
#else
	up(pmutex);
#endif
}

__inline static void _rtw_spinlock_init(_lock *plock)
{
	spin_lock_init(plock);
}

__inline static void _rtw_spinlock_free(_lock *plock) {}

__inline static void _rtw_spinlock(_lock	*plock)
{
	spin_lock(plock);
}

__inline static void _rtw_spinunlock(_lock *plock)
{
	spin_unlock(plock);
}
__inline static void _rtw_spinlock_ex(_lock	*plock)
{
	spin_lock(plock);
}

__inline static void _rtw_spinunlock_ex(_lock *plock)
{
	spin_unlock(plock);
}

__inline static void _enter_critical(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _enter_critical_ex(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical_ex(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _enter_critical_bh(_lock *plock, _irqL *pirqL)
{
	spin_lock_bh(plock);
}

__inline static void _exit_critical_bh(_lock *plock, _irqL *pirqL)
{
	spin_unlock_bh(plock);
}

__inline static void _rtw_init_listhead(_list *list)
{
        INIT_LIST_HEAD(list);
}

__inline static u32	rtw_is_list_empty(_list *phead)
{
	return (list_empty(phead) ? TRUE: FALSE);
}

__inline static void rtw_list_insert_head(_list *plist, _list *phead)
{
	list_add(plist, phead);
}

__inline static void rtw_list_insert_tail(_list *plist, _list *phead)
{
	list_add_tail(plist, phead);
}

__inline static void rtw_list_delete(_list *plist)
{
	list_del_init(plist);
}

__inline static void rtw_list_splice(_list *plist, _list *phead)
{
	list_splice_init(plist, phead);
}

__inline static _list *get_next(_list *list)
{
	return list->next;
}

__inline static _list *get_list_head(_queue *queue)
{
	return (&(queue->queue));
}

__inline static void _rtw_init_queue(_queue *pqueue)
{
	_rtw_init_listhead(&(pqueue->queue));
	_rtw_spinlock_init(&(pqueue->lock));
	pqueue->qlen = 0;
}

__inline static u32 _rtw_queue_empty(_queue	*pqueue)
{
	return (rtw_is_list_empty(&(pqueue->queue)));
}

__inline static u32 rtw_end_of_queue_search(_list *head, _list *plist)
{
	return ((head == plist)? TRUE : FALSE);
}

/*-----------------------------------------------------------------------------
                    Wrappers for struct call_single_data_t
------------------------------------------------------------------------------*/

#ifdef __KERNEL__
#ifdef __LINUX_4_14__
typedef call_single_data_t rtk_call_single_data_t;
#else
typedef struct call_single_data rtk_call_single_data_t;
#endif
#endif // __KERNEL__

/*-----------------------------------------------------------------------------
                    Wrappers for timer-related functions
------------------------------------------------------------------------------*/

#define USE_RTK_TIMER_LIST

#ifdef __LINUX_4_15__

struct rtk_timer_list {
	struct timer_list timer;
	unsigned long data;
};

/* Helper macros for timer callback function */
#define GET_TIMER_CALLBACK(_timerCb) (_timerCb ## _wrapper)

#define DECLARE_TIMER_CALLBACK(_timerCb) \
	void _timerCb ## _wrapper(struct timer_list *t)

/* Helper macro for timer callback function definition */
#define DEFINE_TIMER_CALLBACK(_timerCb) \
	void _timerCb ## _wrapper(struct timer_list *t) \
	{ \
		struct rtk_timer_list *rtkTimer = from_timer(rtkTimer, t, timer); \
		_timerCb(rtkTimer->data); \
	}

#define DEFINE_TIMER_CALLBACK_PTR(_timerCb) \
	void _timerCb ## _wrapper(struct timer_list *t) \
	{ \
		struct rtk_timer_list *rtkTimer = from_timer(rtkTimer, t, timer); \
		_timerCb((void *)rtkTimer->data); \
	}

#define rtk_timer_pending(_rtkTimer) \
	timer_pending(&((_rtkTimer)->timer))

#define rtk_add_timer_on(_rtkTimer, _cpu) \
	add_timer_on(&((_rtkTimer)->timer), (_cpu))

#define rtk_del_timer(_rtkTimer) \
	del_timer(&((_rtkTimer)->timer))

#define rtk_mod_timer(_rtkTimer, _expires) \
	mod_timer(&((_rtkTimer)->timer), (_expires))

#define rtk_timer_reduce(_rtkTimer, _expires) \
	timer_reduce(&((_rtkTimer)->timer), (_expires))

#define rtk_add_timer(_rtkTimer) \
	add_timer(&((_rtkTimer)->timer))

#define rtk_del_timer_sync(_rtkTimer) \
	del_timer_sync(&((_rtkTimer)->timer))

#define rtk_timer_setup(_rtkTimer, _timerCb, _data, _flags) \
	do { \
		timer_setup(&((_rtkTimer)->timer), _timerCb ## _wrapper, (_flags)); \
		(_rtkTimer)->data = (_data); \
	} while(0)

static __always_inline void rtk_timer_set_expires(struct rtk_timer_list *rtkTimer, unsigned long expires)
{
	rtkTimer->timer.expires = expires;
}

static __always_inline unsigned long rtk_timer_get_expires(struct rtk_timer_list *rtkTimer)
{
	return rtkTimer->timer.expires;
}

static __always_inline void (* rtk_timer_get_function(struct rtk_timer_list *rtkTimer))(struct timer_list *)
{
	return rtkTimer->timer.function;
}

#else // for Linux kernel < 4.15 or non-Linux platfotm

#define rtk_timer_list timer_list

/* Helper macros for timer callback function */
#define GET_TIMER_CALLBACK(_timerCb) (_timerCb ## _wrapper)

#define DECLARE_TIMER_CALLBACK(_timerCb) \
	void _timerCb ## _wrapper(unsigned long data)

/* Helper macro for timer callback function definition */
#define DEFINE_TIMER_CALLBACK(_timerCb) \
	void _timerCb ## _wrapper(unsigned long data) \
	{ \
		_timerCb(data); \
	}

#define rtk_timer_pending      timer_pending
#define rtk_add_timer_on       add_timer_on
#define rtk_del_timer          del_timer
#define rtk_mod_timer          mod_timer
#define rtk_add_timer          add_timer
#define rtk_del_timer_sync     del_timer_sync

#ifdef __LINUX_2_6_30__
#define rtk_mod_timer_pending  mod_timer_pending
#endif

#if defined(__LINUX_2_6_31__) && !defined(__LINUX_4_8__)
#define rtk_mod_timer_pinned   mod_timer_pinned
#endif

#ifdef __ECOS
#define rtk_timer_setup(_rtkTimer, _timerCb, _data, _flags) \
	do { \
		init_timer((_rtkTimer), (_data), _timerCb ## _wrapper); \
	} while(0)

#else  // !(__ECOS)
#ifdef __LINUX_3_7__
#define rtk_timer_setup(_rtkTimer, _timerCb, _data, _flags) \
	do { \
		__init_timer((_rtkTimer), (_flags)); \
		(_rtkTimer)->data = (_data); \
		(_rtkTimer)->function = _timerCb ## _wrapper; \
	} while(0)
#else // !(__LINUX_3_7__)
#define rtk_timer_setup(_rtkTimer, _timerCb, _data, _flags) \
	do { \
		init_timer((_rtkTimer)); \
		(_rtkTimer)->data = (_data); \
		(_rtkTimer)->function = _timerCb ## _wrapper; \
	} while(0)
#endif
#endif // __ECOS

static __always_inline void rtk_timer_set_expires(struct rtk_timer_list *rtkTimer, unsigned long expires)
{
	rtkTimer->expires = expires;
}

static __always_inline unsigned long rtk_timer_get_expires(struct rtk_timer_list *rtkTimer)
{
	return rtkTimer->expires;
}

static __always_inline void (* rtk_timer_get_function(struct rtk_timer_list *rtkTimer))(struct timer_list *)
{
	return rtkTimer->function;
}

#endif // __LINUX_4_15__

typedef struct rtk_timer_list _timer;

__inline static void _init_timer(_timer *ptimer, _nic_hdl nic_hdl, void *pfunc, void *cntx)
{
#ifdef __LINUX_4_15__
	timer_setup(&(ptimer->timer), pfunc, 0);
	ptimer->data = (unsigned long)cntx;
#else
	init_timer(ptimer);
	ptimer->function = pfunc;
	ptimer->data = (unsigned long)cntx;
#endif
}

__inline static void _set_timer(_timer *ptimer, u32 delay_time)
{
	rtk_mod_timer(ptimer, jiffies + msecs_to_jiffies(delay_time));
}

__inline static void _cancel_timer(_timer *ptimer, u8 *bcancelled)
{
	rtk_del_timer_sync(ptimer);
	*bcancelled = TRUE;//TRUE ==1; FALSE==0
}

__inline static void _cancel_timer_ex(_timer *ptimer)
{
	rtk_del_timer_sync(ptimer);
}

#ifndef __LINUX_2_6_22__
static inline unsigned char *skb_end_pointer(const struct sk_buff *skb)
{
	return skb->end;
}
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}
static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
	skb->tail = skb->data;
}

static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb->tail = skb->data + offset;
}
#endif

#ifndef __LINUX_2_6_39__
#define skb_queue_reverse_walk_safe(queue, skb, tmp)		\
		for (skb = (queue)->prev, tmp = skb->prev;			\
		     skb != (struct sk_buff *)(queue);				\
		     skb = tmp, tmp = skb->prev)

#endif

#endif // __OSDEP_SERVICE_H_
