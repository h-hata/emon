#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "smtp.h"
#include "config.h"
#include "smtp.h"
#include "oui.h"
static int s=-1;
static C_CONFIG *conf;
#define REC_NUM 1024
typedef struct {
	int used;
	time_t update;
	time_t rep;
	char smac[20];
	char sip[16];
	int protocol;
	int port;
	int pkt;
}RECORD;
static RECORD rec[REC_NUM];

void rec_init(void)
{
	int i;
	for(i=0;i<REC_NUM;i++){
		memset(&rec[i],0,sizeof(RECORD));
	}
}
void rec_insert(char *smac,char *sip,int protocol,int port)
{
	int i;
	for(i=0;i<REC_NUM;i++){
		if(rec[i].used==0){
			rec[i].used=1;
			strcpy(rec[i].smac,smac);
			strcpy(rec[i].sip,sip);
			rec[i].protocol=protocol;
			rec[i].port=port;
			rec[i].pkt=1;
			rec[i].update=time(NULL);
			rec[i].rep=time(NULL);
			return;
		}
	}
}
int rec_search(char *smac,char *sip,int protocol,int port,
		time_t *rep,int *pkt){
	int i;
	for(i=0;i<REC_NUM;i++){
		if(rec[i].used==0){
			continue;
		}
		if(strcmp(smac,rec[i].smac)!=0){
			continue;
		}
		if(strcmp(sip,rec[i].sip)!=0){
			continue;
		}
		if(rec[i].protocol!=protocol){
			continue;
		}
		if(rec[i].port!=port){
			continue;
		}
		if(rep!=NULL){
			*rep=rec[i].rep;
		}
		if(pkt!=NULL){
			*pkt=rec[i].pkt;
		}
		return i;
	}
	return -1;
}

void rec_update(int i)
{
	if(rec[i].used==0){
		return ;
	}
	rec[i].pkt++;
	rec[i].update=time(NULL);
}
void rec_update_rep(int i)
{
	if(rec[i].used==0){
		return ;
	}
	rec[i].rep=time(NULL);
}
void rec_delete(int i)
{
	rec[i].used=0;
}
static void ascdate(time_t *t,char *p,int plen)
{
	struct tm *l;
	char *a;
	l=localtime(t);
	a=asctime(l);
	memset(p,0,plen);
	if(plen>strlen(a)){
		strncpy(p,a,strlen(a));
		p[strlen(p)-1]='\0';
	}
	return;
}
static void just_connected_mail
(char *smac,char *vendor,char *sip,int protocol,int sport,int dport)
{
	char sbj[256];
	char host[256];
	char atime[256];
	char vname[256];
	char data[1024];
	char msg[1024];
	time_t t;
	int ret;

	t=time(NULL);
	gethostname(host,sizeof(host));
	ascdate(&t,atime,sizeof(atime));
	if(vendor==NULL){
		strcpy(vname,"Unknown");
	}else{
		strcpy(vname,vendor);
	}
	sprintf(sbj,"Emon Irregular Report from %s",host);
	sprintf(data,"Windows packet detected on %s at %s\nThis host is detected just now\nMAC:%s\nVendor:%s\nIP:%s\nProtocol:%d\nSourcePort:%d\nDestinationPort:%d\n",
			host,atime,smac,vname,sip,protocol,sport,dport);
	ret=MakeHeader(msg,1000,conf->mail.to,conf->mail.from ,sbj,data);
	if(ret!=0){
		return ;
	}
	ret=SendMail(
		conf->mail.smtp, 25, conf->mail.to,conf->mail.from,
		msg, strlen(msg),conf->mail.user64,conf->mail.pass64);
}

static void start_mail(void)
{
	char sbj[256];
	char host[256];
	char atime[256];
	char data[1024];
	char msg[1024];
	time_t t;
	int ret;

	t=time(NULL);
	gethostname(host,sizeof(host));
	ascdate(&t,atime,sizeof(atime));
	sprintf(data,"Emon start on %s at %s",host,atime);
	sprintf(sbj,"Emon start on %s",host);
	ret=MakeHeader(msg,1000,conf->mail.to,conf->mail.from ,sbj,data);
	if(ret!=0){
		return ;
	}
	ret=SendMail(
		conf->mail.smtp, 25, conf->mail.to,conf->mail.from,
		msg, strlen(msg),conf->mail.user64,conf->mail.pass64);
}

static void *cycle_thread(void *arg)
{
	for(;;){
		sleep(1);
	}
	return NULL;
}

static void put_record(
		char *smac,
		char *vendor,
		char *sip,
		int protocol,
		int sport,
		int dport)
{
	int ret;
	time_t t;
	time_t rep;
	int pkt;

	t=time(NULL);	
	ret=rec_search(smac,sip,protocol,dport,&rep,&pkt);
	if(ret==-1){
		rec_insert(smac,sip,protocol,dport);
		if(conf->daemon==0){
			fprintf(stdout,"Just Connected Host:%s %s %d %d\n",
					smac,sip,protocol,dport);
		}
		just_connected_mail(smac,vendor,sip,protocol,sport,dport);
	}else {
		rec_update(ret);
		if(t-rep > 60*60*6){//6Hours
			rec_update_rep(ret);
			if(conf->daemon==0){
				fprintf(stdout,"Reported Host:%s %s %d %d %d %ld \n",
					smac,sip,protocol,dport,pkt,t-rep);
			}
			//reported_connected_mail(smac,sip,protocol,port,pkt,t-rep);
		}else{
			if(conf->daemon==0){
				fprintf(stdout,"Update:%s %s %d %d %d %ld \n",
					smac,sip,protocol,dport,pkt,t-rep);
			}
		}
	}
	return;
}



