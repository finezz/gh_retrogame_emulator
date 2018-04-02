#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "font.h"

#define DMA_BASE	0x13420000
#define DSA0 			(0x00 >> 2)
#define DTA0 			(0x04 >> 2)
#define DTC0 			(0x08 >> 2)
#define DRT0 			(0x0C >> 2)
#define DCS0 			(0x10 >> 2)
#define DCM0 			(0x14 >> 2)
#define DDA0 			(0x18 >> 2)
#define DSD0 			(0x1C >> 2)
#define DSA1 			(0x20 >> 2)
#define DTA1 			(0x24 >> 2)
#define DTC1 			(0x28 >> 2)
#define DRT1 			(0x2C >> 2)
#define DCS1 			(0x30 >> 2)
#define DCM1 			(0x34 >> 2)
#define DDA1 			(0x38 >> 2)
#define DSD1 			(0x3C >> 2)

#define LCD_BASE 	0x13050000
#define LCDDA0 		(0x40 >> 2)
#define LCDSA0 		(0x44 >> 2)
#define LCDFID0 	(0x48 >> 2)
#define LCDCMD0 	(0x4C >> 2)
#define LCDDA1		(0x50 >> 2)
#define LCDSA1		(0x54 >> 2)
#define LCDFID1 	(0x58 >> 2)
#define LCDCMD1 	(0x5C >> 2)

#define PAGE_SIZE	1024

int fd=-1;
volatile unsigned long *mem;

typedef struct {
	uint64_t pfn : 54;
	unsigned int soft_dirty : 1;
	unsigned int file_page : 1;
	unsigned int swapped : 1;
	unsigned int present : 1;
} PagemapEntry;

int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr)
{
	size_t nread;
	ssize_t ret;
	uint64_t data;

	nread = 0;
	while (nread < sizeof(data)) {
		ret = pread(pagemap_fd, &data, sizeof(data), (vaddr / sysconf(_SC_PAGE_SIZE)) * sizeof(data) + nread);
		nread += ret;
		if(ret <= 0){
			return 1;
		}
	}
	entry->pfn = data & (((uint64_t)1 << 54) - 1);
	entry->soft_dirty = (data >> 54) & 1;
	entry->file_page = (data >> 61) & 1;
	entry->swapped = (data >> 62) & 1;
	entry->present = (data >> 63) & 1;
	return 0;
}

int virt_to_phys_user(uintptr_t *paddr, pid_t pid, uintptr_t vaddr)
{
	char pagemap_file[BUFSIZ];
	int pagemap_fd;

	snprintf(pagemap_file, sizeof(pagemap_file), "/proc/%ju/pagemap", (uintmax_t)pid);
	pagemap_fd = open(pagemap_file, O_RDONLY);
	if (pagemap_fd < 0) {
		return 1;
	}
	PagemapEntry entry;
	if (pagemap_get_entry(&entry, pagemap_fd, vaddr)) {
		return 1;
	}
	close(pagemap_fd);
	*paddr = (entry.pfn * sysconf(_SC_PAGE_SIZE)) + (vaddr % sysconf(_SC_PAGE_SIZE));
	return 0;
}
int map_it(unsigned long addr, unsigned long size)
{
  int fd = open("/dev/mem", O_RDWR);
  if(fd < 0){
    printf("failed to open /dev/mem\n");
    return -1;
  }
  return mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr);
}

void unmap_it(unsigned long size)
{
	munmap(mem, size);
	close(fd);
}

int main(int argc, char* argv[])
{
	//uintptr_t paddr;
	//virt_to_phys_user(&paddr, getpid(), frame);
	//printf("paddr: 0x%x\n", paddr);
	uint32_t dma_addr;

	map_it(LCD_BASE, PAGE_SIZE);
	printf("LCDDA0: 0x%x\n", mem[LCDDA0]);
	printf("LCDSA0: 0x%x\n", mem[LCDSA0]);
	printf("LCDFID0: 0x%x\n", mem[LCDFID0]);
	printf("LCDCMD0: 0x%x\n", mem[LCDCMD0]);

	printf("LCDDA1: 0x%x\n", mem[LCDDA1]);
	printf("LCDSA1: 0x%x\n", mem[LCDSA1]);
	printf("LCDFID1: 0x%x\n", mem[LCDFID1]);
	printf("LCDCMD1: 0x%x\n", mem[LCDCMD1]);

	dma_addr = mem[LCDSA0];
	unmap_it(PAGE_SIZE);

	map_it(DMA_BASE, PAGE_SIZE);
	printf("DSA0: 0x%x\n", mem[DSA0]);
	printf("DTA0: 0x%x\n", mem[DTA0]);
	printf("DTC0: 0x%x\n", mem[DTC0]);
	printf("DRT0: 0x%x\n", mem[DRT0]);
	printf("DCS0: 0x%x\n", mem[DCS0]);
	printf("DCM0: 0x%x\n", mem[DCM0]);
	printf("DDA0: 0x%x\n", mem[DDA0]);
	printf("DSD0: 0x%x\n", mem[DSD0]);

	printf("DSA1: 0x%x\n", mem[DSA1]);
	printf("DTA1: 0x%x\n", mem[DTA1]);
	printf("DTC1: 0x%x\n", mem[DTC1]);
	printf("DRT1: 0x%x\n", mem[DRT1]);
	printf("DCS1: 0x%x\n", mem[DCS1]);
	printf("DCM1: 0x%x\n", mem[DCM1]);
	printf("DDA1: 0x%x\n", mem[DDA1]);
	printf("DSD1: 0x%x\n", mem[DSD1]);
	unmap_it(PAGE_SIZE);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);

	unsigned long c=0, idx=0;
	const uint32_t size=320*480*2;
	map_it(dma_addr, size);
	volatile uint16_t *ptr = (volatile uint16_t*)mem;
	uint16_t color[]={0xf800,0x7e0,0x1f};
	while(1){
		c = 0;
		for(int y=0; y<480; y++){
			for(int x=0; x<320; x++){
				ptr[c++] = color[idx];
			}
		}
		idx+= 1;
		if(idx >= 3){
			idx = 0;
		}
		SDL_Delay(100);
	}
	umap_it(size);
	SDL_Delay(5000);
	SDL_Quit();
  return 0;    
}

