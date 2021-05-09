/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Harry Wang
 *
 * Created on March 26, 2020, 11:22 a.m.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "c-serial-com.h"
#include "uart-tunnel.h"
#include "instance_ctrl.h"
#include "wsocket.h"

#define MULTICAST_IPADDR        "224.0.0.234"

static char static_version[20] = "1.0-1";
static char static_DevName[16] = "/dev/ttyS0";
static char MULTICAST_IP[16] = MULTICAST_IPADDR;
static char REMOTE_IPADD[16] = MULTICAST_IPADDR;
static WORD REMOTE_PORT = 9989;
static int datarate=115200;

static SerialCom            myuart;
static SOCKET_UDP_SERVER    myserver;

int debug_level = 0;

static int GPS_OnSend(SOCKET_BASE* sock_base,BYTE* buf_send) {
    
    int nsend=0;
    nsend=ScRead(&myuart,buf_send,0,1023); //read from serial port
    /* no need to do anything with buf_send it's a pointer 
    and it's value still can be accessed even after the function finishes*/
    if(nsend>0) {
        LOG("%s", buf_send);
//        printf("Onsend:\n");
//        printf(buf_send);
//        printf("\n");
//        fflush(stdout);
    }
    return nsend;
}


static void GPS_OnRecieve(SOCKET_BASE* socket_base, BYTE* buf_rcve, int nrcv)
{    
    if(nrcv > 0){ //if the buffer isn't empty
//        printf("OnRecieve:\n");
//        printf(buf_rcve);
//        printf("\n");
//        fflush(stdout);
        buf_rcve[nrcv]=0;
        LOG("%s", buf_rcve);
        ScSend(&myuart,buf_rcve,nrcv);//Send data to serial port 

    }
}

static void signal_handler(int signo) { //handle system messages
    if ((signo == SIGTERM) || (signo == SIGINT) || (signo == SIGSEGV)) {
        switch (signo) {
            case SIGSEGV:
                //Program run error
                printf("Segmentation fault, aborting.\n");
                UdpServer_StopListenThread(&myserver);
                ScClose(&myuart);
                svrStopInstance();
                exit(-1);
                break;
            case SIGBUS:
                //Bus error
                printf("Bus error, aborting.\n");
                UdpServer_StopListenThread(&myserver);
                ScClose(&myuart);
                svrStopInstance();
                exit(-1);
                break;
            case SIGTERM:
            case SIGINT:
                //ctrl+c
                printf("Request from user Ctrl+C.\n");
                UdpServer_StopListenThread(&myserver);
                ScClose(&myuart);
                svrStopInstance();
                exit(-1);
            default:;
        }
    }
}

static void init_signals() {
    signal (SIGTERM, signal_handler);
    signal (SIGINT, signal_handler);
    signal (SIGSEGV, signal_handler);
    signal (SIGBUS, signal_handler);
}	

/*
 * 
 */
static int uart_tunnel_service(void) {
    if (svrInitInstance()) {
        bool success = false; //if opening serial port is successful
        
        ScInit(&myuart,1000);
        ScSetDevName(&myuart,0,static_DevName);
        success = ScOpen(&myuart,1,datarate,8,1,'n');//Try opening the serial port
        if (success > 0) //if the serial port is opened
        {
            //initiate the server variable with its call functions
            UdpServer_Init(&myserver , REMOTE_PORT, GPS_OnSend , GPS_OnRecieve );

            //If the server is able to listen to this multicast group with this port 
            if (UdpServer_StartListenThread(&myserver, MULTICAST_IP, REMOTE_PORT)) 
            {
                init_signals();//initiate message handlers 
                UdpServer_SetDestAddr(&(myserver.m_connection.sock_base), REMOTE_IPADD, REMOTE_PORT); 

                while(svrTestInstance())
                {//repeat forever until one of the system messages 
                    usleep(1000000);
                }

            } else {
                printf("Failed on starting UDP server...\n"); //unable to listen to this address with this port
                ScClose(&myuart); //close serial port
                return -2;
            }
            ScClose(&myuart);
            return 0;
        }
        printf("Error on opening serial port\n");
        ScClose(&myuart);
        svrExitInstance();
        return -1;
    }
    return EXIT_SUCCESS;
}

#define CMD_LINE_HELP       0
#define CMD_LINE_STOP       1
#define CMD_LINE_START      2
#define CMD_LINE_VERSION    3

static bool CheckIpValid(char* multicastIP) {
    int cnt = 0;
    int Sections[4];
    int DtCount = 0;
    memset(Sections, 0, sizeof (Sections));
    while (multicastIP[cnt] != 0) { //to check if any character other than numbers and periods are in this array 
        if ('0' <= multicastIP[cnt] && multicastIP[cnt] <= '9' || multicastIP[cnt] == '.') { //48 is '0' 57 is '9'
            if (multicastIP[cnt] == '.') {
                DtCount++;
            }
            cnt++;
        } else {
            return false;
        }
    }
    sscanf(multicastIP, "%d.%d.%d.%d", &Sections[0], &Sections[1], &Sections[2], &Sections[3]);
    if (Sections[0] >= 240 || Sections[1] >= 255 || Sections[2] >= 255 ||
        Sections[3] < 1 || Sections[3] >= 255 || DtCount > 3) {
        printf("Sections");
        printf("IP:\n%d.%d.%d.%d\n", Sections[0], Sections[1], Sections[2], Sections[3]);
        return false;
    }
    printf("IP:\n%d.%d.%d.%d\n", Sections[0], Sections[1], Sections[2], Sections[3]);
    return true;
}

