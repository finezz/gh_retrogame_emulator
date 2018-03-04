#include "ipu.h"

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <SDL.h>
#include <SDL_image.h>
#include "img.h"

extern uint8_t img[48684];

struct ipu {
	void *base;
	unsigned long fb;
	int dev_mem_fd;
	unsigned int src_stride;
	unsigned int dst_stride;
};

struct fraction {
	unsigned int num, denom;
};

static struct ipu *ipu;

static int draw_image(void)
{
  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    printf("%s, failed to SDL_Init\n", __func__);
    return -1;
  }
  SDL_ShowCursor(0);
 
  SDL_Surface* screen;
  screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE);
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
  SDL_Flip(screen);
  SDL_Delay(3000);
  SDL_FreeSurface(png);
  SDL_Quit();
	return 0;
}

static inline uint32_t read_reg(struct ipu *ipu, unsigned int reg)
{
	uint32_t result = *(volatile uint32_t *) (ipu->base + reg);
	printf("%-19s contains       %08" PRIX32 "\n", reg_names[reg / sizeof(uint32_t)], result);
	return result;
}

static inline void write_reg(struct ipu *ipu, unsigned int reg, uint32_t value)
{
	printf("%-19s being written  %08" PRIX32 "\n", reg_names[reg / sizeof(uint32_t)], value);
	if (reg == REG_HRSZ_COEF_LUT || reg == REG_VRSZ_COEF_LUT) {
		if (value & 1)
			printf("%-19s                starting configuration\n", "");
		else
			printf("%-19s                coefficient %3u, offset +%2u\n", "", (value >> 6) & 0x7FF, (value >> 1) & 0x1F);
	}
	*(volatile uint32_t *) (ipu->base + reg) = value;
}

static inline void set_bit(struct ipu *ipu, unsigned int reg, uint32_t mask)
{
	write_reg(ipu, reg, read_reg(ipu, reg) | mask);
}

static inline void clr_bit(struct ipu *ipu, unsigned int reg, uint32_t mask)
{
	write_reg(ipu, reg, read_reg(ipu, reg) & ~mask);
}

static void print_regs(struct ipu *ipu)
{
	unsigned int i;
	for (i = 0; i < sizeof(reg_names) / sizeof(reg_names[0]); i++) {
		read_reg(ipu, i * 4);
	}
}

static void ipu_wait_completion(struct ipu *ipu)
{
	while (!(read_reg(ipu, REG_STATUS) & IPU_STATUS_OUT_END))
		usleep(4000);
}

static void ipu_stop(struct ipu *ipu, int force)
{
	if (!force) {
		uint32_t ctrl = read_reg(ipu, REG_CTRL);
		if (ctrl & IPU_CTRL_CHIP_EN) {
			set_bit(ipu, REG_CTRL, IPU_CTRL_STOP);
			ipu_wait_completion(ipu);
		}
	}

	write_reg(ipu, REG_CTRL, 0);
	write_reg(ipu, REG_STATUS, 0);
}

static unsigned int get_gcd(unsigned int a, unsigned int b)
{
	unsigned int c;
	while (a > 0) {
		c = a;
		a = b % a;
		b = c;
	}
	return b;
}

static void reduce_fraction(struct fraction *f)
{
	unsigned int gcd = get_gcd(f->num, f->denom);

	f->num /= gcd;
	f->denom /= gcd;
}

static unsigned int calc_size(unsigned int src, const struct fraction *frac)
{
	return src * frac->num / frac->denom;
}

