#ifndef		__WYY_SOCKET_H__
#define		__WYY_SOCKET_H__

#ifdef	__cplusplus
extern "C" {
#endif
#include    <pthread.h>
#include    <syslog.h>

#ifndef SOCKET_TYPES
#define SOCKET_TYPES

#define	MAX_BUFF_SIZE			4096
#define	MAX_CONNECTIONS			128

#define	MODE_TCPSERVER			1
#define	MODE_TCPCLIENT			2
#define	MODE_UDPIP			3

#define	SOCKET_SYNCHRONOUS		0	//	blocking mode,		accept() µÈŽýœá¹ûºó²Å·µ»Ø
#define	SOCKET_ASYNCHRONOUS             1	//	nonblocking mode£¬	accept() Á¢ŒŽ·µ»Ø

#define bool                            int
#define true                            1
#define false                           0
#define	SOCKET				int
#define	LPSTR				char*
#define	LPVOID				void*
#define	BYTE				unsigned char
#define	WORD				unsigned short
#define	DWORD				unsigned long
#define	INVALID_SOCKET			-1
#define	INVALID_THREAD			(pthread_t) 0xFFFFFFFF
#define	SOCKET_ERROR			-1
#define	SOCKADDR			struct sockaddr
#define	PSOCKADDR			SOCKADDR*
#define	SOCKADDR_IN			struct sockaddr_in
#define	IN_ADDR				struct in_addr
#define	ZeroMemory			bzero
#define	HANDLE				int

#define	WSAEWOULDBLOCK			EWOULDBLOCK	//EAGAIN
#define	WSAECONNRESET			ECONNRESET
#define	WSAENOTSOCK			ENOTSOCK
#define	DEBUG_BASE_BLACK                LOG_WARNING
#define	DEBUG_BASE_BLUE                 LOG_INFO
#define	DEBUG_CONN_BLACK                LOG_WARNING
#define	DEBUG_CONN_BLUE                 LOG_NOTICE
#define	DEBUG_CONN_GREEN                LOG_INFO
#define	DEBUG_TCPS_BLACK                LOG_WARNING
#define	DEBUG_UDPS_BLACK                LOG_WARNING

#endif  //SOCKET_TYPES

void Set_Socket_Debug(int en);
bool Get_Socket_Debug();
#define Socket_TRACE(mask,fmt,args...) do {\
        if (mask) syslog(mask, "libgws_sk:  (%d) " fmt, __LINE__,##args); } while (0)
//        if (mask) syslog(mask, "libgws_sk:  %s(%d) " fmt,__func__,__LINE__,##args); } while (0)
//        if (Get_Socket_Debug() & mask) syslog(LOG_WARNING, \
//void Socket_TRACE(int nColor,char* fmt,...);

typedef struct __SocketBase {
    int m_nRecv;
    BYTE m_buffer[MAX_BUFF_SIZE];
    SOCKET m_socket;
    DWORD m_dwRemoteAdd;
    WORD m_wRemotePort;
    BYTE m_host_ip[4];
    short m_PortNo;
    bool m_mcast;
} SOCKET_BASE;

typedef int     (*PIPE_CALLBACK_ON_SEND)(SOCKET_BASE* pParam,BYTE* buf_send);
typedef void    (*PIPE_CALLBACK_ON_RECV)(SOCKET_BASE* pParam,BYTE* buf_recv,int nRecv);

typedef struct __SocketPipe {
    int m_pipe_er[2];
    int m_pipe_up[2];
    int m_pipe_dn[2];
} SOCKET_PIPE;

typedef struct __SocketConn {
    SOCKET_BASE sock_base;
    char m_multicastIP[16];
    pthread_cond_t m_cond;
    pthread_mutex_t m_lock;
    pthread_mutex_t m_receiveEvent;
    pthread_t m_receiveThread;
    PIPE_CALLBACK_ON_SEND    m_pOnSend;
    PIPE_CALLBACK_ON_RECV    m_pOnRecv;
} SOCKET_CONN;

///////////////////////////////////////////////////////////////
typedef struct __UdpServer {
    char            m_remoteUniIP[16];
    pthread_t       m_udpsvrThread;
    pthread_mutex_t m_udpsvrEvent;
    SOCKET_CONN     m_connection;
} SOCKET_UDP_SERVER;

///////////////////////////////////////////////////////////////
typedef struct __TcpServer {
    pthread_t       m_listenThread;
    pthread_mutex_t m_listenEvent;
    SOCKET_BASE     m_listener;
    SOCKET_CONN     m_connection[MAX_CONNECTIONS];
    bool            m_redir_stdout;
} SOCKET_TCP_SERVER;


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
LPSTR IPconvertN2A(DWORD dwIP);
DWORD IPconvertA2N(const char* ip);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void Socket_Init(SOCKET_BASE* sock_base,short PortNo);
bool Socket_Bind(SOCKET_BASE* sock_base,int type, short port, LPSTR multicastIP); //multicastIP= NULL
DWORD Socket_Accept(SOCKET_BASE* sock_base,SOCKET listener);
bool Socket_Connect(SOCKET_BASE* sock_base,int b1, int b2, int b3, int b4, int port);
bool Socket_IsConnected(SOCKET_BASE* sock_base);
void Socket_Disconnect(SOCKET_BASE* sock_base);
void Socket_RedirectStdout(SOCKET_BASE* sock_base, bool on_off); 

int Socket_RecvData(SOCKET_BASE* sock_base, DWORD size); /* TCP/IP */
int Socket_SendData(SOCKET_BASE* sock_base, LPSTR buffer, DWORD size); /* TCP/IP */

char* Socket_RecvFromS(SOCKET_BASE* sock_base,LPSTR ip, int port, DWORD* size); /* UDP/IP */
int Socket_RecvFromN(SOCKET_BASE* sock_base,DWORD* ip, int port, DWORD size); /* UDP/IP */
int Socket_SendTo(SOCKET_BASE* sock_base, LPSTR ip, int port, LPSTR buffer, DWORD size); /* UDP/IP */
BYTE* Socket_GetData(SOCKET_BASE* sock_base,int* nSize);

/////////////////   utilities ///////////////////////////////////
DWORD Socket_GetIP(const char *if_name);
DWORD Socket_GetRemoteAdd(SOCKET_BASE* sock_base);
WORD Socket_GetRemotePort(SOCKET_BASE* sock_base);
void Socket_SetRemotePort(SOCKET_BASE* sock_base,WORD port);
short Socket_GetPortNo(SOCKET_BASE* sock_base);
void Socket_SetPortNo(SOCKET_BASE* sock_base,short portNo);

bool SocketConn_IsUpdate(SOCKET_CONN* socket_conn);
void SocketConn_Release(SOCKET_CONN* socket_conn);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SocketPipe_Init();
void SocketPipe_Exit();
void SocketPipe_CreatePipe();
int* SocketPipe_GetRecvPipe();       //reading end of the recv pipe
int* SocketPipe_GetSendPipe();       //writing end of the send pipe
bool SocketPipe_RedirectStdIO();
int SocketPipe_TryRead(int fd,BYTE* buff,int nMax);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SocketConn_Init(SOCKET_CONN* socket_conn,short PortNo,PIPE_CALLBACK_ON_SEND pOnSend,PIPE_CALLBACK_ON_RECV pOnRecv);
void SocketConn_Exit(SOCKET_CONN* socket_conn);

bool SocketConn_TCP_Bind(SOCKET_BASE* listener);    //For TCPIP
DWORD SocketConn_Accept(SOCKET_CONN* socket_conn,SOCKET_BASE* listener);
int SocketConn_TCP_Send(SOCKET_CONN* socket_conn,LPSTR buffer, DWORD size);
int SocketConn_TCP_Recv(SOCKET_CONN* socket_conn,DWORD size);
bool SocketConn_Connect(SOCKET_CONN* socket_conn,int b1, int b2, int b3, int b4, short port);
void SocketConn_DisConnect(SOCKET_CONN* socket_conn);

bool SocketConn_UDP_Bind(SOCKET_CONN* socket_conn,LPSTR multicastIP, short port); //	for UDP/IP (datagram)
int SocketConn_UDP_SendN(SOCKET_CONN* socket_conn,DWORD ip, WORD port, LPSTR buffer, DWORD size);
int SocketConn_UDP_SendS(SOCKET_CONN* socket_conn,LPSTR ip, WORD port, LPSTR buffer, DWORD size);
int SocketConn_Multicast(SOCKET_CONN* socket_conn,WORD port, LPSTR buffer, DWORD size);
int SocketConn_UDP_RecvN(SOCKET_CONN* socket_conn,DWORD* ip, WORD port, DWORD size);
LPSTR SocketConn_UDP_RecvS(SOCKET_CONN* socket_conn,LPSTR ip, WORD port, DWORD* size);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Following are the interfaces for UDP/TCP server
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//  For UDP server
bool UdpServer_Init(SOCKET_UDP_SERVER* server, short PortNo,
                    PIPE_CALLBACK_ON_SEND pOnSend,
                    PIPE_CALLBACK_ON_RECV pOnRecv);
bool UdpServer_StartListenThread(SOCKET_UDP_SERVER* server, LPSTR multicastIP, short port);
void UdpServer_StopListenThread(SOCKET_UDP_SERVER* server);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//  For TCP server
bool TcpServer_Init( SOCKET_TCP_SERVER* server, short PortNo,
                        PIPE_CALLBACK_ON_SEND pOnSend,
                        PIPE_CALLBACK_ON_RECV pOnRecv);
bool TcpServer_StartListenThread(SOCKET_TCP_SERVER* server);
void TcpServer_StopListenThread(SOCKET_TCP_SERVER* server);
void UdpServer_SetDestAddr(SOCKET_BASE* sock,LPSTR ip,WORD port);   //call this function within OnSend()
void UdpServer_SetDestIpPort(SOCKET_BASE* sock,DWORD ip,WORD port); //call this function within OnSend()
DWORD UdpServer_GetDestIP(SOCKET_BASE* sock);
//  The following utilities are usually for internal uses
int TcpServer_Broadcast(SOCKET_TCP_SERVER* server,char* buffer, DWORD size);
int TcpServer_Browse(SOCKET_TCP_SERVER* server,int* index); //index=EOF或返回数据长度值
int TcpServer_Recv(SOCKET_TCP_SERVER* server,int index); //为0，均表示无数据
int TcpServer_Scan(SOCKET_TCP_SERVER* server,int* index); //扫描所有端口
int TcpServer_Send(SOCKET_TCP_SERVER* server,int index, char* buffer, DWORD size);
int TcpServer_GetConnectionCount(SOCKET_TCP_SERVER* server); //返回目前已经被占用的通道数
bool TcpServer_DisConnect(SOCKET_TCP_SERVER* server,int index); //断开索引为index的通道
BYTE* TcpServer_GetData(SOCKET_TCP_SERVER* server, int index);
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef	__cplusplus
}
#endif

#endif		//__WSOCKET_H__
