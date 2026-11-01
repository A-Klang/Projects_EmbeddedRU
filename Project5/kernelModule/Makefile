obj-m += kernelInterrupt.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	dtc -@ -I dts -O dtb -o kernelInterrupt.dtbo kernelInterrupt-overlay.dts

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f kernelInterrupt.dtbo
