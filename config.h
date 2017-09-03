/*
 * config.h
 *
 *      Author: hata
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define	LINE_MAX	256	

#define	C_NAME	"emon.conf"
typedef struct {
	char mailto[MAIL_MAX][LINE_MAX];
	char *to[MAIL_MAX];
	char from[LINE_MAX];
	char smtp[LINE_MAX];
	char user64[LINE_MAX];
	char pass64[LINE_MAX];
	int auth;
}C_MAIL;

typedef struct{
	int daemon;
	C_MAIL	mail;
	int verbose;
	int start;
	int daily;
}C_CONFIG;

int load_config(char *name,C_CONFIG *cfg);

#endif /* CONFIG_H_ */
