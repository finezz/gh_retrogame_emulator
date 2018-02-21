
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mame.h"
#include "m68000.h"
#include "state.h"
#include "cpuintrf.h"
#include "fame.h"   // Should contain all FAME_* options
#include "fame_mame.h"   // Should contain all FAME_* options

// These options are only used in fame_mame.c
// Number of memory handlers
//#define FAME_DEFAULT_CALLBACKS

// Debug options
//#define DEBUG_FAME_SHOW_BANKS
//#define DEBUG_FAME
//#define DEBUG_FAME_SINGLE_STEP
//#define DEBUG_FAME_FULL_START 60
//#define DEBUG_FAME_FULL_STOP 72


unsigned int nContext = 0;

#define HT_BANK1  1
#define HT_BANKMAX (HT_BANK1 + MAX_BANKS - 1)
#if LSB_FIRST
#define BYTE_XOR_BE(a) ((a) ^ 1)
#else
#define BYTE_XOR_BE(a) (a)
#endif

#ifdef DEBUG_FAME_FULL_START
#define _68k_dreg(num) (fame68kcontext.dreg[(num)])
#define _68k_areg(num) (fame68kcontext.areg[(num)])
#define _68k_sreg       fame68kcontext.sr
#endif


/* Required by FAME to setup memory handlers */
extern MHELE readhardware[];
extern MHELE writehardware[];
extern data_t (*memoryreadhandler[MH_HARDMAX])(offs_t address);
extern int memoryreadoffset[MH_HARDMAX];
extern void (*memorywritehandler[MH_HARDMAX])(offs_t address,data_t data);
extern int memorywriteoffset[MH_HARDMAX];

extern data_t cpu_readmem32(offs_t offset);
extern data_t cpu_readmem32_word(offs_t offset);
extern void cpu_writemem32(offs_t offset,data_t data);
extern void cpu_writemem32_word(offs_t offset,data_t data);
extern void cpu_setOPbase24(int pc);

//proto
void fame_default_irq_callback(unsigned int level);

#include "fame_mame_funcs.h"

static FAME_CONTEXT ctx[MAX_CONTEXTS];
static M68K_PROGRAM prg[MAX_CONTEXTS * (FAME_N_ENTRIES+1)];
static M68K_DATA    dt_read8[MAX_CONTEXTS * (FAME_N_ENTRIES+1)];
static M68K_DATA    dt_write8[MAX_CONTEXTS * (FAME_N_ENTRIES+1)];
static M68K_DATA    dt_read16[MAX_CONTEXTS * (FAME_N_ENTRIES+1)];
static M68K_DATA    dt_write16[MAX_CONTEXTS * (FAME_N_ENTRIES+1)];

#define FAME_FETCHBITS 12
#define FAME_DATABITS 12
#ifdef FAME_USE_CONTEXT_SWITCH
static uint16_t * Fetch[MAX_CONTEXTS * (1 << FAME_FETCHBITS)];
static M68K_READ_BYTE_DATA DataRB[MAX_CONTEXTS * (1 << FAME_DATABITS)];
static M68K_READ_WORD_DATA DataRW[MAX_CONTEXTS * (1 << FAME_DATABITS)];
static M68K_WRITE_BYTE_DATA DataWB[MAX_CONTEXTS * (1 << FAME_DATABITS)];
static M68K_WRITE_WORD_DATA DataWW[MAX_CONTEXTS * (1 << FAME_DATABITS)];
#else
extern uint16_t * Fetch[1 << FAME_FETCHBITS];
extern M68K_READ_BYTE_DATA DataRB[(1 << FAME_DATABITS)];
extern M68K_READ_WORD_DATA DataRW[(1 << FAME_DATABITS)];
extern M68K_WRITE_BYTE_DATA DataWB[(1 << FAME_DATABITS)];
extern M68K_WRITE_WORD_DATA DataWW[(1 << FAME_DATABITS)];
#endif


static int get_cpu_readmem32 (int address, unsigned **punt)
{
	MHELE hw = cur_mrhard[(unsigned)address >> (ABITS2_32 + ABITS_MIN_32)];
	if (hw <= HT_BANKMAX)
	{
		*punt=(unsigned *)(&cpu_bankbase[hw][0 - memoryreadoffset[hw]]);
		return 0;
	}
	if (hw >= MH_HARDMAX)
	{
		hw -= MH_HARDMAX;
		hw = readhardware[(hw << MH_SBITS) + (((unsigned)address >> ABITS_MIN_32) & MHMASK(ABITS2_32))];
		if (hw <= HT_BANKMAX)
		{
			*punt=(unsigned *)(&cpu_bankbase[hw][0 - memoryreadoffset[hw]]);
			return 0;
		}
	}
	*punt=(unsigned *)memoryreadhandler[hw];
	return 1;
}

