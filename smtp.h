#define MAIL_MAX	9
extern int MakeHeader(
		char *buff,
		int len,
		char *to[],
		char *from,
		char *sbj,
	 	char *data);
extern int SendMail(
	char *smtp,
	int port,
	char *to[],
	char *from,
	char *data,
	int dlen,
	char *user64,
	char *pass64);
