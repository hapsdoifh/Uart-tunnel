
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>


#include "c-serial-com.h"

static char szComName[16][16] = {
    "/dev/ttyS0", "/dev/tty1", "/dev/tty2", "/dev/tty3",
    "/dev/ttyS4", "/dev/tty5", "/dev/tty6", "/dev/ttySAC7",
};

void		ScBitsAssign(unsigned long* target,unsigned long source,unsigned long mask);
int		ScOpenDev(SerialCom* sc,char *Dev);
int		ScSetParity(SerialCom* sc,int databits,int stopbits,int parity);
int		ScSetSpeed(SerialCom* sc,int speed);
static	void*	ScThreadReader(void* pPara);

void ScInit(SerialCom* sc,int nTimeout) {
    sc->m_fd = -1;
    sc->m_nBuff = 0;
    sc->m_reader = 0;
    sc->m_nTimeout = nTimeout;
    memset(sc->m_buffer, 0x00, MAX_COM_BUFFER_SIZE);
    pthread_mutex_init(&sc->m_lockBuffer, NULL);
    sc->m_reader = 0;
}

void ScExit(SerialCom* sc) {
    if (sc->m_reader) {
        pthread_cancel(sc->m_reader);
        pthread_join(sc->m_reader, NULL);
        sc->m_reader = 0;
    }
    pthread_mutex_destroy(&sc->m_lockBuffer);
}
/***@brief  设置串口通信速率
 *@param  fd     类型 int  打开串口的文件句柄
 *@param  speed  类型 int  串口速度
 *@return  void*/
int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300,
    B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300,};
int name_arr[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300,
    115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300,};

//#ifdef  RELEASE_ON_ARM
//char CSerialCom::szCOM[16][16] = {
//    "/dev/ttySAC0", "/dev/ttySAC1", "/dev/ttySAC2", "/dev/ttySAC3",
//    "/dev/ttySAC4", "/dev/ttySAC5", "/dev/ttySAC6", "/dev/ttySAC7",};
//#else   //RELEASE_ON_ARM
//string CSerialCom::szCOM[16] = { "/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3",
//                                  "/dev/ttyS4","/dev/ttyS5","/dev/ttyS6","/dev/ttyS7", };
//#endif  //RELEASE_ON_ARM

int ScSetSpeed(SerialCom* sc,int speed) {
    if (sc->m_fd < 0) return false;
    int i;
    int status;
    struct termios Opt;
    tcgetattr(sc->m_fd, &Opt);
    for (i = 0; i < sizeof (speed_arr) / sizeof (int); i++) {
        if (speed == name_arr[i]) {
            tcflush(sc->m_fd, TCIOFLUSH);
            //cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(sc->m_fd, TCSANOW, &Opt);
            if (status != 0)
                perror("tcsetattr fd1");
            return true;
        }
        tcflush(sc->m_fd, TCIOFLUSH);
    }
    perror("\nInvalidate baud rate.\n");
    return false;
}

/**
 *@brief   设置串口数据位，停止位和效验位
 *@param  fd     类型  int  打开的串口文件句柄*
 *@param  databits 类型  int 数据位   取值 为 7 或者8*
 *@param  stopbits 类型  int 停止位   取值为 1 或者2*
 *@param  parity  类型  int  效验类型 取值为N,E,O,,S
 */
void ScBitsAssign(unsigned long* target, unsigned long source, unsigned long mask) {
    *target &= ~mask;
    *target |= source;
}