static int get_cpu_writemem32 (int address, unsigned **punt)
{
	MHELE hw = cur_mwhard[(unsigned)address >> (ABITS2_32 + ABITS_MIN_32)];
	if (hw <= HT_BANKMAX)
	{
		*punt=(unsigned *)(&cpu_bankbase[hw][0 - memorywriteoffset[hw]]);
		return 0;
	}
	if (hw >= MH_HARDMAX)
	{
		hw -= MH_HARDMAX;
		hw = writehardware[(hw << MH_SBITS) + (((unsigned)address >> ABITS_MIN_32) & MHMASK(ABITS2_32))];
		if (hw <= HT_BANKMAX)
		{
			*punt=(unsigned *)(&cpu_bankbase[hw][0 - memorywriteoffset[hw]]);
			return 0;
		}
	}

	*punt=(unsigned *)memorywritehandler[hw];
	return 1;
}


static int get_cpu_readoffset(int address)
{
	MHELE hw = cur_mrhard[(unsigned)address >> (ABITS2_32 + ABITS_MIN_32)];
	if (hw >= MH_HARDMAX)
	{
		MHELE back=hw;
		hw -= MH_HARDMAX;
		hw = readhardware[(hw << MH_SBITS) + (((unsigned)address >> ABITS_MIN_32) & MHMASK(ABITS2_32))];
		if (hw <= HT_BANKMAX)
			hw=back;
	}
	return (int)memoryreadoffset[hw];
}

static int get_cpu_writeoffset(int address)
{
	MHELE hw = cur_mwhard[(unsigned)address >> (ABITS2_32 + ABITS_MIN_32)];
	if (hw >= MH_HARDMAX)
	{
		MHELE back = hw;
		hw -= MH_HARDMAX;
		hw = writehardware[(hw << MH_SBITS) + (((unsigned)address >> ABITS_MIN_32) & MHMASK(ABITS2_32))];
		if (hw <= HT_BANKMAX)
			hw = back;
	}
	return (int)memorywriteoffset[hw];
}


static unsigned get_data_read (int address)
{
	MHELE hw = cur_mrhard[(unsigned)address >> (ABITS2_32 + ABITS_MIN_32)];
	if (hw <= HT_BANKMAX)
		return (unsigned)(&cpu_bankbase[hw][address - memoryreadoffset[hw]]);
	if (hw >= MH_HARDMAX)
	{
		hw -= MH_HARDMAX;
		hw = readhardware[(hw << MH_SBITS) + (((unsigned)address >> ABITS_MIN_32) & MHMASK(ABITS2_32))];
		if (hw <= HT_BANKMAX)
			return (unsigned)(&cpu_bankbase[hw][address - memoryreadoffset[hw]]);
	}
	return 0xFFFFFFFF;
}

static unsigned get_data_write (int address)
{
	MHELE hw = cur_mwhard[(unsigned)address >> (ABITS2_32 + ABITS_MIN_32)];
	if (hw <= HT_BANKMAX)
		return (unsigned)(&cpu_bankbase[hw][address - memorywriteoffset[hw]]);
	if (hw >= MH_HARDMAX)
	{
		hw -= MH_HARDMAX;
		hw = writehardware[(hw << MH_SBITS) + (((unsigned)address >> ABITS_MIN_32) & MHMASK(ABITS2_32))];
		if (hw <= HT_BANKMAX)
			return (unsigned)(&cpu_bankbase[hw][address - memorywriteoffset[hw]]);
	}
	return 0xFFFFFFFF;
}

