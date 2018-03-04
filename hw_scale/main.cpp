#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include "img.h"

extern uint8_t img[48684];

#define MEM_SIZE				1024
#define IPU_BASE 				0x13080000
#define LCD_BASE 				0x13050000

#define LCDCTRL 				(0x030 >> 2)
#define LCDIPUR					(0x11c >> 2)
#define LCDOSDC 				(0x100 >> 2)
#define LCDOSDCTRL 			(0x104 >> 2)
#define LCDSTATE 				(0x034 >> 2)
#define LCDDA0 					(0x040 >> 2)
#define LCDDAH 					(0x010 >> 2)
#define LCDDAV 					(0x014 >> 2)
#define LCDXYP0 				(0x120 >> 2)
#define LCDXYP1 				(0x124 >> 2)
#define LCDSIZE0 				(0x128 >> 2)
#define LCDSIZE1 				(0x12C >> 2)

#define IPU_CONTROL 		(0x00 >> 2)
#define IPU_STATUS 			(0x04 >> 2)
#define D_FMT 					(0x08 >> 2)
#define Y_ADDR 					(0x0C >> 2)
#define IN_FM_GS      	(0x18 >> 2)
#define Y_STRIDE      	(0x1C >> 2)
#define OUT_GS 					(0x28 >> 2)
#define OUT_STRIDE 			(0x2C >> 2)
#define OUT_ADDR 				(0x24 >> 2)
#define RSZ_COEF_INDEX 	(0x30 >> 2)
#define HRSZ_COEF_LUT 	(0x48 >> 2)
#define VRSZ_COEF_LUT 	(0x4C >> 2)

void set_upscale_bilinear_coefs(volatile uint32_t *mem, unsigned int reg, unsigned int num, unsigned int denom)
{
  uint32_t i, weight_num = 0;

  for(i=0; i<num; i++){
    unsigned int weight = 512 - 512 * weight_num / num;
    uint32_t offset = 0, value;

    weight_num+= denom;
    if(weight_num >= num){
      weight_num-= num;
      offset = 1;
    }

    value = (weight & 0x7FF) << 6 | (offset << 1);
		mem[reg] = value;
  }
}

int hw_scale(void *src, uint32_t len)
{
	int fd = open("/dev/mem", O_RDWR);
  if(fd < 0){
    printf("failed to open /dev/mem\n");
    return -1;
  }
  volatile uint32_t *lcd_mem  = (volatile uint32_t*)mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, LCD_BASE);
	if(!lcd_mem){
		printf("failed mmap lcd memory\n");
		return -1;
	}
	volatile uint32_t *ipu_mem  = (volatile uint32_t*)mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, IPU_BASE);
	if(!ipu_mem){
		printf("failed mmap ipu memory\n");
		return -1;
	}
	void *in_mem  = mmap(0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(!in_mem){
		printf("failed mmap in memory\n");
		return -1;
	}
	void *out_mem  = mmap(0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(!out_mem){
		printf("failed mmap out memory\n");
		return -1;
	}
	memcpy(in_mem, src, len);
	
	uint16_t sdl_w = 320;
	uint16_t sdl_h = 240;
	uint16_t scale_w = 320;
	uint16_t scale_h = 480;
	printf("lcd state: 0x%x\n", lcd_mem[LCDSTATE]);
	printf("lcd dah: 0x%x\n", lcd_mem[LCDDAH]); // 0x3c017c
	printf("lcd dav: 0x%x\n", lcd_mem[LCDDAV]); // 0x1c01fc
	printf("lcd size0: 0x%x\n", lcd_mem[LCDSIZE0]); // 0x1e00140
	printf("lcd size1: 0x%x\n", lcd_mem[LCDSIZE1]); // 0x1e00140
	printf("lcd ipur: 0x%x\n", lcd_mem[LCDIPUR]);
	
	//lcd_mem[LCDCTRL]|= 0x10; // disable lcd
	ipu_mem[IPU_CONTROL] = 0; // stop ipu
	//lcd_mem[LCDOSDC] = 0x09; // f0 en, enable osd
	//lcd_mem[LCDOSDCTRL] = 0x8004; // ipu, 16 bpp

	ipu_mem[IPU_CONTROL] = 0x41; // enable ipu chip, reset ipu
	ipu_mem[D_FMT] = 0x00080003; // 16bit bpp
	ipu_mem[IN_FM_GS] = ((sdl_w << 1) << 16) | sdl_h;
	ipu_mem[Y_STRIDE] = sdl_w << 1;
	ipu_mem[Y_ADDR] = (uint32_t)in_mem;
	ipu_mem[IPU_CONTROL] = 0x01 | 0x20; // ipu chip enable, fm irq
  set_upscale_bilinear_coefs(ipu_mem, VRSZ_COEF_LUT, sdl_w, scale_w);
  set_upscale_bilinear_coefs(ipu_mem, HRSZ_COEF_LUT, sdl_h, scale_h);
	ipu_mem[IPU_CONTROL]|= 0x0c; // hrsz, vrsz
	ipu_mem[RSZ_COEF_INDEX]|= ((scale_w - 1) << 16) | (scale_h - 1);
	ipu_mem[OUT_GS] = ((scale_w << 1) << 16) | scale_h;
	ipu_mem[OUT_STRIDE] = scale_w << 2;
	ipu_mem[OUT_ADDR] = (uint32_t)out_mem;
	ipu_mem[IPU_STATUS] = 0x01; // clear ipu status
	ipu_mem[IPU_CONTROL]|= 0x02; // run ipu
	//lcd_mem[LCDCTRL]&= ~0x00000010; // enable lcd

	memcpy(src, out_mem, len);
	munmap((void*)lcd_mem, MEM_SIZE);
	munmap((void*)ipu_mem, MEM_SIZE);
	munmap((void*)in_mem, len);
	munmap((void*)out_mem, len);
	close(fd);
	return 0;
}
 
int main(int argc, char* args[])
{
  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    printf("%s, failed to SDL_Init\n", __func__);
    return -1;
  }
  SDL_ShowCursor(0);
 
  SDL_Surface* screen;
  screen = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
  if(screen == NULL){
    printf("%s, failed to SDL_SetVideMode\n", __func__);
    return -1;
  }
 
 	SDL_RWops *rw = SDL_RWFromMem(img, sizeof(img));
  SDL_Surface* png = IMG_Load_RW(rw, 1);
  if(png == NULL){
    printf("%s, failed to IMG_Load\n", __func__);
    return -1;
  }

  SDL_BlitSurface(png, NULL, screen, NULL);
	if(SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
	hw_scale(screen->pixels, 320*480*2);
	if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  SDL_Flip(screen);
  SDL_Delay(3000);
  SDL_FreeSurface(png);
  SDL_Quit();
  return 0;    
}

