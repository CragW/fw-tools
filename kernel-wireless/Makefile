
obj-m := mywlan.o
mywlan-objs := cw_wlan.o cw_dsm_intel.o cw_regd.o

EXTRA_CFLAGS += -I/usr/src/linux-headers-$(shell uname -r) -Wall
PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build

default:
	make -C ${KDIR} M=$(PWD) modules

clean:
	make -C ${KDIR} M=$(PWD) clean
