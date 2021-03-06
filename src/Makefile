SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
DFLAGS =
# Warnings (GCC)
WFLAGS = -W -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wsign-compare -Wmissing-noreturn -Wmissing-format-attribute -Wpacked -Wredundant-decls
# Warnings (G++)
WPPFLAGS = -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Woverloaded-virtual -Wsign-promo -Wsynth
OFLAGS = -O2 -Wall
LFLAGS = -L. -lz
CFLAGS =

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif

CFLAGS += $(OFLAGS) $(DFLAGS) $(WFLAGS) $(WPPFLAGS) -I.

OBJS = atom.o base64.o bencode.o client.o config.o md5.o server.o sha1.o sort.o tracker_admin.o tracker_comments.o tracker_file.o tracker_index.o tracker_info.o tracker_login.o tracker_signup.o tracker_stats.o tracker_torrent.o tracker_upload.o tracker_users.o util.o
OBJS_BNBT = bnbt.o tracker.o tracker_announce.o tracker_scrape.o
OBJS_BNBTMYSQL = bnbt.mysql.o bnbt_mysql.mysql.o tracker.mysql.o tracker_announce.mysql.o tracker_scrape.mysql.o

PROGS = ./bnbt ./bnbtmysql

all: $(OBJS) $(OBJS_BNBT) $(OBJS_BNBTMYSQL) $(PROGS)

./bnbt: $(OBJS) $(OBJS_BNBT)
	$(C++) -o ./bnbt $(OBJS) $(OBJS_BNBT) $(LFLAGS)

./bnbtmysql: $(OBJS) $(OBJS_BNBTMYSQL)
	$(C++) -o ./bnbtmysql $(OBJS) $(OBJS_BNBTMYSQL) $(LFLAGS) -L/usr/lib/mysql -lmysqlclient

clean:
	rm -f $(OBJS) $(OBJS_BNBT) $(OBJS_BNBTMYSQL) $(PROGS)

%.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

%.mysql.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -DBNBT_MYSQL -c $<

./bnbt: $(OBJS) $(OBJS_BNBT)

./bnbtmysql: $(OBJS) $(OBJS_BNBTMYSQL)

all: $(PROGS)

atom.o: bnbt.h atom.h util.h
base64.o: base64.h
bencode.o: bnbt.h atom.h bencode.h util.h
bnbt.o: bnbt.h config.h server.h util.h
client.o: bnbt.h atom.h base64.h client.h md5.h server.h tracker.h util.h
config.o: bnbt.h atom.h config.h tracker.h util.h
md5.o: md5.h
server.o: bnbt.h client.h config.h server.h tracker.h util.h
sha1.o: sha1.h
sort.o: bnbt.h sort.h tracker.h util.h
tracker.o: bnbt.h atom.h bencode.h config.h md5.h server.h sort.h tracker.h util.h
tracker_admin.o: bnbt.h atom.h tracker.h server.h util.h
tracker_announce.o: bnbt.h atom.h bencode.h tracker.h util.h
tracker_comments.o: bnbt.h atom.h tracker.h util.h
tracker_file.o: bnbt.h atom.h tracker.h util.h
tracker_index.o: bnbt.h atom.h sort.h tracker.h util.h
tracker_info.o: bnbt.h atom.h tracker.h bencode.h util.h
tracker_login.o: bnbt.h atom.h tracker.h util.h
tracker_scrape.o: bnbt.h atom.h bencode.h tracker.h util.h
tracker_signup.o: bnbt.h atom.h tracker.h util.h
tracker_stats.o: bnbt.h atom.h bencode.h sort.h tracker.h util.h
tracker_torrent.o: bnbt.h atom.h bencode.h tracker.h util.h
tracker_upload.o: bnbt.h atom.h bencode.h client.h tracker.h util.h
tracker_users.o: bnbt.h atom.h md5.h sort.h tracker.h util.h
util.o: bnbt.h atom.h bencode.h sha1.h tracker.h util.h

bnbt.mysql.o: bnbt.h bnbt_mysql.h config.h server.h util.h
bnbt_mysql.mysql.o: bnbt.h bnbt_mysql.h util.h
tracker.mysql.o: bnbt.h bnbt_mysql.h atom.h bencode.h config.h md5.h server.h sort.h tracker.h util.h
tracker_announce.mysql.o: bnbt.h bnbt_mysql.h atom.h bencode.h tracker.h util.h
tracker_scrape.mysql.o: bnbt.h bnbt_mysql.h atom.h bencode.h tracker.h util.h
