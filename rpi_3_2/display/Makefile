TARGET = rpi_3_2_driver
obj-m := $(TARGET).o
$(TARGET)-objs := src/lcd_init.o src/fonts.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

tree:
	dtc overlay/spidev_off.dts -O dtb >spidev_off.dtbo
	sudo dtoverlay -d . spidev_off

rm:
	sudo rmmod $(TARGET)


app:                                                            
	@sudo dmesg -C                                          
	@$(CC) src/main.c -o app                                
	@-sudo ./app img/dog.raw img/romashki.raw img/autumn.raw img/fractal.raw img/sea.raw img/meme.raw 	
	@rm app                                                 
	@dmesg                                                  

ins:
	sudo insmod $(TARGET).ko

test:
	@sudo dmesg -C
	@sudo insmod  $(TARGET).ko
	@sudo ./test.sh
	@sleep 2
	@sudo rmmod $(TARGET)
	@dmesg