int ScSetParity(SerialCom* sc,int databits, int stopbits, int parity) {
    if (sc->m_fd < 0) return false;
    struct termios options;
    if (tcgetattr(sc->m_fd, &options) != 0) {
        perror("SetupSerial 1");
        return (false);
    }
    switch (databits) { /*设置数据位数*/
        case 7: ScBitsAssign((unsigned long*) &options.c_cflag, CS7, CSIZE);
            break;
        case 8: ScBitsAssign((unsigned long*) &options.c_cflag, CS8, CSIZE);
            break;
        default: fprintf(stderr, "Unsupported data size\n");
            return false;
    }
    switch (parity) {
        case 'n': case 'N':
            ScBitsAssign((unsigned long*) &options.c_cflag, 0, PARENB); /* Clear parity enable */
            ScBitsAssign((unsigned long*) &options.c_iflag, 0, INPCK); /* Enable parity checking */
            break;
        case 'o': case 'O':
            ScBitsAssign((unsigned long*) &options.c_cflag, (PARODD | PARENB),0); /* 设置为奇效验*/
            ScBitsAssign((unsigned long*) &options.c_iflag, INPCK,0); /* Disnable parity checking */
            break;
        case 'e': case 'E':
            ScBitsAssign((unsigned long*) &options.c_cflag, PARENB,0); /* Enable parity */
            ScBitsAssign((unsigned long*) &options.c_cflag, 0, PARODD); /* 转换为偶效验*/
            ScBitsAssign((unsigned long*) &options.c_iflag, INPCK,0); /* Disnable parity checking */
            break;
        case 'S': case 's': /*as no parity*/
            ScBitsAssign((unsigned long*) &options.c_cflag, 0, PARENB);
            ScBitsAssign((unsigned long*) &options.c_cflag, 0, CSTOPB);
            break;
        default:
            fprintf(stderr, "Unsupported parity\n");
            return (false);
    }
    /* 设置停止位*/
    switch (stopbits) {
        case 1: ScBitsAssign((unsigned long*) &options.c_cflag, 0, CSTOPB);
            break;
        case 2: ScBitsAssign((unsigned long*) &options.c_cflag, CSTOPB,0);
            break;
        default: fprintf(stderr, "Unsupported stop bits\n");
            return false;
    }
    /* Set input parity option */
    if (parity != 'n') ScBitsAssign((unsigned long*) &options.c_iflag, INPCK,0);
    ScBitsAssign((unsigned long*) &options.c_lflag, 0, ICANON | ECHO | ECHOE | ISIG);
    ScBitsAssign((unsigned long*) &options.c_oflag, 0, INLCR | IGNCR | ICRNL);
    ScBitsAssign((unsigned long*) &options.c_oflag, 0, ONLCR | OCRNL);
    /*设置流控*/
    //RTS/CTS (硬件) 流控制
    ScBitsAssign((unsigned long*) &options.c_cflag, 0, CRTSCTS); //无流控
    //opt.c_cflag  |=  CRTSCTS  //硬流控
    //opt.c_cflag  | = IXON|IXOFF|IXANY  //软流控
    ScBitsAssign((unsigned long*) &options.c_iflag, 0, IXOFF); //不启用输入的 XON/XOFF 流控制
    ScBitsAssign((unsigned long*) &options.c_iflag, 0, IXON); //不启用输出的 XON/XOFF 流控制
    ScBitsAssign((unsigned long*) &options.c_iflag, 0, INLCR | IGNCR | ICRNL);
    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0;

    tcflush(sc->m_fd, TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(sc->m_fd, TCSANOW, &options) != 0) {
        perror("SetupSerial 3");
        return (false);
    }
    return (true);
}

/**
 *@breif 打开串口
 */
int ScOpenDev(SerialCom* sc,char *Dev) {
    sc->m_fd = open(Dev, O_RDWR | O_SYNC); // | O_NONBLOCK);         //| O_NOCTTY | O_NDELAY
    if (sc->m_fd > 0) return true;
    else {
        perror("\nCan't Open Serial Port :");
        perror(Dev);
        perror("\n");
        return false;
    }
}

/**
 *@breif 	main()
 */
void ScSetDevName(SerialCom* sc,int i,char* name) {
    if (0 <= i && i < 8 && name != NULL) {
        strncpy(szComName[i],name,15);
    }
}

//  port num is 1 based index
bool ScOpen(SerialCom* sc,int port, int baud, int data, int stop, char parity) {
    if (port < 1) port = 1;
    if (port > 8) port = 8;
    ScClose(sc);

    char* sDevName = szComName[port - 1];
    if (0 == sDevName[0]) return false;
    if (!ScOpenDev(sc,sDevName)) return false;
    if (ScSetSpeed(sc,baud) && ScSetParity(sc,data, stop, parity)) {
//        if (0 == m_nTimeout) //unblock mode
            pthread_create(&sc->m_reader, NULL, ScThreadReader, sc);
        return true;
    } else {
        ScClose(sc);
        return false;
    }
}

void ScClose(SerialCom* sc) {
    if (sc->m_reader) {
        pthread_cancel(sc->m_reader);
        pthread_join(sc->m_reader, NULL);
        sc->m_reader = 0;
    }
    pthread_mutex_unlock(&sc->m_lockBuffer);
    if (sc->m_fd > 0) close(sc->m_fd);
    sc->m_fd = -1;
}

void ScClear(SerialCom* sc) {
    if (sc->m_fd > 0) {
        pthread_mutex_lock(&sc->m_lockBuffer);
        sc->m_nBuff = 0;
        memset(sc->m_buffer, 0x00, MAX_COM_BUFFER_SIZE);
        pthread_mutex_unlock(&sc->m_lockBuffer);
    }
}

