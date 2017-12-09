
CFLAGS=-g -c -Wall -I ../include -DLINUX

CCR= gcc $(CFLAGS) -o $@ $<
LINK=gcc -g -o $@ $< -L../lib -lutil
LIB= ar -cr $@ $^

# goals
all: ../../bin/formjson ../lib/libutil.a test_config


# build library
../lib/libutil.a: obj/asprintf.o obj/config.o obj/filelength.o obj/readfile.o obj/salloc.o obj/str.o
	$(LIB)

# link
../../bin/formjson: obj/formjson.o
	$(LINK)

test_config: obj/test_config.o ../lib/libutil.a
	$(LINK)

# compile
obj/test_config.o: config.c
	gcc $(CFLAGS) -DTEST_CONFIG -o$@ $<

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
