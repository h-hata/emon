/*
 * htcp.h
 *
 *  Created on: 2013/06/08
 *      Author: hata
 */
#ifndef HTCP_H_
#define HTCP_H_
#define	E_OK			0
#define E_SOCK_PARAM	0xF0000010
#define E_SOCK_SOCK		0xF0000020
#define E_SOCK_BIND		0xF0000030
#define E_SOCK_HENT		0xF0000040
#define E_SOCK_CONNECT	0xF0000050
#define E_SOCK_SEND		0xF0000060
#define E_SOCK_SHUTDOWN	-1
#define E_SOCK_TIMEOUT	0xF0000080
#define E_SOCK_RECV		0xF0000090
#define E_SOCK_ACCEPT	0xF00000A0
#define	E_SOCK_CALLBACK	0xF00000B0
#define	E_SOCK_LISTEN	0xF00000C0
#define	E_SOCK_FCTL		0xF00000D0
#define	E_SOCK_SELECT	0xF00000E0
#define	E_SOCK_ALLOC	0xF00000F0

int SendToTCPPeer(int socket, char *sbuffer,size_t slen);
int RecvFromTCPPeerByOne(int socket,char *rbuffer,int *rlen, int time_out);
int MakeConnectPeerWithTimer(char *peerIP,unsigned short peerPort,int *sock,int sec);


#endif /* HTCP_H_ */