int ScSend(SerialCom* sc, const char* pData, int nData) {
    if (sc->m_fd > 0) {
//        if (ENABLE_TRACE5)
//            for (int i=0;i<nData;i++) printf("%02x ",0x00ff&pData[i]);
//        fflush(stdout);
        return write(sc->m_fd, pData, nData);
    }
    return -1;
}

static int ScRecv(SerialCom* sc) {
    if (sc->m_fd > 0) { //read COM port in block mode
        char buffer[MAX_COM_BUFFER_SIZE / 4];
        memset(buffer, 0x00, MAX_COM_BUFFER_SIZE / 4);
        int nData = read(sc->m_fd, buffer, MAX_COM_BUFFER_SIZE / 4);
        if (nData < 0) {
            perror("\nCOM read Error:\n");
            return -1;
        } else if (nData > 0) {
            pthread_mutex_lock(&sc->m_lockBuffer);
//            TRACE4("[");  fflush(stdout);
//            TRACE4("[");
//            for(int i = 0; i < nData; i ++ ) TRACE4("%02X~",0x00ff&buffer[i]);
//            TRACE4("]");
            if (sc->m_nBuff + nData > MAX_COM_BUFFER_SIZE) {
                // |<....MAX_SIZE.....>|
                //  <..m_nBuff..>|<......nData....>
                // |*************|.....|<..nOver..>
                int nOver = sc->m_nBuff + nData - MAX_COM_BUFFER_SIZE;
                // give up(overwrite) the first nOver bytes
                // |*****|<...nData...>|
                memmove(sc->m_buffer, sc->m_buffer + nOver, sc->m_nBuff - nOver);
                sc->m_nBuff -= nOver;
            }
            memcpy(sc->m_buffer + sc->m_nBuff, buffer, nData); //append the new data
            sc->m_nBuff += nData;
//            TRACE4("]");  fflush(stdout);
            pthread_mutex_unlock(&sc->m_lockBuffer);
        }// else TRACE4(".");
        return sc->m_nBuff;
    }
    return -1;
}

static int ScBuffPopup(SerialCom* sc, char* pData, int nMax) {
    int nRead;
    if (sc->m_nBuff < nMax) {
        nRead = sc->m_nBuff;
        memcpy(pData, sc->m_buffer, nRead);
        sc->m_nBuff = 0;
    } else { //m_nBuff >= nMax
        nRead = nMax;
        memcpy(pData, sc->m_buffer, nRead);
        memmove(sc->m_buffer, sc->m_buffer + nRead, sc->m_nBuff - nRead);
        memset(sc->m_buffer+nRead,0x00,sc->m_nBuff-nRead);
        sc->m_nBuff -= nRead;
    }
    return nRead;
}

int ScRead(SerialCom* sc, const char* pData, int nOffset, const int nMax) {
    char* pOutput = (char*) pData + nOffset;
    int nMaxout = nMax - nOffset;
    
    if (pData == NULL) return -1;
    if (nOffset >= nMax) return 0;

    memset(pOutput, 0x00, nMaxout);
    if (sc->m_nTimeout > 0) { //block mode
        int i = 0;
        int nFrequency = sc->m_nTimeout * 1000 / COM_RECEIVE_INTERVAL;
        for (i = 0; i < nFrequency; i++, usleep(COM_RECEIVE_INTERVAL)) {
            pthread_mutex_lock(&sc->m_lockBuffer);
            if (sc->m_nBuff > 0) {
                int nRead = ScBuffPopup(sc, pOutput, nMaxout);
                pthread_mutex_unlock(&sc->m_lockBuffer);
                return nRead;
            }
            pthread_mutex_unlock(&sc->m_lockBuffer);
        }
        return COM_RECEIVE_TIME_OUT;
    } else { //unblock mode
        if (0 == pthread_mutex_trylock(&sc->m_lockBuffer)) {
            if (sc->m_nBuff > 0) {
                int nRead = ScBuffPopup(sc, pOutput, nMaxout);
                pthread_mutex_unlock(&sc->m_lockBuffer);
                return nRead;
            }
            pthread_mutex_unlock(&sc->m_lockBuffer);
        }
        return 0;
    }
}

void* ScThreadReader(void* pPara) {
    if (pPara) {
        SerialCom* sc = (SerialCom*) pPara;
        const bool blocked = (sc->m_nTimeout > 0) ? true : false;
        //sc->m_nTimeout > 0 means in block mode, do not need usleep()
        for (; true; ) {
            ScRecv(sc);
            if (!blocked) usleep(COM_RECEIVE_INTERVAL);
        }
    }
}
