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

all:  json-server-$(EXEC_SUFFIX)

json-server-$(EXEC_SUFFIX): server.c
	$(CC) $(CFLAGS) $(OSINC) $(OSLIB) $(OSDEF) server.c smartalloc.c -o $@

handin: README
	handin bellardo 464_p3 README server.c server.h smartalloc.c smartalloc.h Makefile

clean:
	rm -rf json-server-* json-server-*.dSYM
