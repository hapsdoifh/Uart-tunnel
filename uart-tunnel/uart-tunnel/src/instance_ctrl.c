/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include "uart-tunnel.h"
#include "c-serial-com.h"

#define SEM_NAME_UART_TUNNEL        "SEM_NAME_UART_TUNNEL"

static sem_t*  static_semInstance = SEM_FAILED;

bool svrInitInstance(void) {
    static_semInstance = sem_open(SEM_NAME_UART_TUNNEL,O_CREAT|O_RDWR,00777,1);
    if (SEM_FAILED == static_semInstance) {   //errno == EEXIST
        printf("\nFailed on initializing instance.\n\n");
        return false;
    } else {
        if (0 == sem_trywait(static_semInstance)) {
            printf("\n<OK, this is the first UART-TUNNEL instance.>\n");
            return true;
        } else {
            sem_close(static_semInstance);
            static_semInstance = SEM_FAILED;
            printf("\nAnother UART-TUNNEL is running.\n\n");
            return false;
        }
    }
}

void svrExitInstance(void) {
    if (SEM_FAILED != static_semInstance) {
        sem_unlink(SEM_NAME_UART_TUNNEL);
        sem_close(static_semInstance);
        static_semInstance = SEM_FAILED;
        printf("\nUART-TUNNEL shutting down\n");
    }
}

bool svrTestInstance(void) {
    if (static_semInstance && 0 == sem_trywait(static_semInstance)) {
        sem_post(static_semInstance);
        return false;   //unlocked, means we can exit
    } else {
        return true;    //locked, we have to continue our work
    }
}

void svrStopInstance(void) {
    if (static_semInstance)
        sem_post(static_semInstance);
    else exit(1);
}

void ShutdownInstance(void) {
    sem_t*  sem = sem_open(SEM_NAME_UART_TUNNEL,O_RDWR,00777,1);
    if (SEM_FAILED != sem) {
        sem_post(sem);
//        sem_unlink(SEM_NAME_UART_TUNNEL);
        sem_close(sem);
    }
}

static void sig(int signo) {
    if ((signo == SIGTERM) || (signo == SIGINT) || (signo == SIGSEGV)) {
        switch (signo) {
            case SIGSEGV:
                LOG("Segmentation fault, aborting.\n");
                svrStopInstance();
                break;
            case SIGBUS:
                LOG("Bus error, aborting.\n");
                svrStopInstance();
                break;
            case SIGTERM:
            case SIGINT:
                LOG("User abort\n");
                svrStopInstance();
            default:;
        }
    }
}
