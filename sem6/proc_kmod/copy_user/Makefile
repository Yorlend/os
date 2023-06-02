KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: clean

obj-m += md.o
md-objs = ./src/md.o

all: build_modules
	@rm -rf .*.ko.cmd .*.mod.o.cmd .*.o.cmd .*.o.d *.mod.c *.mod.o *.o .*.*.cmd *.mod *.symvers *.order
	@rm -rf ./**/.*.ko.cmd ./**/.*.mod.o.cmd ./**/.*.o.cmd ./**/.*.o.d ./**/*.mod.c ./**/*.mod.o ./**/*.o .*.*.cmd *.mod *.symvers *.order

build_modules:
	@make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	@make -C $(KERNEL_DIR) M=$(PWD) clean
