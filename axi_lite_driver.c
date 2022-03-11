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

#define GPIO_BASE_ADDRESS 	0x43C00000
#define GPIO_DATA_OFFSET 	0
#define GPIO_DIRECTION_OFFSET 	4

#define NO_LABELS	2115
 
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
int close(int fd); 
 
int main() 
{   

        FILE *file, *timefile;

        int myvar;

        double secs = 0;

        double x[784];

	int i,j;
	int ii =0;
	int jj =0;
	int x_temp[2115][784];
        struct timeval stop, start;
	int memfd;
	void *mapped_base, *mapped_dev_base; 
	off_t dev_base = GPIO_BASE_ADDRESS; 
	
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
    	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		exit(0);
	}
	//printf("/dev/mem opened.\n"); 
    
	// Map one page of memory into user space such that the device is in that page, but it may not
	// be at the start of the page

	mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, dev_base & ~MAP_MASK);
    	if (mapped_base == (void *) -1) {
		printf("Can't map the memory to user space.\n");
		exit(0);
	}
 	//printf("Memory mapped at address %p.\n", mapped_base); 
 
	// get the address of the device in user space which will be an offset from the base 
	// that was mapped as memory is mapped at the start of a page 
   
	mapped_dev_base = mapped_base + (dev_base & MAP_MASK);


	*((volatile unsigned long *) (mapped_dev_base + GPIO_DIRECTION_OFFSET)) = 0;

	// toggle the output as fast as possible just to see how fast it works

              

  // send in_valid signal 0
	//	*((volatile unsigned long *) (mapped_dev_base + GPIO_DATA_OFFSET)) = 0;

              // 784 inputs of 8 bits
//==================== reading the inputs of the hidden layar =====================//   
 
    file = fopen("inputs_FP.txt", "r");
    if(file == NULL) {
        perror("Error opening file");
        return(-1);
    }

	for(i = 0; i < 2115; i++)
   	 {
        for (j = 0 ; j < 784; j++)
        {
            fscanf(file,"%d",&myvar);
            x_temp[i][j] = myvar ;
//printf("%d:	%d \n",i, x_temp[i][j]);
        }
     // printf("\n\n");
    }
//==================================================================================// 


        timefile=fopen("time_file.txt", "w");
        jj =0;
	

	for(jj=0;jj<2115;jj++) {
// send in_valid signal 0
		*((volatile unsigned long *) (mapped_dev_base + GPIO_DATA_OFFSET)) = 0;
          i= 0;
       // gettimeofday(&start, NULL); 
	for(ii = 0; ii < 196; ii=ii+1){  
		*((unsigned long int *) (mapped_dev_base + GPIO_DATA_OFFSET + i + 16)) = (x_temp[jj][i+3]<<24)|(x_temp[jj][i+2]<<16)|(x_temp[jj][i+1]<<8)|(x_temp[jj][i]);
               
i =i+4;

}
     //  gettimeofday(&stop, NULL);
        
// send in_valid signal 1
        gettimeofday(&start, NULL); 
      *((volatile unsigned long *) (mapped_dev_base + GPIO_DATA_OFFSET)) = 1;

    // read outputs 
   while ( *((volatile unsigned long *) (mapped_dev_base + GPIO_DATA_OFFSET +12)) == 0 ){}

       gettimeofday(&stop, NULL);

secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
 fprintf(timefile, "%d	%f\n",(jj+1), secs); 
  // printf("output ready  :%d \n\r", *((unsigned long *) (mapped_dev_base + GPIO_DATA_OFFSET+ 12)));
            

}

	// unmap the memory before exiting
 	
	if (munmap(mapped_base, MAP_SIZE) == -1) {
		printf("Can't unmap memory from user space.\n");
		exit(0);
	}
	
	close(memfd);
        fclose(file);
	fclose(timefile);
	return 0;
}
