obj-m += ramdisk_module.o

all:
	make -C /lib/modules/`uname -r`/build SUBDIRS=$(PWD) modules
	gcc -Wall our_tests.c rd.c -o our_tests
	gcc -Wall prof_tests.c rd.c -o prof_tests
	gcc -Wall real_tests.c rd.c -o real_tests

clean:
	make -C /lib/modules/`uname -r`/build SUBDIRS=$(PWD) clean
	rm *.o
	rm our_tests prof_tests real_tests