#ifndef	__C_SERIAL_COM_H__
#define	__C_SERIAL_COM_H__

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef bool
#define bool    int
#endif
#ifndef true
#define true    1
#endif
#ifndef false
#define false   0
#endif
#define	MAX_COM_BUFFER_SIZE		4096
#define COM_RECEIVE_TIME_OUT            -1
#define COM_RECEIVE_INTERVAL            50000   //10000us = 10ms

#define BOARD_TO_OUTPUT 0

#define ENABLE_TRACE0  true
#define ENABLE_TRACE1  true
#define ENABLE_TRACE2  true
#define ENABLE_TRACE3  true
#define ENABLE_TRACE4  true
#define ENABLE_TRACE5  false
#define ENABLE_TRACE6  false

#define TRACE0          if(ENABLE_TRACE0) printf
#define TRACE1          if(ENABLE_TRACE1) printf
#define TRACE2          if(ENABLE_TRACE2) printf
#define TRACE3          if(ENABLE_TRACE3) printf
#define TRACE4          if(ENABLE_TRACE4) printf
#define TRACE5          if(ENABLE_TRACE5) printf
#define TRACE6          if(ENABLE_TRACE6) printf
#define TC4(x)          if(ENABLE_TRACE4 && BOARD_TO_OUTPUT == x) printf

#include "pthread.h"


typedef struct __serial_comm {
    pthread_mutex_t m_lockBuffer;
    pthread_t       m_reader;
    int             m_fd;
    int             m_nBuff;
    int             m_nTimeout;
    char            m_buffer[MAX_COM_BUFFER_SIZE];
} SerialCom;

void ScInit(SerialCom* sc,int nTimeout);/*ms*/
void ScExit(SerialCom* sc);
bool ScOpen(SerialCom* sc,int port,int baud,int data,int stop,char parity);
void ScSetDevName(SerialCom* sc,int i,char* name);
void ScClose(SerialCom* sc);
void ScClear(SerialCom* sc);
int ScSend(SerialCom* sc, const char* pData, int nData);
int ScRead(SerialCom* sc, const char* pData, int nOffset, const int nMax);

#ifdef	__cplusplus
}
#endif

#endif		//__C_SERIAL_COM_H__
