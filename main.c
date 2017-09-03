#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "smtp.h"
#include "config.h"
#include "emon.h"
static void daemonize(void)
{
	int ret;
	//1回目
//	fclose(stdin);
//	fclose(stdout);
//	fclose(stderr);
	ret=fork();
	if(ret>0){
		//親プロセス
		exit(EXIT_SUCCESS);
	}else if(ret<0){
		exit(1);
	}   
	//2回目
	ret=fork();
	if(ret>0){
		//親プロセス
		exit(EXIT_SUCCESS);
	}else if(ret<0){
		exit(1);
	}
}
		
int main()
{
	int ret;
	C_CONFIG conf;

	if((ret=load_config(NULL,&conf))!=0){
		printf("load_config error %d\n",ret);
		_exit(0);
	}

	if(conf.daemon==1){
		daemonize();
	}
 	ret=setup_inf(NULL,&conf);
	if(ret!=0){
		if(conf.daemon==0){
	  		printf("setup_inf = %d\n",ret);
		}
	 	_exit(0);
	}
	ret=start_mon();
	if(conf.daemon==0){
		printf("start_mon = %d\n",ret);
	}
	return 0;
}