static void ipu_set_upscale_bilinear_coef(struct ipu *ipu,
			const struct fraction *frac, unsigned int reg)
{
	unsigned int add = frac->denom, i;
	struct fraction weight_frac = { .num = 0, .denom = frac->num };

	usleep(20000); /* a 20ms sleep seems necessary */
	write_reg(ipu, reg, 0x1);

	for (i = 0; i < frac->num; i++) {
		unsigned int weight = 512 - 512 * weight_frac.num / weight_frac.denom;
		unsigned int offset = 0;
		weight_frac.num += add;

		if (weight_frac.num >= weight_frac.denom) {
			offset = 1;
			weight_frac.num -= weight_frac.denom;
		}

		uint32_t value = ((weight & 0x7FF) << 6) | (offset << 1);

		usleep(20000); /* a 20ms sleep seems necessary */
		write_reg(ipu, reg, value);
	}
}

/*
 * Sets up the IPU to downscale an image with bilinear resampling.
 * If the scaling fraction >= 1/2, each input pixel is used at least once.
 * If the scaling fraction < 1/2, not all input pixels are used, and it is
 * possible that the pixels read from each group of input pixels are more
 * to the left than expected. This is due to the scaling coefficients not
 * supporting specifying an offset for the first pixel, only those after it.
 *
 * The input pixels to use for each output pixel are calculated from the
 * middle of each output pixel.
 *
 * IN:
 *   ipu: Pointer to IPU registers.
 *   frac: Pointer to downscaling fraction. src * frac->num / frac->denom
 *     == dst.
 *   reg: The register number to write. This is either REG_HRSZ_COEF_LUT or
 *     REG_VRSZ_COEF_LUT.
 */
static void ipu_set_downscale_bilinear_coef(struct ipu *ipu,
			const struct fraction *frac, unsigned int reg)
{
	unsigned int add = frac->denom * 2, i;
	struct fraction weight_frac = { .num = frac->denom, .denom = frac->num * 2 };

	usleep(20000); /* a 20ms sleep seems necessary */
	write_reg(ipu, reg, 0x1);

	for (i = 0; i < frac->num; i++) {
		weight_frac.num = weight_frac.denom / 2
					+ (weight_frac.num - weight_frac.denom / 2) % weight_frac.denom;

		/*
		 * Here, "input pixel 1.0" means half of 0 and half of 1;
		 * "input pixel 0.5" means all of 0; and
		 * "input pixel 1.49" means almost all of 1.
		 */
		unsigned int weight = 512
					- (512 * (weight_frac.num - weight_frac.denom / 2)
					   / weight_frac.denom);
		weight_frac.num += add;

		unsigned int offset = (weight_frac.num - weight_frac.denom / 2)
				/ weight_frac.denom;

		uint32_t value = ((weight & 0x7FF) << 6) | (offset << 1);

		usleep(20000); /* a 20ms sleep seems necessary */
		write_reg(ipu, reg, value);
	}
}

static void ipu_set_bilinear_resize_coef(struct ipu *ipu,
			const struct fraction *frac, unsigned int reg)
{
	if (frac->num >= frac->denom) {
		ipu_set_upscale_bilinear_coef(ipu, frac, reg);
	} else {
		ipu_set_downscale_bilinear_coef(ipu, frac, reg);
	}
}

static void ipu_set_nearest_resize_coef(struct ipu *ipu,
			const struct fraction *frac, unsigned int reg)
{
	unsigned int add = frac->denom, i;
	struct fraction weight_frac = { .num = 0, .denom = frac->num };

	usleep(20000); /* a 20ms sleep seems necessary */
	write_reg(ipu, reg, 0x1);

	for (i = 0; i < frac->num; i++) {
		const unsigned int weight = 512;
		unsigned int offset = 0;
		weight_frac.num += add;

		offset = weight_frac.num / weight_frac.denom;
		weight_frac.num %= weight_frac.denom;

		uint32_t value = (weight << 6) | (offset << 1);

		usleep(20000); /* a 20ms sleep seems necessary */
		write_reg(ipu, reg, value);
	}
}

