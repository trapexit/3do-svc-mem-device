OUTPUT = svc_mem

DEBUG	= 1

STACKSIZE 	= 8192

CC		= armcc
AS 		= armasm
LD		= armlink
LIB             = armlib
RM		= rm
MODBIN          = modbin
MAKEBANNER	= MakeBanner

CFLAGS	= -bigend -za1 -zps0 -zi4 -fa -fh -fx -fpu none -arch 3 -apcs '3/32/fp/swst/wide/softfp'
ASFLAGS = -bigend -fpu none -arch 3 -apcs '3/32/fp/swst'
LDFLAGS = -aif -reloc -ro-base 0x00 -dupok -remove -nodebug -verbose
INCPATH	= -I${TDO_DEVKIT_PATH}/include/3do -I${TDO_DEVKIT_PATH}/include/community
LIBPATH	= ${TDO_DEVKIT_PATH}/lib

LIBS 	= $(LIBPATH)/3do/clib.lib \
	  $(LIBPATH)/community/svc_funcs.lib \
	  $(LIBPATH)/3do/cstartup.o

SRC_S = $(wildcard src/*.s)
SRC_C = $(wildcard src/*.c)

all: builddir svc_mem_drv.signed svc_mem.lib

build/svc_mem_drv.c.o: src/svc_mem_drv.c src/svc_mem_drv.h
	$(CC) $(INCPATH) $(CFLAGS) -c $< -o $@

build/svc_mem_dev.c.o: src/svc_mem_dev.c
	$(CC) $(INCPATH) $(CFLAGS) -c $< -o $@

build/main.c.o: src/main.c
	$(CC) $(INCPATH) $(CFLAGS) -c $< -o $@

svc_mem_drv.unsigned: build/svc_mem_dev.c.o build/svc_mem_drv.c.o build/main.c.o
	$(LD) $(LDFLAGS) $? $(LIBS) -o build/$@

svc_mem_drv.signed: svc_mem_drv.unsigned
	$(MODBIN) --stack=$(STACKSIZE) --flags=0x2 --sign=3do --name=svc_mem build/$< build/$@

build/svc_mem.c.o: src/svc_mem.c
	$(CC) $(INCPATH) $(CFLAGS) -c $< -o $@

svc_mem.lib: build/svc_mem.c.o
	$(LIB) -c build/$@ $<

clean:
	$(RM) -rfv build/

builddir:
	mkdir -p build

install: all
	cp -fv build/svc_mem_drv.signed ${TDO_DEVKIT_PATH}/takeme/System/Drivers/svc_mem_drv
	cp -fv build/svc_mem.lib ${TDO_DEVKIT_PATH}/lib/community/svc_mem.lib
	cp -fv src/svc_mem.h ${TDO_DEVKIT_PATH}/include/community/svc_mem.h

.PHONY: builddir install
