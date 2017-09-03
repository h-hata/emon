#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "htcp.h"
#include "smtp.h"
#define BLEN 1500


int MakeHeader(char *buff,int len,char *to[],char *from , char *sbj,char *data){
	int clen;
	char line[1024];
	char tmp[1024];
	int i;

	if(buff==NULL){
		return -1;
	}
	*buff='\0';
	clen=strlen(buff);
	//To:
	sprintf(line,"To:");
	for(i=0;i<MAIL_MAX && to[i]!=NULL && *to[i]!='\0';i++){
		if(i==MAIL_MAX-1 || to[i+1]==NULL || *to[i+1]=='\0'){
			sprintf(tmp," %s\r\n",to[i]);
		}else{
			sprintf(tmp," %s,",to[i]);
		}
		if(strlen(line)+strlen(tmp)>1000){
			return -1;
		}
		strcat(line,tmp);
	}
	if(clen+strlen(line)>len-4){
		return -1;
	}
	strcpy(buff,line);
	clen=strlen(buff);
	//From:
	sprintf(line,"From: %s\r\n",from);
	if(clen+strlen(line)>len-4){
		return -1;
	}
	strcat(buff,line);
	clen=strlen(buff);
	//From:
	sprintf(line,"Subject: %s\r\n\r\n",sbj);
	if(clen+strlen(line)>len-4){
		return -1;
	}
	strcat(buff,line);
	clen=strlen(buff);
	//data
	if(clen+strlen(data)>len-4){
		return -1;
	}
	strcat(buff,data);
	clen=strlen(buff);
	//data
	if(clen+strlen(data)>len-4){
		return -1;
	}
	strcat(buff,"\r\n");
	return 0;	
}

