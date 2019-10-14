
CFLAGS=-g -c -Wall -I../include -I/usr/include/mysql -DLINUX

CCR= gcc $(CFLAGS) -o$@ $<
LINK=gcc -no-pie -g -o$@ $< -L../lib -lqdb -lutil $$(mysql_config --libs)
LIB= ar -cr $@ $^


# goals
all: ../../bin/qdb_create ../../bin/qdb_dump ../../bin/qdb_insert ../../bin/qdb_query


# build library
../lib/libqdb.a: obj/qdb.o
	$(LIB)

# link
../../bin/qdb_create: obj/qdb_create.o ../lib/libutil.a ../lib/libqdb.a
	$(LINK)

../../bin/qdb_dump: obj/qdb_dump.o ../lib/libutil.a ../lib/libqdb.a
	$(LINK)

../../bin/qdb_insert: obj/qdb_insert.o ../lib/libutil.a ../lib/libqdb.a
	$(LINK)

../../bin/qdb_query: obj/qdb_query.o ../lib/libutil.a ../lib/libqdb.a
	$(LINK)

# compile
obj/%.o : %.c
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
