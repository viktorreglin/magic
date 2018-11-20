
CFLAGS=-g -c -Wall -I../include -I/usr/include/mysql -DLINUX

CCR= gcc $(CFLAGS) -o$@ $<
LINK = gcc -g -o $@ $^ -L../lib -lqdb -lutil $$(mysql_config --libs)

parse_json: parse_json.o cJSON.o ../lib/libutil.a ../lib/libqdb.a
	$(LINK)

parse_json.o: parse_json.c
	$(CCR)

cJSON.o: ../lib/cJSON.c
	$(CCR)

