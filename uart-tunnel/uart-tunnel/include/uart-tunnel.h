/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   uart-tunnel.h
 * Author: yaoyang
 *
 * Created on March 16, 2020, 12:25 p.m.
 */

#ifndef UART_TUNNEL_H
#define UART_TUNNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <syslog.h>

//#define __DEBUG  1
//#if __DEBUG
extern int  debug_level;

#define LOG(fmt, ...) 	do {\
    int lvl = debug_level;\
    if (lvl & 0x01) printf(fmt, ##__VA_ARGS__); fflush(stdout);\
    if (lvl & 0x02) syslog(LOG_INFO, "UART-TUNNEL : " fmt, ##__VA_ARGS__);\
} while (0)
//#else
//#define LOG(fmt, ...) 	do {\
//    printf(fmt, ##__VA_ARGS__); fflush(stdout);\
//} while (0)
//#endif

#ifdef __cplusplus
}
#endif

#endif /* UART_TUNNEL_H */

