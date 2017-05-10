
CFLAGS=-g -c -Wall -I../util -I/usr/include/mysql
LFLAGS=-g -lmysqlclient

CCR= gcc $(CFLAGS) -o$@ $<
LINK=gcc $(LFLAGS)

# goals
all: dbclient

# link
dbclient: obj/dbclient.o obj/salloc.o
	$(LINK) -o$@ $^

# compile
obj/%.o : %.c
	$(CCR)

obj/%.o : ../util/%.c
	$(CCR)


clean:
	-rm -f -r obj


# automatic dependency generation
# can use wildcards for all existing C files
# add generated files explicitely
#ifneq ($(MAKECMDGOALS),clean)
#$(OBJDIR)/depend.mk: src/*.c  src/xbnfparse.c
#	touch $@
#	makedepend -f$@ -p$(OBJDIR)/ -Y $(CFLAGS) $^

#-include $(OBJDIR)/depend.mk
#endif
# # #

