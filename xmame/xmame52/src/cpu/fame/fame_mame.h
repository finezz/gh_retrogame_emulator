#ifndef FAME__HEADER
#define FAME__HEADER

#include "osd_cpu.h"

enum
{
	/* NOTE: M68K_SP fetches the current SP, be it USP, ISP, or MSP */
	M68K_PC=1, M68K_SP, M68K_ISP, M68K_USP, M68K_MSP, M68K_SR, M68K_VBR,
	M68K_SFC, M68K_DFC, M68K_CACR, M68K_CAAR, M68K_PREF_ADDR, M68K_PREF_DATA,
	M68K_D0, M68K_D1, M68K_D2, M68K_D3, M68K_D4, M68K_D5, M68K_D6, M68K_D7,
	M68K_A0, M68K_A1, M68K_A2, M68K_A3, M68K_A4, M68K_A5, M68K_A6, M68K_A7
};

/* The MAME API for FAME */

#define FAME_INT_NONE 0
#define FAME_IRQ_1    1
#define FAME_IRQ_2    2
#define FAME_IRQ_3    3
#define FAME_IRQ_4    4
#define FAME_IRQ_5    5
#define FAME_IRQ_6    6
#define FAME_IRQ_7    7

#define FAME_INT_ACK_AUTOVECTOR    -1
#define FAME_INT_ACK_SPURIOUS      -2

extern void fame_reset(void *param);
extern void fame_exit(void);
extern int	fame_execute(int cycles);
extern unsigned fame_get_context(void *dst);
extern void fame_set_context(void *src);
extern unsigned fame_get_pc(void);
extern void fame_set_pc(unsigned val);
extern unsigned fame_get_sp(void);
extern void fame_set_sp(unsigned val);
extern unsigned fame_get_reg(int regnum);
extern void fame_set_reg(int regnum, unsigned val);
extern void fame_set_nmi_line(int state);
extern void fame_set_irq_line(int irqline, int state);
extern void fame_set_irq_callback(int (*callback)(int irqline));
extern const char *fame_info(void *context, int regnum);
extern unsigned fame_dasm(char *buffer, unsigned pc);

extern int fame_ICount;
extern FAME_CONTEXT fame68kcontext;


#endif /* FAME__HEADER */
