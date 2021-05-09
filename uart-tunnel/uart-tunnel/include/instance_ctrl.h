/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   instance_ctrl.h
 * Author: yaoyang
 *
 * Created on March 16, 2020, 1:06 p.m.
 */

#ifndef INSTANCE_CTRL_H
#define INSTANCE_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

void init_uart_tunnel_signals();
bool svrInitInstance(void);
void svrExitInstance(void);
bool svrTestInstance(void);
void svrStopInstance(void);
void ShutdownInstance(void);


#ifdef __cplusplus
}
#endif

#endif /* INSTANCE_CTRL_H */