static void ipu_set_resize_params(struct ipu *ipu, enum ipu_resize_algorithm algorithm,
			unsigned int srcW, unsigned int srcH,
			unsigned int dstW, unsigned int dstH,
			unsigned int bytes_per_pixel)
{
	struct fraction fracW = { .num = dstW, .denom = srcW },
	                fracH = { .num = dstH, .denom = srcH };
	uint32_t coef_index = 0;

	reduce_fraction(&fracW);
	reduce_fraction(&fracH);
	printf("Width  resizing fraction: %2u/%2u\n", fracW.num, fracW.denom);
	printf("Height resizing fraction: %2u/%2u\n", fracH.num, fracH.denom);

	/* Calculate valid W/H parameters */
	dstW = calc_size(srcW, &fracW);
	dstH = calc_size(srcH, &fracH);
	printf("New output size: %ux%u\n", dstW, dstH);

	set_bit(ipu, REG_CTRL,
		(srcW != dstW ? IPU_CTRL_HRSZ_EN : 0) |
		(srcH != dstH ? IPU_CTRL_VRSZ_EN : 0));

	if (srcW != dstW)
		coef_index = (fracW.num - 1) << 16;
	if (srcH != dstH)
		coef_index |= fracH.num - 1;

	/* Set the LUT index register */
	write_reg(ipu, REG_RSZ_COEF_INDEX, coef_index);

	/* Set the input/output height/width */
	write_reg(ipu, REG_IN_FM_GS,
				((srcW * bytes_per_pixel) << IN_FM_W_SFT)
				| (srcH << IN_FM_H_SFT));
	write_reg(ipu, REG_OUT_GS,
				((dstW * bytes_per_pixel) << IN_FM_W_SFT)
				| (dstH << IN_FM_H_SFT));

	/* Set the input/output stride */
	write_reg(ipu, REG_Y_STRIDE, ipu->src_stride);
	write_reg(ipu, REG_OUT_STRIDE, ipu->dst_stride);

	if (srcW != dstW) {
		/* Set the horizontal resize coefficients */
		switch (algorithm) {
		case IPU_NEAREST_NEIGHBOR:
			ipu_set_nearest_resize_coef(ipu, &fracW, REG_HRSZ_COEF_LUT);
			break;
		case IPU_BILINEAR:
			ipu_set_bilinear_resize_coef(ipu, &fracW, REG_HRSZ_COEF_LUT);
			break;
		}
	}

	if (srcH != dstH) {
		/* Set the vertical resize coefficients */
		switch (algorithm) {
		case IPU_NEAREST_NEIGHBOR:
			ipu_set_nearest_resize_coef(ipu, &fracH, REG_VRSZ_COEF_LUT);
			break;
		case IPU_BILINEAR:
			ipu_set_bilinear_resize_coef(ipu, &fracH, REG_VRSZ_COEF_LUT);
			break;
		}
	}
}

static void ipu_reset(struct ipu *ipu, enum ipu_resize_algorithm algorithm,
			unsigned int srcW, unsigned int srcH,
			unsigned int dstW, unsigned int dstH, bool swap)
{
	/* Enable the chip and packed mode */
	write_reg(ipu, REG_CTRL, IPU_CTRL_CHIP_EN | IPU_CTRL_SPKG_SEL);

	/* Set input/output pixel format to rgb888 */
	write_reg(ipu, REG_D_FMT, (2 << OUT_FMT_SFT) | (2 << IN_FMT_SFT));

	/* Set the input/output addresses */
	if (swap) {
		write_reg(ipu, REG_Y_ADDR, ipu->fb + 240 * ipu->src_stride);
		write_reg(ipu, REG_OUT_ADDR, (uint32_t) ipu->fb);
	} else {
		write_reg(ipu, REG_Y_ADDR, (uint32_t) ipu->fb);
		write_reg(ipu, REG_OUT_ADDR, ipu->fb + 240 * ipu->dst_stride);
	}

	printf("Setting the resize params...\n");
	/* Set the resize params */
	ipu_set_resize_params(ipu, algorithm, srcW, srcH, dstW, dstH, 4);
}

