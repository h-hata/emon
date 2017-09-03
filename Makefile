T=emon
OBJS=main.o dump.o emon.o htcp.o smtp.o config.o oui.o rbtree.o
CC=gcc
FLAGS=-ggdb -DDEBUG -Wall 
.c.o:
	$(CC) $(FLAGS) -c $<
$T:$(OBJS)
	$(CC) -o $T $(OBJS) -lpthread
clean:
	rm -f $(OBJS) $(T)
tar:
#	(cd ..;tar cvfz $(T).tgz $(T)/*.[ch] $(T)/Makefile $(T)/oui.txt $(T)/$(T).conf;mv $(T).tgz $(T))
	(cd ..;tar cvfz $(T).tgz $(T)/*.[ch] $(T)/Makefile ;mv $(T).tgz $(T))
