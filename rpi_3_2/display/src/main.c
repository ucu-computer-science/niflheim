#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "../include/user_api.h"

#define to_color(rgb) (0xFF00 & (rgb << 8) | rgb >> 8)
#define PAGE_SIZE 4096
#define FULL_REFRESH "1:0" 

void rpi_draw_image(char *file, uint16_t *buf){
	int fd = open(file, O_RDWR);
	uint16_t buffer[128];
	size_t ind=0;
	size_t rb=0;

	if(fd < 0){
		perror("Failed to open image");
		exit(1);
	}

	while(1){
		rb = read(fd, buffer, 256);
		if(rb == 0 || (rb < 0 && errno != EINTR))
			break;

		for(int i=0; i < rb/2; i++){
			buf[ind++] = to_color(buffer[i]);
		}
	}
	close(fd);
}

int main(int argc, char* argv[]) {
	puts("=_+");
	int fd = open("/dev/display", O_RDWR);
	uint16_t *fb = NULL;

	if ( -1 == fd ) {
		puts("+_+");
		return -1;
	}

	size_t size = LCD_HEIGHT*LCD_WIDTH;

	printf("%lu, %lu\n", FB_PAGES, FB_PAGES*PAGE_SIZE);
	fb = mmap(0,  
			FB_PAGES*PAGE_SIZE,  
			PROT_READ | PROT_WRITE,  
			MAP_SHARED,  
			fd,  
			0);

	for (int ind=1; ind < argc; ++ind) {
		printf("image: %s\n", argv[ind]);
		rpi_draw_image(argv[ind], fb);
		write(fd, FULL_REFRESH, sizeof(FULL_REFRESH)-1);
		sleep(1);
	}

	munmap(fb, FB_PAGES*PAGE_SIZE);
	puts("+_=");
	return 0;
}
