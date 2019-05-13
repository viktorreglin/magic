
CFLAGS=-g -c -Wall -I../include -I/usr/include/mysql -DLINUX

CCR= gcc $(CFLAGS) -o$@ $<
LINK = gcc -g -o $@ $^ -lcurl -L../lib -lqdb -lutil $$(mysql_config --libs)

parse_json: parse_json.o cJSON.o ../lib/libutil.a ../lib/libqdb.a get_price.o
	$(LINK)

parse_json.o: parse_json.c
	$(CCR)

cJSON.o: ../lib/cJSON.c
	$(CCR)

get_price.o: ../get_price/get_price.c
	$(CCR) -lcurl

