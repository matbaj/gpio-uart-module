obj-m += gpio-uart.o 
ksrc   = "/lib/modules/`uname -r`/build/"
sysr   = "/"
mdir   = "kernel/drivers/gpio"

all:
	make -C $(ksrc) M=$(PWD) modules

module_install:
	make -C $(ksrc) M=$(PWD) INSTALL_MOD_PATH=$(sysr) INSTALL_MOD_DIR=$(mdir) modules_install

clean:
	make -C $(ksrc) M=$(PWD) clean
