/*
 * config.c
 *
 *      Author: hata
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "smtp.h"
#include "config.h"

static char *skip(char *ptr)
{
	for(;*ptr==' '||*ptr=='\t';ptr++){
	}
	return ptr;
}
static void trim(char *ptr)
{
	for(;*ptr;ptr++){
		if(*ptr==';' || *ptr=='\n' || *ptr=='\r' ){
			*ptr='\0';
			break;
		}
	}
}

int load_config(char *name,C_CONFIG *cfg)
{
	char	cname[LINE_MAX];
	FILE	*fp;
	char	line[256];
	char	tmp[256];
	int n,ret;
	char *cptr;
	int flag;
	int i;

	ret=0;
	memset(cfg,0,sizeof(C_CONFIG));
	flag=0;
	if(name==NULL){
		strcpy(cname,C_NAME);
	}else if (strlen(name)>LINE_MAX-1){
		return -1;
	}else if(*name=='\0'){
		strcpy(cname,C_NAME);
	}else{
		strcpy(cname,name);
	}
	fp=fopen(cname,"r");
	if(fp==NULL){
		return -2;
	}
	for(i=0;i<MAIL_MAX;i++){
		cfg->mail.to[i]=cfg->mail.mailto[i];
	}
	for(i=0;;){
		//一行読み込み
		if(fgets(line,255,fp)==NULL){
			break;
		}
		cptr=skip(line);
		trim(cptr);
		//コメント行は読み飛ばし
		if(*cptr=='#'||*cptr==';' || *cptr=='\n'||*cptr=='\r'||*cptr=='\0'){
			continue;
		}
		if(strncmp(cptr,"TO ",3)==0){
			if(i<=MAIL_MAX){
				n=sscanf(cptr ,"%s %s",tmp,cfg->mail.mailto[i]);
				if(n!=2){
					ret=-3;
					break;
				}
				flag|=1;
				i++;
			}
		}else if(strncmp(cptr,"FROM ",5)==0){
			n=sscanf(cptr ,"%s %s",tmp,cfg->mail.from);
			if(n!=2){
				ret=-4;
				break;
			}
			flag|=2;
		}else if(strncmp(cptr,"SMTP ",5)==0){
			n=sscanf(cptr ,"%s %s",tmp,cfg->mail.smtp);
			if(n!=2){
				ret=-5;
				break;
			}
			flag|=4;
		}else if(strncmp(cptr,"USER64 ",7)==0){
			n=sscanf(cptr ,"%s %s",tmp,cfg->mail.user64);
			if(n!=2){
				ret=-6;
				break;
			}
			flag|=8;
		}else if(strncmp(cptr,"PASS64 ",7)==0){
			n=sscanf(cptr ,"%s %s",tmp,cfg->mail.pass64);
			if(n!=2){
				ret=-7;
				break;
			}
			flag|=16;
		}else if(strncmp(cptr,"DAEMON",6)==0){
			cfg->daemon=1;
		}else if(strncmp(cptr,"VERBOSE",7)==0){
			cfg->verbose=1;
		}else if(strncmp(cptr,"STARTMAIL",9)==0){
			cfg->start=1;
		}else if(strncmp(cptr,"DAILYMAIL",9)==0){
			cfg->daily=1;
		}
	}
	fclose(fp);
	if(ret!=0){
		return ret;
	}
	if(flag!=7 && flag!=31){
		return -9;
	}
	if(flag==0x11111){
		cfg->mail.auth=1;
	}
	if(cfg->daemon==1){
		cfg->verbose=0;
	}
	return ret;
}