#ifdef DEBUG_FAME_SHOW_BANKS
static void show_banks_all(void)
{
	int i=0;
	printf("\n-----------\n");
	printf("- PROGRAM -\n");
	printf("-----------\n");
	while((prg[i + nContext*(FAME_N_ENTRIES+1)].offset!=0)&&(prg[i + nContext*(FAME_N_ENTRIES+1)].high_addr!=0xFFFFFFFF))
	{
		printf("%i:\t%X %X -> %p\n",i,prg[i + nContext*(FAME_N_ENTRIES+1)].low_addr,prg[i + nContext*(FAME_N_ENTRIES+1)].high_addr,prg[i + nContext*(FAME_N_ENTRIES+1)].offset);
		i++;
	}
	i=0;
	printf("\n--------\n");
	printf("- READ -\n");
	printf("--------\n");
	while((dt_read8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler!=NULL)||(dt_read8[i + nContext*(FAME_N_ENTRIES+1)].data!=NULL))
	{
		if ((!dt_read8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler)&&(dt_read8[i + nContext*(FAME_N_ENTRIES+1)].data))
			printf("%i\t%X %X ->            %p\n",i,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].high_addr,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].data);
		else
		if ((dt_read8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler)&&(!dt_read8[i + nContext*(FAME_N_ENTRIES+1)].data))
		{
			if (dt_read8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler==(void *)&cpu_readmem32)
				printf("%i\t%X %X -> DEFAULT\n",i,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].high_addr);
			else
			{
				int j;
				for (j=0;j<FAME_N_ENTRIES;j++)
					if (dt_read8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler==(void *)miread8_funcs_indirect[j + nContext*(FAME_N_ENTRIES+1)]) break;
				if (j>=FAME_N_ENTRIES)
					printf("%i\t%X %X -> %p\n",i,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].high_addr,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler);
				else
					printf("%i\t%X %X -> FUNC-%i (%p,%X)\n",i,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].high_addr,j,miread8_funcs[j + nContext*(FAME_N_ENTRIES+1)],miread8_offs[j + nContext*(FAME_N_ENTRIES+1)]);
			}
		}
		else
			printf("%i\t%X %X -> %p %p\n",i,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].high_addr,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler,dt_read8[i + nContext*(FAME_N_ENTRIES+1)].data);
			
		i++;
	}
	i=0;
	printf("\n---------\n");
	printf("- WRITE -\n");
	printf("---------\n");
	while((dt_write8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler!=NULL)||(dt_write8[i + nContext*(FAME_N_ENTRIES+1)].data!=NULL))
	{
		if ((!dt_write8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler)&&(dt_write8[i + nContext*(FAME_N_ENTRIES+1)].data))
			printf("%i\t%X %X ->            %p\n",i,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].high_addr,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].data);
		else
		if ((dt_write8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler)&&(!dt_write8[i + nContext*(FAME_N_ENTRIES+1)].data))
		{
			if (dt_write8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler==(void *)&cpu_writemem32)
				printf("%i\t%X %X -> DEFAULT\n",i,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].high_addr);
			else
			{
				int j;
				for (j=0;j<FAME_N_ENTRIES;j++)
					if (dt_write8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler==(void *)miwrite8_funcs_indirect[j + nContext*(FAME_N_ENTRIES+1)]) break;
				if (j>=FAME_N_ENTRIES)
					printf("%i\t%X %X -> %p\n",i,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].high_addr,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler);
				else
					printf("%i\t%X %X -> FUNC-%i (%p,%X)\n",i,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].high_addr,j,miwrite8_funcs[j + nContext*(FAME_N_ENTRIES+1)],miwrite8_offs[j + nContext*(FAME_N_ENTRIES+1)]);
			}
		}
		else
			printf("%i\t%X %X -> %p %p\n",i,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].low_addr,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].high_addr,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler,dt_write8[i + nContext*(FAME_N_ENTRIES+1)].data);
		i++;
	}
	printf("\n"); fflush(stdout);
}

#endif


#define CALCULA_OFFS(DI) ((unsigned *)((((unsigned)(DI))-actual+begin)&0xffffffc0))
#define QUITA_BANCO(DI) ((unsigned*)(((unsigned)(DI))&0xffffffc0))

#define TEST_BANK_READWRITE(_R_OR_W_) \
static int test_back_##_R_OR_W_(unsigned begin, unsigned theend) \
{ \
	unsigned start=begin; \
	unsigned end=theend; \
	unsigned actual=(((start+end)>>1)+4)&0xfffffffe; \
	unsigned *punt; \
	get_cpu_##_R_OR_W_##mem32(start,&punt); \
	unsigned *pstart=QUITA_BANCO(punt); \
	while(actual<end) \
	{ \
		unsigned *p; \
		int f=get_cpu_##_R_OR_W_##mem32(actual,&p); \
		if (p!=punt) \
		{ \
			if (f) \
				return 0; \
			if ((CALCULA_OFFS(p)!=pstart)) \
				return 0; \
		} \
		punt=p; \
		start=actual; \
		actual=(((start+end)>>1)+4)&0xfffffffe; \
	} \
	start=begin; \
	actual=((start+end)>>1)-2; \
	while((actual>start)&&(actual<0x1000000)) \
	{ \
		unsigned *p; \
		int f=get_cpu_##_R_OR_W_##mem32(actual,&p); \
		if (p!=punt) \
		{ \
			if (f) \
				return 0; \
			if ((CALCULA_OFFS(p)!=pstart)) \
				return 0; \
		} \
		punt=p; \
		end=actual; \
		actual=(((start+end)>>1)-4)&0xfffffffe; \
	} \
	for(actual=begin;actual<theend;actual+=64) \
	{ \
		unsigned *p; \
		int f=get_cpu_##_R_OR_W_##mem32(actual,&p); \
		if (p!=punt) \
		{ \
			if (f) \
				return 0; \
			if ((CALCULA_OFFS(p)!=pstart)) \
				return 0; \
		} \
	} \
	return 1; \
}

TEST_BANK_READWRITE(read)
TEST_BANK_READWRITE(write)

#undef TEST_BANK_READWRITE
#undef CALCULA_OFFS
#undef QUITA_BANCO

static void put_prg(M68K_PROGRAM *p, unsigned low, unsigned high, unsigned offset)
{
	p->low_addr=(unsigned)low;
	p->high_addr=(unsigned)high;
	p->offset=(uint16_t *)offset;
}

static void put_end_prg(M68K_PROGRAM *p)
{
	p->low_addr=(unsigned)-1;
	p->high_addr=(unsigned)-1;
	p->offset=(uint16_t *)0;
}

static void put_addr_low(M68K_DATA *r8, M68K_DATA *r16, unsigned low_addr)
{
	r8->low_addr=r16->low_addr=low_addr;
}

static void put_addr_high(M68K_DATA *r8, M68K_DATA *r16, unsigned high_addr)
{
	r8->high_addr=r16->high_addr=high_addr;
}

