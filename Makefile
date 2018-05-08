obj-m += ramdisk_module.o

all:
	make -C /lib/modules/`uname -r`/build SUBDIRS=$(PWD) modules
	gcc -Wall test_file.c ramdisk.c -o test_file

clean:
	make -C /lib/modules/`uname -r`/build SUBDIRS=$(PWD) clean
	rm *.o
	rm test_file