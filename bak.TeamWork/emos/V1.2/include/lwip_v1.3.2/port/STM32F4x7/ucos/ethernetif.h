#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);

/* 备注：4-5之间的优先级作为PIP或者保留，7-9之间的优先级由LwIP协议栈占用 */




#endif 
