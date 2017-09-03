#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "rbtree.h"

#define BUFF 1024
#define FNAME "oui.txt"
#ifdef MAIN
typedef struct {
	int f;
	char k[128];
	char v[128];
} TBL;
TBL tbl[20000];
#endif
RBTree t;
static void rmCR(char *p)
{
	for(;;p++){
		if(*p=='\n'||*p=='\r'||*p=='\0'){
			*p='\0';
			break;
		}
	}
}

static char *skip(char *p)
{
	for(;;p++){
		if(*p=='\t'||*p==' '){
			continue;
		}
	return p;
	}
}

static void macformat(char *ptr)
{
	if(*(ptr+2)=='-'){
		*(ptr+2)=':';
	}
	if(*(ptr+5)=='-'){
		*(ptr+5)=':';
	}
}

static int separate(char *line,char **term,int col,int blen)
{
	int i,loc;
	char *ptr;
	char *p;

	p=line;
	rmCR(p);
	for(i=0;i<col;i++){
		ptr=*term++;
		p=skip(p);
		for(loc=0;;loc++){
			if(loc>=blen-1){
				return i;
			}
			if(*p=='\0'){
				ptr[loc]='\0';
				if(i+1==col){
					i++;
				}
				return i;
			}
			if(*p==' ' || *p=='\t'){
				if(i+1!=col){
					ptr[loc]='\0';
					break;
				}
			}
			ptr[loc]=*p++;
		}
	}
	return i;
}
static void uppercase(char *ptr)
{
	for(;*ptr;ptr++){
		if(*ptr>='a' && *ptr<='z'){
			*ptr= *ptr - ('a'-'A') ;
		}
	}
}

int OUI_load(void)
{
	char line[BUFF];
	FILE *fp;
	char term[3][BUFF];
	char *pterm[3];
	int i;
	int f;
	int ret;
	int n=0;
	int s;
	int e;

	t=RB_new();
	if(t==NULL){
		return 0;
	}
	for(i=0;i<3;i++){
		pterm[i]=term[i];
	}
	fp=fopen(FNAME,"r");
	if(fp==NULL){
		return 0;
	}
	f=0;
	s=e=0;
	for(i=1;;i++){
		if(fgets(line,BUFF,fp)==NULL){
			break;
		}
		n=separate(line,pterm,3,BUFF);
		if(3!=n){
			continue;
		}
		if(strcmp(term[1],"(hex)")!=0){
			continue;
		}
		macformat(term[0]);
		uppercase(term[2]);
#ifdef MAIN
		tbl[f].f=1;
		strcpy(tbl[f].k,term[0]);
		strcpy(tbl[f].v,term[2]);
		f++;
#endif
		if((ret=RB_insert(t,term[0],term[2]))!=0){
			if(ret!=-30){
				break;
			}
			e++;
			continue;
		}else{
			s++;
		}
	}
	fclose(fp);
	return s;
}

char *OUI_search(char *key)
{
	return RB_search(t,key);
}



#ifdef MAIN
main()
{
	int i,j;
	char *c;
	for(i=0;i<20000;i++){
		tbl[i].f=0;
	}
	OUI_load("oui.txt");
	j=0;
	for(i=0;i<20000;i++){
		if(tbl[i].f==1){
			c=RB_search(t,tbl[i].k);
			if(c==NULL){
				printf("c NULL k=%s\n",tbl[i].k);
				continue;
			}
			if(strcmp(c,tbl[i].v)!=0){
				printf("c Unmatch %s %s %s\n",tbl[i].k,tbl[i].v,c);
				continue;
			}
			j++;
		}
	}
	printf("%d compared\n",j);
	printf("%s\n",RB_search(t,"08:00:30"));
}
#endif
