obj-m := driver.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement 

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
