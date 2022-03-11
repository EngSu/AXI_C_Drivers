#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/time.h>
// The purpose this test is to show that users can get to devices in user
// mode for simple things like GPIO. This is not to say this should replace
// a kernel driver, but does provide some short term solutions sometimes
// or a debug solution that can be helpful.

// This test maps a GPIO in the hardware into the user space such that a
// GPIO signal can be toggled fast. On the ML507 reference system, the 
// signal could be toggled about every 50 ns which is pretty fast.

// This test was derived from devmem2.c.

#define GPIO_BASE_ADDRESS0 	0x43C00000
#define GPIO_BASE_ADDRESS1 	0x4aa0000
#define GPIO_DATA_OFFSET0 	0
#define GPIO_DATA_OFFSET 	0
#define GPIO_DIRECTION_OFFSET 	4
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
int close(int fd); 

int main() 
{   


      
	int memfd;
	void *mapped_base0, *mapped_base1, *mapped_dev_base0, *mapped_dev_base1; 
	off_t dev_base0 = GPIO_BASE_ADDRESS0; 
	off_t dev_base1 = GPIO_BASE_ADDRESS1; 
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
    	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		exit(0);
	}
	//printf("/dev/mem opened.\n"); 
    
	// Map one page of memory into user space such that the device is in that page, but it may not
	// be at the start of the page

	
mapped_base0 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, dev_base0 & ~MAP_MASK);
    	if (mapped_base0 == (void *) -1) {
		printf("Can't map the memory to user space.\n");
		exit(0);
	}


mapped_base1 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, dev_base1 & ~MAP_MASK);
    	if (mapped_base1 == (void *) -1) {
		printf("Can't map the memory to user space.\n");
		exit(0);
	}


 	//printf("Memory mapped at address %p.\n", mapped_base); 
 
	// get the address of the device in user space which will be an offset from the base 
	// that was mapped as memory is mapped at the start of a page 
   
	mapped_dev_base0 = mapped_base0 + (dev_base0 & MAP_MASK);
	mapped_dev_base1 = mapped_base1 + (dev_base1 & MAP_MASK);

	// write to the direction register so all the GPIOs are on output to drive LEDs

	*((unsigned long *) (mapped_dev_base0 + GPIO_DIRECTION_OFFSET)) = 0;

              


		*((volatile unsigned long *) (mapped_dev_base0 + GPIO_DATA_OFFSET)) = 0x0;
                *((volatile unsigned long *) (mapped_dev_base1 + GPIO_DATA_OFFSET)+0) = 0x0;
                *((volatile unsigned long *) (mapped_dev_base1 + GPIO_DATA_OFFSET)+4) = 0x0;
                *((volatile unsigned long *) (mapped_dev_base1 + GPIO_DATA_OFFSET)+8) = 0x0;
                *((volatile unsigned long *) (mapped_dev_base1 + GPIO_DATA_OFFSET)+0x7f) = 0x0;
 
 printf("output : %d\n", *((volatile unsigned long *) (mapped_dev_base1 + GPIO_DATA_OFFSET))+0);
printf("output : %08x\n", *((volatile unsigned long *) (mapped_dev_base0 + GPIO_DATA_OFFSET))+0);
	

	// unmap the memory before exiting
 	
	if (munmap(mapped_base0, MAP_SIZE) == -1) {
		printf("Can't unmap memory from user space.\n");
		exit(0);
	}
if (munmap(mapped_base1, MAP_SIZE) == -1) {
		printf("Can't unmap memory from user space.\n");
		exit(0);
	}
	close(memfd);

	return 0;
}