static void put_addr(M68K_DATA *r8, M68K_DATA *r16,
		unsigned low_addr , unsigned high_addr)
{
	put_addr_low(r8,r16,low_addr);
	put_addr_high(r8,r16,high_addr);
}

static void put_handler(M68K_DATA *r, void *handler)
{
	r->mem_handler=handler;
	r->data=NULL;
}

static void put_end(M68K_DATA *r8, M68K_DATA *r16)
{
	r8->low_addr=r16->low_addr=(unsigned)-1;
	r8->high_addr=r16->high_addr=(unsigned)-1;
	r8->mem_handler=r16->mem_handler=NULL;
	r8->data=NULL;	r16->data=NULL;
}

#define PUT_READWRITE(_R_OR_W_) \
static void put_default_##_R_OR_W_##handler(M68K_DATA *r8, M68K_DATA *r16) \
{ \
	put_handler(r8,(void *)&cpu_##_R_OR_W_##mem32); \
	put_handler(r16,(void *)&cpu_##_R_OR_W_##mem32_word); \
} \
 \
static void put_custom_##_R_OR_W_(unsigned j, unsigned start, unsigned back) \
{ \
	mi##_R_OR_W_##8_funcs[j+nContext*FAME_N_ENTRIES]=(mi##_R_OR_W_##8_func)back; \
/* debug_printf("ctx:%d %s[%d]=%x\n", nContext, "mi##_R_OR_W_##8_funcs", j+nContext*FAME_N_ENTRIES, back); */ \
	mi##_R_OR_W_##8_offs[j+nContext*FAME_N_ENTRIES]=get_cpu_##_R_OR_W_##offset(start); \
/* debug_printf("ctx:%d %s[%d]=%x\n", nContext, "mi##_R_OR_W_##8_offs", j+nContext*FAME_N_ENTRIES, start); */ \
	put_handler(&dt_##_R_OR_W_##8[j + nContext*(FAME_N_ENTRIES+1)],(void *)mi##_R_OR_W_##8_funcs_indirect[j+nContext*FAME_N_ENTRIES]); \
	mi##_R_OR_W_##16_funcs[j+nContext*FAME_N_ENTRIES]=(mi##_R_OR_W_##16_func)back; \
	mi##_R_OR_W_##16_offs[j+nContext*FAME_N_ENTRIES]=mi##_R_OR_W_##8_offs[j+nContext*FAME_N_ENTRIES]; \
	put_handler(&dt_##_R_OR_W_##16[j + nContext*(FAME_N_ENTRIES+1)],(void *)mi##_R_OR_W_##16_funcs_indirect[j+nContext*FAME_N_ENTRIES]); \
} \
 \
static int put_data_##_R_OR_W_(M68K_DATA *r8, M68K_DATA *r16, unsigned start, unsigned back) \
{ \
	back--; \
	r8->mem_handler=r16->mem_handler=NULL; \
	unsigned data=get_data_##_R_OR_W_(start); \
	if (data&3) \
	{ \
		put_default_##_R_OR_W_##handler(r8,r16); \
		return 1; \
	} \
	r8->data=r16->data=(void *)(data-start); \
/*debug_printf("put_data_RW r/w8 %x = %x\n", r8, (unsigned int *)(data-start));*/ \
	return 0; \
} 

PUT_READWRITE(read)
PUT_READWRITE(write)

#undef PUT_READWRITE

#define SEARCH_PRG() \
{ \
	change_pc32(i); \
	if (back_op!=(unsigned)OP_ROM) \
	{ \
		if (start_op==1) \
		{ \
			start_op=0; \
			back_op=(unsigned)OP_ROM; \
		} \
		else \
		{ \
			put_prg(&prg[j_op + nContext*(FAME_N_ENTRIES+1)],start_op,i-1,back_op); \
			j_op++; \
			start_op=i; \
		} \
		back_op=(unsigned)OP_ROM; \
	} \
}

#define SEARCH_BANK_READWRITE(_R_OR_W_,_LIMIT_) \
{ \
	put_addr(&dt_##_R_OR_W_##8[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],start_##_R_OR_W_,(_LIMIT_)-1); \
	if (test_back_##_R_OR_W_(start_##_R_OR_W_,(_LIMIT_))) \
	{ \
		if (func_##_R_OR_W_) \
			put_custom_##_R_OR_W_(j_##_R_OR_W_,start_##_R_OR_W_,back_##_R_OR_W_); \
		else \
			func_##_R_OR_W_=put_data_##_R_OR_W_(&dt_##_R_OR_W_##8[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],start_##_R_OR_W_,back_##_R_OR_W_); \
	} \
	else \
		put_default_##_R_OR_W_##handler(&dt_##_R_OR_W_##8[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)]); \
	j_##_R_OR_W_++; \
	start_##_R_OR_W_=(_LIMIT_); \
} \

#define SEARCH_READWRITE(_R_OR_W_) \
{ \
	func=get_cpu_##_R_OR_W_##mem32(i,&punt); \
	if ((j_##_R_OR_W_<(FAME_N_ENTRIES-1)) && (i>start_##_R_OR_W_) && \
	    ( (func!=func_##_R_OR_W_) || \
 	      (func)&&((unsigned)punt!=(unsigned)back_##_R_OR_W_) || \
 	      (!func)&& \
	      (((unsigned)punt-(i-start_##_R_OR_W_))!=(unsigned)back_##_R_OR_W_)&& \
	      ((unsigned)punt!=(unsigned)back_##_R_OR_W_) \
	   )) \
	{ \
		if (start_##_R_OR_W_==1) \
		{ \
			start_##_R_OR_W_=0; \
			back_##_R_OR_W_=(unsigned)punt; \
			func_##_R_OR_W_=(unsigned)func; \
		} \
		else \
		if (i&0xFFF) \
		{ \
			if ((!func_##_R_OR_W_)&&((i-start_##_R_OR_W_)>4096)) \
			{ \
				put_addr(&dt_##_R_OR_W_##8[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],start_##_R_OR_W_,(i&0xFFF000)-1); \
				put_data_##_R_OR_W_(&dt_##_R_OR_W_##8[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],start_##_R_OR_W_,back_##_R_OR_W_); \
				j_##_R_OR_W_++; \
				put_addr_low(&dt_##_R_OR_W_##8[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],(i&0xFFF000)); \
				j_##_R_OR_W_++; \
				 \
			} \
			else \
			if ((dt_##_R_OR_W_##8[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)].high_addr+1-dt_##_R_OR_W_##8[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)].low_addr)>=8192) \
			{ \
				dt_##_R_OR_W_##8[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)].high_addr-=4096; \
				put_addr_low(&dt_##_R_OR_W_##8[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],start_##_R_OR_W_-4096); \
				j_##_R_OR_W_++; \
			} \
			put_default_##_R_OR_W_##handler(&dt_##_R_OR_W_##8[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)]); \
			put_addr_high(&dt_##_R_OR_W_##8[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)],(i&0xFFF000)+0xFFF); \
			func=1; \
			back_##_R_OR_W_=(unsigned)dt_##_R_OR_W_##8[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)].mem_handler; \
			start_##_R_OR_W_=(i&0xFFF000)+0x1000; \
		} \
		else \
			SEARCH_BANK_READWRITE(_R_OR_W_,i) \
		back_##_R_OR_W_=(unsigned)punt; \
		func_##_R_OR_W_=func; \
	} \
}

#define FINISH_SEARCH_PRG() \
{ \
	put_prg(&prg[j_op + nContext*(FAME_N_ENTRIES+1)],start_op,0xFFFFFF,back_op); \
	put_end_prg(&prg[j_op+1 + nContext*(FAME_N_ENTRIES+1)]); \
}

#define FINISH_SEARCH_READWRITE(_R_OR_W_) \
{ \
	if (dt_##_R_OR_W_##8[j_##_R_OR_W_-1 + nContext*(FAME_N_ENTRIES+1)].high_addr!=0xFFFFFF) \
		SEARCH_BANK_READWRITE(_R_OR_W_,0x1000000) \
	put_end(&dt_##_R_OR_W_##8[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)],&dt_##_R_OR_W_##16[j_##_R_OR_W_ + nContext*(FAME_N_ENTRIES+1)]); \
}	

static void search_memory_banks(void)
{
	unsigned i;
       	unsigned j_op=0, j_read=0, j_write=0;
	unsigned start_op=1, start_read=1, start_write=1;
	unsigned func_read=0xFF000FFF, func_write=0xFFFFFFFF;
	unsigned back_op=0xFFFFFFFF, back_read=0xFFFFFFFF, back_write=0xFFFFFFFF;

	for(i=0;i<0x1000000;i+=2)
	{
		unsigned *punt;
		int func;
		if (!(i&0xFFF))
			SEARCH_PRG()
#ifndef FAME_ONLY_HANDLER
		SEARCH_READWRITE(read)
		SEARCH_READWRITE(write)
#endif
	}
	FINISH_SEARCH_PRG()
#ifndef FAME_ONLY_HANDLER
	FINISH_SEARCH_READWRITE(read)
	FINISH_SEARCH_READWRITE(write)
#endif

#ifdef FAME_ONLY_HANDLER
	put_addr(&dt_read8[0 + nContext*(FAME_N_ENTRIES+1)],&dt_read16[0 + nContext*(FAME_N_ENTRIES+1)],0,0xFFFFFF);
	put_default_readhandler(&dt_read8[0 + nContext*(FAME_N_ENTRIES+1)],&dt_read16[0 + nContext*(FAME_N_ENTRIES+1)]);
	put_end(&dt_read8[1 + nContext*(FAME_N_ENTRIES+1)],&dt_read16[1 + nContext*(FAME_N_ENTRIES+1)]);
	put_addr(&dt_write8[0 + nContext*(FAME_N_ENTRIES+1)],&dt_write16[0 + nContext*(FAME_N_ENTRIES+1)],0,0xFFFFFF);
	put_default_writehandler(&dt_write8[0 + nContext*(FAME_N_ENTRIES+1)],&dt_write16[0 + nContext*(FAME_N_ENTRIES+1)]);
	put_end(&dt_write8[1 + nContext*(FAME_N_ENTRIES+1)],&dt_write16[1 + nContext*(FAME_N_ENTRIES+1)]);
#else
#ifdef FAME_ONE_HANDLER
	for(i=0;i<FAME_N_ENTRIES;i++)
	{
		if (dt_read8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler || dt_read8[i + nContext*(FAME_N_ENTRIES+1)].data)
			put_default_readhandler(&dt_read8[i + nContext*(FAME_N_ENTRIES+1)],&dt_read16[i + nContext*(FAME_N_ENTRIES+1)]);
		if (dt_write8[i + nContext*(FAME_N_ENTRIES+1)].mem_handler || dt_write8[i + nContext*(FAME_N_ENTRIES+1)].data)
			put_default_writehandler(&dt_write8[i + nContext*(FAME_N_ENTRIES+1)],&dt_write16[i + nContext*(FAME_N_ENTRIES+1)]);
	}
#endif
#endif
#ifdef DEBUG_FAME_SHOW_BANKS
	show_banks_all();
#endif
}

void fame_reset(void* param)
{
	fame68k_init();	    // Only done once regardless of number of contexts

	/* Initial setup of contexts is only performed once per context */
	/* Each CPU is reset from cpu_run() when emulation first starts */
	if (nContext < MAX_CONTEXTS)
	{
	    /* Setup the memory handlers */
	    memset(&prg[0 + nContext*(FAME_N_ENTRIES+1)], 0, sizeof(M68K_PROGRAM) * (FAME_N_ENTRIES+1) );
	    memset(&dt_read8[0 + nContext*(FAME_N_ENTRIES+1)], 0, sizeof(M68K_DATA) * (FAME_N_ENTRIES+1) );
	    memset(&dt_read16[0 + nContext*(FAME_N_ENTRIES+1)], 0, sizeof(M68K_DATA) * (FAME_N_ENTRIES+1) );
	    memset(&dt_write8[0 + nContext*(FAME_N_ENTRIES+1)], 0, sizeof(M68K_DATA) * (FAME_N_ENTRIES+1) );
	    memset(&dt_write16[0 + nContext*(FAME_N_ENTRIES+1)], 0, sizeof(M68K_DATA) * (FAME_N_ENTRIES+1) );

	    search_memory_banks();

	    /* Setup the context */
	    memset(&ctx[nContext], 0, sizeof(FAME_CONTEXT));

	    ctx[nContext].fetch=(M68K_PROGRAM*)&prg[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].read_byte=(M68K_DATA*)&dt_read8[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].read_word=(M68K_DATA*)&dt_read16[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].write_byte=(M68K_DATA*)&dt_write8[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].write_word=(M68K_DATA*)&dt_write16[nContext*(FAME_N_ENTRIES+1)];
#ifdef FAME_SV_USER
	    ctx[nContext].sv_fetch=(M68K_PROGRAM*)&prg[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].sv_read_byte=(M68K_DATA*)&dt_read8[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].sv_read_word=(M68K_DATA*)&dt_read16[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].sv_write_byte=(M68K_DATA*)&dt_write8[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].sv_write_word=(M68K_DATA*)&dt_write16[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].user_fetch=(M68K_PROGRAM*)&prg[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].user_read_byte=(M68K_DATA*)&dt_read8[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].user_read_word=(M68K_DATA*)&dt_read16[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].user_write_byte=(M68K_DATA*)&dt_write8[nContext*(FAME_N_ENTRIES+1)];
	    ctx[nContext].user_write_word=(M68K_DATA*)&dt_write16[nContext*(FAME_N_ENTRIES+1)];
#endif
#ifdef FAME_USE_CONTEXT_SWITCH
	    ctx[nContext].FetchList=&Fetch[nContext*(1 << FAME_FETCHBITS)];
	    ctx[nContext].DataRB=&DataRB[nContext*(1 << FAME_DATABITS)];
	    ctx[nContext].DataRW=&DataRW[nContext*(1 << FAME_DATABITS)];
	    ctx[nContext].DataWB=&DataWB[nContext*(1 << FAME_DATABITS)];
	    ctx[nContext].DataWW=&DataWW[nContext*(1 << FAME_DATABITS)];
#endif
#ifdef FAME_DEFAULT_CALLBACKS
	    // Default IRQ callback
	    ctx[nContext].iack_handler=(void (*)(unsigned int))fame_default_irq_callback;
#endif

	    //fame68k_set_context(&ctx[nContext]);	// Only call once per context, subsequent calls to set_context
							// should just copy the FAME_CONTEXT struc
	    memcpy(&fame68kcontext, &ctx[nContext], sizeof(FAME_CONTEXT));
	    fame68k_SetBanks();				// Setup the context access
	    nContext++;
	}

	fame68k_reset();			// Resets contexts, sets the PC
	change_pc24(fame68k_get_pc());
	fame_ICount=0;
//debug_printf("fame68k_reset(cpu=%d) pc=%d\n", cpu_getactivecpu(), fame68k_get_pc());
}

void fame_exit(void)
{
}

unsigned fame_get_pc(void)
{
	return fame68k_get_pc();
}

void fame_set_pc(unsigned val)
{
	fame68k_set_register(M68K_REG_PC, val);
}
unsigned fame_get_sp(void)
{
	return fame68k_get_register(M68K_REG_A7);
}

void fame_set_sp(unsigned val)
{
	fame68k_set_register(M68K_REG_A7,val);
}

unsigned fame_get_reg(int regnum)
{
	switch( regnum )
	{
		case M68K_ISP:
		case M68K_USP: return fame68k_get_register(M68K_REG_ASP);
		case M68K_SR:  return fame68k_get_register(M68K_REG_SR);
		case M68K_D0:  return fame68k_get_register(M68K_REG_D0);
		case M68K_D1:  return fame68k_get_register(M68K_REG_D1);
		case M68K_D2:  return fame68k_get_register(M68K_REG_D2);
		case M68K_D3:  return fame68k_get_register(M68K_REG_D3);
		case M68K_D4:  return fame68k_get_register(M68K_REG_D4);
		case M68K_D5:  return fame68k_get_register(M68K_REG_D5);
		case M68K_D6:  return fame68k_get_register(M68K_REG_D6);
		case M68K_D7:  return fame68k_get_register(M68K_REG_D7);
		case M68K_A0:  return fame68k_get_register(M68K_REG_A0);
		case M68K_A1:  return fame68k_get_register(M68K_REG_A1);
		case M68K_A2:  return fame68k_get_register(M68K_REG_A2);
		case M68K_A3:  return fame68k_get_register(M68K_REG_A3);
		case M68K_A4:  return fame68k_get_register(M68K_REG_A4);
		case M68K_A5:  return fame68k_get_register(M68K_REG_A5);
		case M68K_A6:  return fame68k_get_register(M68K_REG_A6);
		case M68K_SP:
		case M68K_A7:  return fame68k_get_register(M68K_REG_A7);
		case M68K_PREF_ADDR:  return 0; // ??
		case M68K_PREF_DATA:  return 0; // ??
#ifdef FAME_PREVIOUSPC
		case REG_PREVIOUSPC:
				      return fame68k_get_register(M68K_REG_PPC);
#else
		case REG_PREVIOUSPC:	/* fall throught */
#endif
		case M68K_PC:  return fame68k_get_pc();
		default:
			if( regnum < REG_SP_CONTENTS )
			{
				unsigned offset = fame68k_get_register(M68K_REG_A7) + 4 * (REG_SP_CONTENTS - regnum);
				if( offset < 0xfffffd )
					return cpu_readmem32_dword( offset );
			}
	}
	return 0;
}

void fame_set_reg(int regnum, unsigned val)
{
	switch( regnum )
	{
		case M68K_PC:  fame68k_set_register(M68K_REG_PC, val); break;
		case M68K_ISP:
		case M68K_USP: fame68k_set_register(M68K_REG_ASP, val); break;
		case M68K_SR:  fame68k_set_register(M68K_REG_SR, val); break;
		case M68K_D0:  fame68k_set_register(M68K_REG_D0, val); break;
		case M68K_D1:  fame68k_set_register(M68K_REG_D1, val); break;
		case M68K_D2:  fame68k_set_register(M68K_REG_D2, val); break;
		case M68K_D3:  fame68k_set_register(M68K_REG_D3, val); break;
		case M68K_D4:  fame68k_set_register(M68K_REG_D4, val); break;
		case M68K_D5:  fame68k_set_register(M68K_REG_D5, val); break;
		case M68K_D6:  fame68k_set_register(M68K_REG_D6, val); break;
		case M68K_D7:  fame68k_set_register(M68K_REG_D7, val); break;
		case M68K_A0:  fame68k_set_register(M68K_REG_A0, val); break;
		case M68K_A1:  fame68k_set_register(M68K_REG_A1, val); break;
		case M68K_A2:  fame68k_set_register(M68K_REG_A2, val); break;
		case M68K_A3:  fame68k_set_register(M68K_REG_A3, val); break;
		case M68K_A4:  fame68k_set_register(M68K_REG_A4, val); break;
		case M68K_A5:  fame68k_set_register(M68K_REG_A5, val); break;
		case M68K_A6:  fame68k_set_register(M68K_REG_A6, val); break;
		case M68K_SP:
		case M68K_A7:  fame68k_set_register(M68K_REG_A7, val); break;
		default:
			if( regnum < REG_SP_CONTENTS )
			{
				unsigned offset = fame68k_get_register(M68K_REG_A7) + 4 * (REG_SP_CONTENTS - regnum);
				if( offset < 0xfffffd )
					cpu_writemem32_dword( offset, val );
			}
	}
}


void fame_set_context(void *src)
{
	if (src)
	{
	    //if ( (*((unsigned *)src)) != ((unsigned)&fame68kcontext) )
	    {
		//fame68kcontext = *(FAME_CONTEXT*)src;	// fame68kcontext, this is same as memcpy
		memcpy(&fame68kcontext, src, sizeof(FAME_CONTEXT));
#ifndef FAME_USE_CONTEXT_SWITCH
		memcpy(&Fetch, src+sizeof(FAME_CONTEXT),  (sizeof(unsigned int *) * (1 << FAME_FETCHBITS)));
#endif
	    }

	}
}

unsigned fame_get_context(void *dst)
{
	if (dst)
	{
	    //*(FAME_CONTEXT*)dst = fame68kcontext;	// fame68kcontext, this is same as memcpy
	    memcpy(dst, &fame68kcontext, sizeof(FAME_CONTEXT));
#ifndef FAME_USE_CONTEXT_SWITCH
	    memcpy(dst+sizeof(FAME_CONTEXT), &Fetch,  (sizeof(unsigned int *) * (1 << FAME_FETCHBITS)));
#endif

	}
#ifdef FAME_USE_CONTEXT_SWITCH
	return sizeof(FAME_CONTEXT);
#else
	return sizeof(FAME_CONTEXT) + (sizeof(unsigned int *) * (1 << FAME_FETCHBITS));
#endif
}

void fame_set_nmi_line(int state)
{
	{
		if (state==CLEAR_LINE)
			fame68kcontext.interrupts[0] = 0;   // Clear all
		else
			fame68k_raise_irq(7,M68K_AUTOVECTORED_IRQ);
	}
}

void fame_set_irq_line(int irqline, int state)
{
	{
		if (state==CLEAR_LINE)
			fame68kcontext.interrupts[0] = 0;   // Clear all, seems to be required for some Atari games ie. RoadBlasters
			//fame68k_lower_irq(irqline);      // Some game will fail if you simply clear all. ie Birdie Try
		else
			fame68k_raise_irq(irqline,M68K_AUTOVECTORED_IRQ);
	}
}

int fame_execute(int cycles)
{
#ifdef DEBUG_FAME_SINGLE_STEP
	int cycles_to_end=cycles;
#endif
#ifdef DEBUG_FAME
	static unsigned slice=0;
	printf("fame_execute %i/%i (%i)\n",cpu_getactivecpu(),slice++,cycles);fflush(stdout);
#endif
#ifdef DEBUG_FAME_SINGLE_STEP
	fame68kcontext.cycles_counter=0;
	do{
		fame68kcontext.cycles_counter=0;
#ifdef DEBUG_FAME_FULL_START
		if (slice>DEBUG_FAME_FULL_START)
		{
			printf("\tPC=%.8X  OPCODE=%.4X %.4X %.4X %.4X SR=%.2X\n", fame68k_get_pc(), fame68k_fetch(fame68k_get_pc(),0),fame68k_fetch(fame68k_get_pc()+2,0),fame68k_fetch(fame68k_get_pc()+4,0),fame68k_fetch(fame68k_get_pc()+6,0),_68k_sreg);
			printf("A0=%.8X  A1=%.8X  A2=%.8X  A3=%.8X\n",_68k_areg(0),_68k_areg(1),_68k_areg(2),_68k_areg(3));
			printf("A4=%.8X  A5=%.8X  A6=%.8X  A7=%.8X\n",_68k_areg(4),_68k_areg(5),_68k_areg(6),_68k_areg(7));
			printf("D0=%.8X  D1=%.8X  D2=%.8X  D3=%.8X\n",_68k_dreg(0),_68k_dreg(1),_68k_dreg(2),_68k_dreg(3));
			printf("D4=%.8X  D5=%.8X  D6=%.8X  D7=%.8X\n",_68k_dreg(4),_68k_dreg(5),_68k_dreg(6),_68k_dreg(7));
		       	fflush(stdout);

		}
#endif
printf("\tcycles_to_end=%x PC=%.8X\n", cycles_to_end, fame68k_get_pc());
		fame68k_emulate(1);
		cycles_to_end-=fame68kcontext.cycles_counter;
	}while(cycles_to_end>0);
	printf("RETURN %i\n",cycles-cycles_to_end);
#ifdef DEBUG_FAME_FULL_STOP
	if (slice==DEBUG_FAME_FULL_STOP)
		exit(0);
#endif
	return cycles-cycles_to_end;
#else
	fame68kcontext.cycles_counter=0;
	fame68k_emulate(cycles);
	return fame68kcontext.cycles_counter;
#endif
}

void fame_set_irq_callback(int (*callback)(int irqline))
{
	fame68kcontext.iack_handler=(void (*)(unsigned int))callback;
}


const char *fame_info(void *context, int regnum)
{
	switch( regnum )
	{
		case CPU_INFO_NAME: return "FAME";
		case CPU_INFO_FAMILY: return "Motorola 68K";
		case CPU_INFO_VERSION: return "FAME 2.1";
		case CPU_INFO_FILE: return __FILE__;
		case CPU_INFO_CREDITS: return "Copyright (c) 2002-2005 Oscar Orallo Peláez / Daniel Lancha García.";
	}
	return "";
}

unsigned fame_dasm(char *buffer, unsigned pc)
{
	change_pc24(pc);
	sprintf( buffer, "$%04X", fame68k_fetch(pc,0));
	return 2;
}

#ifdef FAME_CHANGE_PC
void fame_change_pc(unsigned pc)
{
	change_pc24bew(pc);
}
#endif

#ifdef FAME_DEFAULT_CALLBACKS
void fame_default_irq_callback(unsigned int level)
{
    fame68kcontext.interrupts[0] = 0;   // Clear all after service interrupt
}
#endif
