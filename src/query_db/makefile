
CFLAGS=-g -c -Wall -I../util -I/usr/include/mysql -DLINUX
LFLAGS=-g -lmysqlclient

CCR= gcc $(CFLAGS) -o$@ $<
LINK=gcc $(LFLAGS) -o$@ $^

# goals
all: qdb_create dbclient

# link
qdb_create: obj/qdb_create.o obj/qdb.o obj/salloc.o obj/str.o obj/asprintf.o
	$(LINK)

dbclient: obj/dbclient.o obj/salloc.o
	$(LINK)

# compile
obj/%.o : %.c
	$(CCR)

obj/%.o : ../util/%.c
	$(CCR)


clean:
	-rm -f -r obj


# automatic include dependencies:
obj/%.d: %.c
	gcc -M $(CFLAGS) $< > $@.x
	sed 's,\($*\)\.o[ :]*,obj/\1.o $@ : ,g' < $@.x > $@
	rm -f $@.x

ifneq ($(MAKECMDGOALS),clean)
include $(patsubst %.c,obj/%.d,$(wildcard *.c))
endif