int setup_inf(char *eth,C_CONFIG *cnf)
{
	pthread_t tid;
	int n;
	conf=cnf;
	s=socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(s<0){
		return -1000;
	}
	if(0!=pthread_create(&tid,NULL,cycle_thread,NULL)){
		close(s);
		return -1100;
	}
	n=OUI_load();
	return 0;
}

void analyze(char *buf,ssize_t len)
{
	struct ether_header *eth;
	struct iphdr *ip;
	struct tcphdr *tcp;
	struct udphdr *udp;
	char smac[20], dmac[20];
	char soui[16];
	char *vendor=NULL;
	char sip[16],dip[16];
	int protocol;

	eth=(struct ether_header *)buf;
	sprintf(dmac, "%02x:%02x:%02x:%02x:%02x:%02x",
		eth->ether_dhost[0], eth->ether_dhost[1],
		eth->ether_dhost[2], eth->ether_dhost[3],
		eth->ether_dhost[4], eth->ether_dhost[5]);
	sprintf(smac, "%02x:%02x:%02x:%02x:%02x:%02x",
		eth->ether_shost[0], eth->ether_shost[1],
		eth->ether_shost[2], eth->ether_shost[3],
		eth->ether_shost[4], eth->ether_shost[5]);
	sprintf(soui, "%02X:%02X:%02X",
		eth->ether_shost[0], eth->ether_shost[1],
		eth->ether_shost[2]);
	protocol = ntohs(eth->ether_type);
	vendor=OUI_search(soui);
	if(conf->verbose==1){
	  fprintf(stdout,"\nMAC : %s >> %s (Protocol=%04x, size=%ldbyte)\n",
		smac, dmac, protocol, len);
		if(vendor!=NULL){
	  	fprintf(stdout,"Vendor : %s\n",vendor);
		}
	}
	switch(protocol){
		case ETHERTYPE_IP:
			ip = (struct iphdr *)(buf + sizeof(struct ether_header));
			sprintf(sip, "%s", inet_ntoa(*(struct in_addr *)&(ip->saddr)));
			sprintf(dip, "%s", inet_ntoa(*(struct in_addr *)&(ip->daddr)));
			if(conf->verbose==1){
				fprintf(stdout, "IP %s > %s (%dbyte)\n",
					sip, dip, ntohs(ip->tot_len));
				printf("IP->PROTOCOL=%d\n",ip->protocol);
			}
			if(ip->protocol==6){//TCP
				tcp=(struct tcphdr *)(buf + 
						sizeof(struct ether_header)+
						sizeof(struct iphdr));
				if(conf->verbose==1){
					printf("TCP PORT %d => %d\n",
						ntohs(tcp->source),ntohs(tcp->dest));
				}
				if(
					ntohs(tcp->dest)==135||ntohs(tcp->dest)==135||
					ntohs(tcp->dest)==137||ntohs(tcp->dest)==137||
					ntohs(tcp->dest)==138||ntohs(tcp->dest)==138||
					ntohs(tcp->dest)==139||ntohs(tcp->dest)==139||
					ntohs(tcp->dest)==445||ntohs(tcp->dest)==445
					){
					if((vendor=OUI_search(soui))!=NULL){
						if(strncmp(vendor,"APPLE",5)!=0){
							put_record(smac,sip,vendor,ip->protocol,
									ntohs(tcp->source),ntohs(tcp->dest));
						}
					}
				}
			}else if(ip->protocol==17){//UDP
		udp=(struct udphdr *)(buf + 
						sizeof(struct ether_header)+
						sizeof(struct iphdr));
				if(conf->verbose==1){
					printf("UDP PORT %d => %d\n",
						ntohs(udp->source),ntohs(udp->dest));
				}
				if(
					ntohs(udp->dest)==135||ntohs(udp->dest)==135||
					ntohs(udp->dest)==137||ntohs(udp->dest)==137||
					ntohs(udp->dest)==138||ntohs(udp->dest)==138||
					ntohs(udp->dest)==139||ntohs(udp->dest)==139||
					ntohs(udp->dest)==445||ntohs(udp->dest)==445
					){
					if((vendor=OUI_search(soui))!=NULL){
						if(strncmp(vendor,"APPLE",5)!=0){
							put_record(smac,vendor,sip,ip->protocol,
									ntohs(udp->source),ntohs(udp->dest));
						}
					}
				}
			}
			break;
		case ETHERTYPE_ARP:
			if(conf->verbose==1)
				fprintf(stdout, "ARP\n");
			break;
		case ETHERTYPE_REVARP:
			if(conf->verbose==1)
				fprintf(stdout, "Reverse ARP\n");
			break;
		default:
			if(conf->verbose==1)
				fprintf(stdout, "other protocol\n");
		break;
	}
}

int start_mon(void)
{
	ssize_t len;
	int ret=0;
	char	buff[2000];
	if(s<0||conf==NULL){
		return -1200;
  	}
	if(conf->start==1){
		start_mail();
	}
	for(;;){
		len=recv(s,buff,sizeof(buff),0);
		if(len<0){
			perror("recv");
			ret=-1300;
			break;
		}
		analyze(buff,len);	
	}
	close(s);
	s=-1;
	return ret;
}


