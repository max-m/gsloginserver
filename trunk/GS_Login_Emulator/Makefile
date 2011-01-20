# Ubuntu, required library packages: 
#   libmysqlclient-dev sqlite3-dev libc6-dev 

SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
LDFLAGS = -L. -lpthread -ldl -lz -lStorm -lmysqlclient_r
CFLAGS =

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
else
LFLAGS += -lrt
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I.

ifeq ($(SYSTEM),Darwin)
CFLAGS += -I../mysql/include/
endif

OBJS = bf2.available.o client.o config.o database.o gpcm.o gpsp.o gs_server.o md5.o socket.o
COBJS = sqlite3.o
PROGS = ./gsloginemulator

all: $(OBJS) $(COBJS) $(PROGS)

./gsloginemulator: $(OBJS) $(COBJS)
	$(C++) -o ./gsloginemulator $(OBJS) $(COBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

./gsloginemulator: $(OBJS) $(COBJS)

all: $(PROGS)

bf2.available.o: bf2.available.h gs_server.h socket.h
client.o: client.h gpcm.h gpsp.h gs_server.h socket.h database.h md5.h
config.o: config.h
database.o: sqlite3.h database.h
gpcm.o: gpcm.h socket.h client.h gs_server.h
gpsp.o: gpsp.h gpcm.h socket.h client.h gs_server.h
gs_server.o: gs_server.h log.h md5.h database.h client.h config.h gpcm.h gpsp.h bf2.available.h
md5.o: md5.h
socket.o: socket.h