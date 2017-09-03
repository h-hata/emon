/*
 * htcp.c
 *
 *  Created on: 2013/06/08
 *      Author: hata
 */


#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<netinet/in.h>

#include	<stdio.h>
#include	<string.h>
#include	<netdb.h>
#include	<fcntl.h>
#include	<pwd.h>
#include	<time.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<errno.h>
#include	<sys/wait.h>
#include	<errno.h>
#include	<sys/socket.h>
#include	<sys/ioctl.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include 	<pthread.h>
#include 	<stdint.h>
#include	<malloc.h>
#include	"htcp.h"


#define	BUFF	64*1024


int MakeConnectPeerWithTimer(char *peerIP,unsigned short peerPort,int *sock,int sec)
{
	struct sockaddr_in	m_addr;
	struct sockaddr_in	p_addr;
	struct hostent *hent;
	int	s;
	int	ret;
	fd_set set;
	struct timeval tv;
	int val;
	if(peerIP == NULL||sock == NULL) return E_SOCK_PARAM;

	/*socket*****************************************************/
	s=socket(PF_INET,SOCK_STREAM,0);
	if(s<0) return E_SOCK_SOCK;
	/*bind********************************************************/
	m_addr.sin_family = AF_INET;
	m_addr.sin_port=0;
	m_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(s,(struct sockaddr *)&m_addr,sizeof(m_addr)) == -1){
		close(s);
		return E_SOCK_BIND;
	}
	/*Convert HOST to IP Address**********************************/
	p_addr.sin_addr.s_addr=inet_addr(peerIP);
	if(p_addr.sin_addr.s_addr == INADDR_NONE){
		hent=gethostbyname(peerIP);
		if(hent==NULL){
			close(s);
			return E_SOCK_HENT;
		}
		p_addr.sin_addr.s_addr=*(unsigned long *)hent->h_addr_list[0];
	}
	/*connect*****************************************************/
	p_addr.sin_family =AF_INET;
	p_addr.sin_port=htons(peerPort);

	/*set socket to NONBLOCKING MODE******************************/
	ret = fcntl(s, F_SETFL, O_NONBLOCK);
	if(ret < 0) {
		close(s);
		return E_SOCK_FCTL;
	}
	/* 1st connect, it return -1**********************************/
	ret=connect(s,(struct sockaddr *)&p_addr,sizeof(p_addr));
	if(ret<0){
		if(errno == EINPROGRESS) {
			tv.tv_sec = sec;
			tv.tv_usec = 0;
			FD_ZERO(&set);
			FD_SET(s, &set);
			ret =select(s+1,NULL,&set,NULL,&tv);
				if(ret < 0 ) {
				close(s);
				return E_SOCK_SELECT;
			} else if( ret > 0 ) {
				/*connect again***********************************/
				ret=connect(s,(struct sockaddr *)&p_addr,sizeof(p_addr));
				if(ret<0){
					if(errno != EISCONN) {
						close(s);
						return E_SOCK_CONNECT;
					}
				}
			} else {
				/** Time out **/
				close(s);
				return E_SOCK_TIMEOUT;
			}
		} else {
			close(s);
			return E_SOCK_CONNECT;
		}
	}
	val=0;
	ioctl(s,FIONBIO,&val);//Blocking
	*sock=s;
	return E_OK;
}


int RecvFromTCPPeerByOne(int sock,char *rbuffer,int *rlen, int time_out)
{
	ssize_t	  nByte;//TR-14
	struct timeval tv;
	time_t	start,curr;
	int val;

	if(rbuffer == NULL ||sock<0 || rlen==NULL) return E_SOCK_PARAM;
	if(*rlen<64) return E_SOCK_PARAM;
	val=0;
	ioctl(sock,FIONBIO,&val);//Blocking
	tv.tv_sec=time_out;
	tv.tv_usec=0;
	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
		return E_SOCK_PARAM;
	time(&start);
	for(;;){
		nByte=recv(sock,rbuffer,*rlen,0);
		if(nByte<=0){
			if(errno==EWOULDBLOCK||errno==EAGAIN||errno==EINTR){//TR-18
				time(&curr);
				if(curr>start){
					return E_SOCK_TIMEOUT;
				}else{
					if(curr==start){
						tv.tv_sec=1;
					}else{
						tv.tv_sec=curr-start;
					}
					tv.tv_usec=0;
					if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
							return E_SOCK_PARAM;
					continue;
				}
			}
			return E_SOCK_SHUTDOWN;
		}
		*rlen=nByte;
		break;
	}
	return E_OK;
}


int SendToTCPPeer(int sock,char *sbuffer, size_t slen)
{
	ssize_t	  nByte;//TR-14
	ssize_t	nByteToSend;
	ssize_t nTotalByteToSend;
	int		bcount;
	int val=0;
	//struct timespec req={0,1000};
	struct timeval timeout;
	int ret;
	fd_set fdset;

	if(sbuffer == NULL )
		return E_SOCK_PARAM;
	if(slen==0)
		return E_SOCK_PARAM;
	if(sock < 0){
		return E_SOCK_PARAM;
	}
	val=0;
	ioctl(sock,FIONBIO,&val);//Blocking
	nTotalByteToSend=nByteToSend=slen;
	/*Send******************************************/
	for(bcount=0;nByteToSend;){
		nByte=send(sock,
				sbuffer+nTotalByteToSend-nByteToSend ,
				(int)nByteToSend,
				0);
		if(nByte<0){
				if(errno==EWOULDBLOCK||errno==EAGAIN||errno==EINTR){
					bcount++;
					//nanosleep(&req,NULL);
					FD_ZERO( &fdset ); 
					FD_SET( sock , &fdset );
					timeout.tv_sec = 4; 
					timeout.tv_usec = 0;
					/* writeできるかチェック */
					ret = select( sock+1 , NULL , &fdset , NULL , &timeout );
					if(ret==0){
#ifdef DEBUG
						printf("send select timeout\n");
#endif
						return E_SOCK_SEND;
					}
					continue;
				}
#ifdef DEBUG
				printf("send errno=%d sock=%d\n",errno,sock);
#endif
				return E_SOCK_SEND;
		}
		nByteToSend -= nByte;
	}
#ifdef DEBUG
	//printf("send return retry %d\n",bcount);
#endif
	return 0;
}