static bool CheckNumbValid(char* port) {
    int cnt = 0;
    while (port[cnt] != 0) {
        if ('0' <= port[cnt] && port[cnt] <= '9' || port[cnt] == '.') { //48 is '0' 57 is '9'
            cnt++;
        } else {
            return false;
        }
    }
    return true;
}

static int read_cmdline_options(int argc, char** argv) {
#define IS_OPTION_NAME(name, argv, i)       (strcmp(argv[i], name) == 0)
#define HAS_OPTION_VALUE(argc, argv, i)     (i + 1 < argc && argv[i+1] && strchr(argv[i+1], '-') == NULL)
    int cnt = 1;
    //handling the arguments passed in to main
    while(cnt < argc) {   //argc starts of with 1
        if (IS_OPTION_NAME("-stop", argv, cnt)) {
            return CMD_LINE_STOP;
        } else if (IS_OPTION_NAME("-start", argv, cnt)) {
            return CMD_LINE_START;
        } else if (IS_OPTION_NAME("-help", argv, cnt)) {
            return CMD_LINE_HELP;
        } else if (IS_OPTION_NAME("-ip", argv, cnt)) { //The multicast ip
            if (HAS_OPTION_VALUE(argc, argv, cnt)) {
                if(CheckIpValid(argv[cnt+1])){ 
                    strncpy(REMOTE_IPADD,argv[cnt+1],15);   cnt ++;
                } else {
                    printf("\nError: Invalid IP\n");
                    return CMD_LINE_HELP;
                }
            }
        } else if (IS_OPTION_NAME("-port", argv, cnt)) { //the port
            if (HAS_OPTION_VALUE(argc, argv, cnt)) {
                if(CheckNumbValid(argv[cnt+1])){
                    REMOTE_PORT = (WORD) atoi(argv[cnt+1]); cnt ++;
                } else {
                    printf("\nError: Invalid port\n");
                    return CMD_LINE_HELP;
                }
            }
        } else if (IS_OPTION_NAME("-dev", argv, cnt)) { //the serial device name, ex: if connected to a gps then the name will be different
            if (HAS_OPTION_VALUE(argc, argv, cnt)) {
                cnt ++;
                sprintf(static_DevName, "/dev/%.10s", argv[cnt]);
            }
        } else if (IS_OPTION_NAME("-debug", argv, cnt)) {
            if (HAS_OPTION_VALUE(argc, argv, cnt)) {
                sscanf(argv[cnt+1], "%x", &debug_level);    cnt ++;
            }
        }else if(IS_OPTION_NAME("-baud",argv,cnt)){
            if(HAS_OPTION_VALUE(argc,argv,cnt)){
                if(CheckNumbValid(argv[cnt+1])){
                    datarate=atoi(argv[cnt+1]);
                    cnt++;
                }else{
                    printf('\nError: Invalid baud rate\n');
                    return CMD_LINE_HELP;
                }
            }
        }
        cnt ++;
    }
    return CMD_LINE_HELP;
}

static void print_help(char** argv) {
    char* app_name = argv[0];
    printf("\nUsage:   %s [-option] [parameter]", app_name);
    printf("\n--------------- options and values ------------------");
    printf("\n -start &                 #run as daemon in background");
    printf("\n -stop                    #stop running %s daemon", app_name);
    printf("\n -dev [name]              #specify the UART device name, by default is ttyS0");
    printf("\n -ip [IP address]         #specify the IP address where the message be sent to");
    printf("\n -port [IP port]          #specify the IP port where the message be sent to");
    printf("\n -baud [baud rate]        #specify the rate at which data is transferred through the UART port");
    printf("\n -version                 #show current version(build time)");
    printf("\n -help                    #print this information\n");
}

/*
 * The argv parameters are:
 * 1: <specifies if connect to GPS> //if not connected, type in "0', else type in anything else
 * 2: <specifies the IP address> //if not required, type in "0"
 * 3: <specifies the port number> //if not required, type in "0" or leave blank
 */
int main(int argc, char** argv) {
    int cmd = read_cmdline_options(argc, argv);

    switch (cmd) {
        case CMD_LINE_START:    return uart_tunnel_service();
        case CMD_LINE_STOP:     ShutdownInstance();     break;
        case CMD_LINE_HELP:     print_help(argv);       break;
        case CMD_LINE_VERSION:  printf("Version %s built(%s %s)\n", static_version, __DATE__, __TIME__);
        default:;
    }
    return (EXIT_SUCCESS);
}