int SendMail(
	char *smtp,
	int port,
	char *to[],
	char *from,
	char *data,
	int dlen,
	char *user64,
	char *pass64){
	char buff[BLEN];
	int len;
	int sock;
	int ret;
	int n;
	int i;
	char p1[BLEN];
	char p2[BLEN];
	char p4[BLEN];
	char p3[BLEN];


	if(to==NULL||from==NULL||data==NULL){
		return -4010;
	}
	if(strlen(user64)>256 || strlen(pass64)>256){
		return  -4020;
	}
	ret=MakeConnectPeerWithTimer(smtp,25,&sock,5);
	if(ret!=E_OK){
		return -4090;
	}

	len=BLEN-100;
	//220 check
	ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
	if(ret!=E_OK){
		close(sock);
		return -4100;
	}
	n=sscanf(buff,"%s %s",p1,p2);
	if(n!=2){
		close(sock);
		return -4110;
	}
	//EHLO
	sprintf(buff,"EHLO EMON\r\n");
	ret=SendToTCPPeer(sock,buff,strlen(buff));
	if(ret!=E_OK){
		close(sock);
		return -4120;
	}
	//250
	len=BLEN-100;
	ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
	if(ret!=E_OK){
		close(sock);
		return -4130;
	}
	n=sscanf(buff,"%s %s",p1,p2);
	if(n!=2){
		close(sock);
		return -4140;
	}
	if(strncmp(p1,"250",3)!=0){
		close(sock);
		return -4150;
	}
	if(user64!=NULL && *user64!='\0'){
		//AUTH LOGIN PLAIN
		sprintf(buff,"AUTH LOGIN\r\n");
		ret=SendToTCPPeer(sock,buff,strlen(buff));
		if(ret!=E_OK){
			close(sock);
			return -4160;
		}
		//Username:
		len=BLEN-100;
		ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
		if(ret!=E_OK){
			close(sock);
			return -4170;
		}
		n=sscanf(buff,"%s %s",p1,p2);
		if(n!=2){
			close(sock);
			return -4180;
		}
		if(strcmp(p1,"334")!=0 || strncmp(p2,"VXNlcm5hbWU6",12)!=0){
			close(sock);
			return -4190;
		}
		//Username:
		sprintf(buff,"%s\r\n",user64);
		ret=SendToTCPPeer(sock,buff,strlen(buff));
		if(ret!=E_OK){
			close(sock);
			return -4200;
		}
		//Password:
		len=BLEN-100;
		ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
		if(ret!=E_OK){
			close(sock);
			return -4210;
		}
		n=sscanf(buff,"%s %s",p1,p2);
		if(n!=2){
			close(sock);
			return -4220;
		}
		if(strcmp(p1,"334")!=0 || strncmp(p2,"UGFzc3dvcmQ6",12)!=0){
			close(sock);
			return -4230;
		}
		//pass64
		sprintf(buff,"%s\r\n",pass64);
		ret=SendToTCPPeer(sock,buff,strlen(buff));
		if(ret!=E_OK){
			close(sock);
			return -4240;
		}
		//OK
		len=BLEN-100;
		ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
		if(ret!=E_OK){
			close(sock);
			return -4240;
		}
		n=sscanf(buff,"%s %s %s %s",p1,p2,p3,p4);
		if(n!=4){
			close(sock);
			return -4220;
		}
		if(strcmp(p1,"235")!=0 || strcmp(p3,"OK")!=0){
			close(sock);
			return -4250;
		}
	}
	//MAIL FROM
	sprintf(buff,"MAIL FROM: %s\r\n",from);
	ret=SendToTCPPeer(sock,buff,strlen(buff));
	if(ret!=E_OK){
		close(sock);
		return -4260;
	}
	//250
	len=BLEN-100;
	ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
	if(ret!=E_OK){
		close(sock);
		return -4270;
	}
	n=sscanf(buff,"%s %s",p1,p2);
	if(n!=2){
		close(sock);
		return -4280;
	}
	if(strcmp(p1,"250")!=0 ){
		close(sock);
		return -4290;
	}
	//RECT TO
	for(i=0;i<MAIL_MAX && to[i]!=NULL && *to[i]!='\0' ;i++){
		sprintf(buff,"RCPT TO: %s\r\n",to[i]);
		ret=SendToTCPPeer(sock,buff,strlen(buff));
		if(ret!=E_OK){
			close(sock);
			return -4300-i;
		}
		//250
		len=BLEN-100;
		ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
		if(ret!=E_OK){
			close(sock);
			return -4310-i;
		}
		n=sscanf(buff,"%s %s",p1,p2);
		if(n!=2){
			close(sock);
			return -4320-i;
		}
		if(strcmp(p1,"250")!=0 ){
			close(sock);
			return -4330-i;
		}
	}
	//DATA
	sprintf(buff,"DATA\r\n");
	ret=SendToTCPPeer(sock,buff,strlen(buff));
	if(ret!=E_OK){
		close(sock);
		return -4340;
	}
	//354
	len=BLEN-100;
	ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
	if(ret!=E_OK){
		close(sock);
		return -4350;
	}
	n=sscanf(buff,"%s %s",p1,p2);
	if(n!=2){
		close(sock);
		return -4360;
	}
	if(strcmp(p1,"354")!=0 ){
		close(sock);
		return -4370;
	}
	//BODY
	ret=SendToTCPPeer(sock,data,strlen(data));
	if(ret!=E_OK){
		close(sock);
		return -4380;
	}
	ret=SendToTCPPeer(sock,"\r\n.\r\n",5);
	if(ret!=E_OK){
		close(sock);
		return -4390;
	}
	//250
	len=BLEN-100;
	ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
	if(ret!=E_OK){
		close(sock);
		return -4400;
	}
	n=sscanf(buff,"%s %s",p1,p2);
	if(n!=2){
		close(sock);
		return -4410;
	}
	if(strcmp(p1,"250")!=0 ){
		close(sock);
		return -4420;
	}
	//QUIT
	sprintf(buff,"QUIT\r\n");
	ret=SendToTCPPeer(sock,buff,strlen(buff));
	if(ret!=E_OK){
		close(sock);
		return -4700;
	}
	//OK
	len=BLEN-100;
	ret=RecvFromTCPPeerByOne(sock,buff,&len,5);
	if(ret!=E_OK){
		close(sock);
		return -4710;
	}
	n=sscanf(buff,"%s %s",p1,p2);
	if(n!=2){
		close(sock);
		return -4720;
	}
	if(strcmp(p1,"221")!=0 ){
		close(sock);
		return -4730;
	}
	close(sock);
	return 0;
}
