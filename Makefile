# Example makefile for CPE 464
#

CC = gcc
CFLAGS = -g -Wall -Werror
OS = $(shell uname -s)
PROC = $(shell uname -p)
EXEC_SUFFIX=$(OS)-$(PROC)

ifeq ("$(OS)", "SunOS")
	OSLIB=-L/opt/csw/lib -R/opt/csw/lib -lsocket -lnsl
	OSINC=-I/opt/csw/include
	OSDEF=-DSOLARIS
else
ifeq ("$(OS)", "Darwin")
	OSLIB=
	OSINC=
	OSDEF=-DDARWIN
else
	OSLIB=
	OSINC=
	OSDEF=-DLINUX
endif
endif

all:  fishnode-$(EXEC_SUFFIX)

test: fishtest-$(EXEC_SUFFIX)

fishnode-$(EXEC_SUFFIX): fishnode.c
	$(CC) $(CFLAGS) $(OSINC) $(OSLIB) $(OSDEF) fishnode.c smartalloc.c -o $@ -L. -lfish-Linux-x86_64

tarball:
	tar -czvf transfer.tgz README fish.h libfish-Linux-x86_64.a smartalloc.c smartalloc.h fishnode.c fishnode.h fishtest.c fishtest.h Makefile

handin: README
	handin bellardo 464_p3 README fish.h libfish-Linux-x86_64.a smartalloc.c smartalloc.h fishnode.c fishnode.h Makefile

fishtest-$(EXEC_SUFFIX): fishtest.c
	$(CC) $(CFLAGS) $(OSINC) $(OSLIB) $(OSDEF) fishtest.c smartalloc.c -o $@ -L. -lfish-Linux-x86_64

clean:
	rm -rf fishnode-* fishnode-*.dSYM