static void ipu_run(struct ipu *ipu)
{
	set_bit(ipu, REG_CTRL, IPU_CTRL_RUN);
}

static void ipu_control_clock(struct ipu *ipu, int enable)
{
	ipu_stop(ipu, !!enable);

	void *addr = mmap(NULL, 0x100, PROT_READ | PROT_WRITE,
				MAP_SHARED, ipu->dev_mem_fd, JZ4770_CPM_BASE_ADDR);
	if (!addr) {
		fprintf(stderr, "Unable to mmap register clkgr0\n");
	} else {
		uint32_t *ptr = addr + REG_CLKGR0;
		if (enable)
			*ptr &= ~CLKGR0_IPU;
		else
			*ptr |= CLKGR0_IPU;
		munmap(addr, 0x100);
	}

	/* Sleep a bit to be sure that the clock is ready */
	usleep(4000);
}

#define ipu_enable_clock(ipu) ipu_control_clock(ipu, 1)
#define ipu_disable_clock(ipu) ipu_control_clock(ipu, 0)

static void ipu_run_test(struct ipu *ipu, bool swap)
{
	//printf("Enabling clock...\n");
	//ipu_enable_clock(ipu);
	printf("Reseting IPU...\n");
	ipu_reset(ipu, IPU_NEAREST_NEIGHBOR, 320, 240, 640, 480, swap);
	printf("IPU reseted. Running IPU...\n");
	draw_image();
	ipu_run(ipu);
	draw_image();
	printf("IPU should be running now. Waiting for EOF status bit...\n");
	ipu_wait_completion(ipu);
	printf("Sequence completed! IPU exited with status code %x.\n\n",
				read_reg(ipu, REG_STATUS) & 0x7);
	print_regs(ipu);
	//printf("\nStopping clock...\n");
	//ipu_disable_clock(ipu);
}

static void quit_all(int err)
{
	if (ipu) {
		ipu_disable_clock(ipu);
		close(ipu->dev_mem_fd);
		munmap(ipu->base, 0x64);
		free(ipu);
	}

	exit(err);
}

static void set_handler(int signal, void (*handler)(int))
{
	struct sigaction sig;
	sigaction(signal, NULL, &sig);
	sig.sa_handler = handler;
	sigaction(signal, &sig, NULL);
}

int main(void)
{
	set_handler(SIGINT, &quit_all);
	set_handler(SIGSEGV, &quit_all);
	set_handler(SIGTERM, &quit_all);

	int fd = open("/dev/fb0", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Unable to open framebuffer\n");
		return EXIT_FAILURE;
	}

	struct fb_fix_screeninfo fbinfo;
	struct fb_var_screeninfo varinfo;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fbinfo) < 0) {
		fprintf(stderr, "get fixed screen info failed\n");
		close(fd);
		return EXIT_FAILURE;
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &varinfo) < 0) {
		fprintf(stderr, "get var screen info failed\n");
		close(fd);
		return EXIT_FAILURE;
	}

	close(fd);

	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Unable to open /dev/mem\n");
		return EXIT_FAILURE;
	}

	ipu = malloc(sizeof(*ipu));
	if (!ipu) {
		fprintf(stderr, "Unable to allocate memory\n");
		close(fd);
		return EXIT_FAILURE;
	}

	void *addr = mmap(NULL, 0x64, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, JZ4770_IPU_BASE_ADDR);
	if (!addr) {
		fprintf(stderr, "Unable to mmap /dev/mem\n");
		free(ipu);
		close(fd);
		return EXIT_FAILURE;
	}

	ipu->base = addr;
	ipu->fb = fbinfo.smem_start;
	ipu->src_stride = ipu->dst_stride = fbinfo.line_length;
	ipu->dev_mem_fd = fd;

	printf("Framebuffer physical address: 0x%lx\n", fbinfo.smem_start);

	ipu_run_test(ipu, !varinfo.yoffset);
	quit_all(EXIT_SUCCESS);

	return 0;
}
