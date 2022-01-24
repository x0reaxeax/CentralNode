OBJS		= main.o base/cnode.o base/help.o data/data.o gui/cursesctl.o gui/menu.o net/curlcomm.o net/ifs.o net/netops.o sys/crypto/uuid.o sys/crypto/md5.o sys/daemon/tincdctl.o sys/config.o sys/logging.o sys/sysctl.o sys/nodeWD/wdctl.o
SOURCE		= main.c base/cnode.c base/help.o data/data.c gui/cursesctl.c gui/menu.c net/curlcomm.c net/ifs.c net/netops.c sys/crypto/uuid.c sys/crypto/md5.c sys/daemon/tincdctl.c sys/config.c sys/logging.c sys/sysctl.c sys/nodeWD/wdctl.c
HEADER		= base/cnode.h base/help.h data/data.h gui/gui.h gui/cursesctl.h gui/menu.h net/curlcomm.h net/ifs.h net/netops.h net/networks.h sys/crypto/uuid.h sys/crypto/md5.h sys/daemon/tincdctl.h sys/config.h sys/logging.h sys/sysctl.h sys/nodeWD/wdctl.h
OUT			= node71
CC			= gcc
NASM		= nasm
DBGHOOKOBJ	= sys/debughook.o
DBGHOOKSRC	= sys/getsp64.asm
KMODULEOBJ	= sys/kmodule.o
KMODULESRC	= sys/kmodule.asm
NASMTARGET	= elf64
LDLIBS		= -lncurses -lmenu -lcurl -luuid -lssl -lcrypto
LFLAGS		= -O2 -pedantic
DEBUG		= -g
DEBUGINFO	= -Wall -Wextra -Wno-ignored-qualifiers
ASAN		= -fsanitize=address
NASMOBJS	= $(DBGHOOKOBJ) $(KMODULEOBJ)
all: debughook kmodulectl node71

debughook:
	$(NASM) $(DEBUG) -f $(NASMTARGET) $(DBGHOOKSRC) -o $(DBGHOOKOBJ)

kmodulectl:
	$(NASM) $(DEBUG) -f $(NASMTARGET) $(KMODULESRC) -o $(KMODULEOBJ)

node71: $(OBJS)
	$(CC) $(LFLAGS) $(ASAN) $(DEBUGINFO) $(DEBUG) -o $@ $^ $(NASMOBJS) $() $(LDLIBS)

%.o: %.c $(HEADER)
	$(CC) $(LFLAGS) $(ASAN) $(DEBUGINFO) $(DEBUG) -c -o $@ $<

clean:
	rm -f $(OBJS) $(NASMOBJS) $(KMODULEOBJ) $(OUT)
