#ident	"@(#)dbg.c	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/dbg.c	1.5"

/*
 * Intel 80386 Debugger
 */

#include "sys/types.h"
#include "sys/immu.h"
#include "sys/reg.h"
#include "sys/regset.h"
#include "sys/tss.h"
#include "sys/sysmsg.h"
#include "sys/cmn_err.h"
#include "sys/kdebugger.h"
#include "sys/xdebug.h"
#include "sys/inline.h"

extern gregset_t *regset[NREGSET];

/* Remote Console Support */
extern struct smsg_flags smsg_flags;
static int smsg_rcef;
static int rmcsan;

#define NULL    ((char *)0)
#define TRUE    (1==1)
#define FALSE   (1==0)
#define ESC     0x1b
#define	TAB	0x09
#define CR      0x0d
#define LF      0x0a
#define NAK     0x15
#define BS      0x08
#define DEL     0x7f

#define BYTE        1           /* display formats */
#define WORD        2
#define LONG        4

#define NUM_LINES   16          /* number of dumped lines   */
#define NUM_BYTES   16          /* number of dumped bytes   */

#define NUM_BRKPTS  16          /* max number of brk points */
#define NUM_WATCHPTS	4	/* max number of watch points */
#define	WP_BYTE		0x00	/* byte watchpoint */
#define	WP_WORD		0x01	/* word watchpoint */
#define	WP_LONG		0x03	/* long watchpoint */
#define	ACCESS		0x03	/* access watchpoint */
#define	MODIFY		0x01	/* modify watchpoint */
#define EXEC		0x00	/* execute watchpoint */
#define TRACE_SIMPLE	0x00	/* print whenever a location is touched */
#define TRACE_EQUAL	0x01	/* stop when location becomes equal to value */
#define TRACE_NOTEQ	0x02	/* stop when location becomes not eq to value */
#define NOWATCHPT	(unsigned char *)0xffffffff
#define BRKPT       0xcc	/* breakpoint instruction   */
#define TRACE       0x100       /* mask to set trace        */
#define NO_TRACE    ~(TRACE)    /* mask to unset trace      */

struct reg_info {
	char	*reg_name;
	short	index;
	short	type;
};

static struct reg_info	rinfo[] = {
	{ "EAX", EAX, 4 },
	{ "EBX", EBX, 4 },
	{ "ECX", ECX, 4 },
	{ "EDX", EDX, 4 },
	{ "ESI", ESI, 4 },
	{ "EDI", EDI, 4 },
	{ "EBP", EBP, 4 },
	{ "ESP", ESP, 4 },
	{ "CS", CS, 3 },
	{ "SS", SS, 3 },
	{ "DS", DS, 3 },
	{ "ES", ES, 3 },
	{ "FS", FS, 3 },
	{ "GS", GS, 3 },
	{ "EIP", EIP, 4 },
	{ "EFL", EFL, 4 },
	{ "AX", EAX, 3 },
	{ "AH", EAX, 2 },
	{ "AL", EAX, 1 },
	{ "BX", EBX, 3 },
	{ "BH", EBX, 2 },
	{ "BL", EBX, 1 },
	{ "CX", ECX, 3 },
	{ "CH", ECX, 2 },
	{ "CL", ECX, 1 },
	{ "DX", EDX, 3 },
	{ "DH", EDX, 2 },
	{ "DL", EDX, 1 },
	{ "SI", ESI, 3 },
	{ "DI", EDI, 3 },
	{ "BP", EBP, 3 },
	{ "SP", ESP, 3 },
	{ "IP", EIP, 3 },
	{ "FL", EFL, 3 },
	{ 0, 0, 0}
};

unsigned long	*gexp;	/* global pointer to exception stack */

static unsigned char   *buf_ptr; /* global pointer into command buffer */
static unsigned char   buf[80]; /* command buffer */

char *findsymname();
static long parse_addr();
static unsigned char *getname();

unsigned char * nmi_sav;

int	step_over,     /* stepping over brkpt flag  */
	format = LONG, /* display format -- BYTE, WORD, LONG */
	trace_count;   /* trace multiple instructions */


struct brk_tbl {
	unsigned char *    adr;
	unsigned char      instr;
} brk_tbl[NUM_BRKPTS];

struct watch_tbl {
	unsigned char *    adr;
	unsigned char      length;
	unsigned char      access;
	unsigned char	   trace;
	unsigned char	   type;
	unsigned long	   value;
} watch_tbl[NUM_WATCHPTS] = {
	{NOWATCHPT}, {NOWATCHPT}, {NOWATCHPT}, {NOWATCHPT}
};

/* exception types */
static char * ex_msg[] = {
	"DIV", "TRS", "NMI", "BKP", "OVF", "BND", "ILL", "DNA",
	"DBF", "CSO", "ITS", "SEG", "STK", "GPF", "PGF", "",
	"COE", "T17", "T18", "T19", "T20", "T21", "T22", "T23",
	"T24", "T25", "T26", "T27", "T28", "T29", "T30", "T31",
	"T32",
};
#define	NUM_EX_MSG	(sizeof(ex_msg)/sizeof(char *))

/* generic exception */
static char gen_msg[] = "EXC";
static char huh_msg[] = "?";
static char trace_msg[] = "Bad match, simple trace assumed\n";

void _gdebugger();
struct kdebugger dbg_kdb = {
	_gdebugger
};

void
dbg_init()
{
	kdb_register(&dbg_kdb);
}


void
dbg_tellsymname(name, addr, sym_addr)
	char	*name;
	ulong	addr, sym_addr;
{
	dbg_printf("%s", name);
	if (addr != sym_addr)
		dbg_printf("+%l", addr - sym_addr);
}


asm int
get_efl()
{
	pushfl
	popl	%eax
}


void
_gdebugger(why)
{
	int	brk_num;
	int	wp_num;
	char	cmd;
	char	match;
	unsigned long	*ex_frame;
	unsigned	save_efl;

	if (why == DR_INIT && (!debugger_init || *debugger_init == '\0'))
		return;

#if !DEBUG
	if (kdb_security && (why == DR_USER || why == DR_OTHER))
#else
	if (kdb_security && why == DR_USER)
#endif
		return;

	save_efl = get_efl();
	asm(" cli ");

	/* Remote Console Support
	 *
	 * Don't have the kernel debugger write to both
	 * the alternate and the remote console.
	 */
	if ((smsg_rcef = smsg_flags.rcef) != 0)
		smsg_flags.rcef = 0;
	rmcsan = rmcsantim(0);

	db_stacktrace(NULL, _gdebugger);
	ex_frame = (ulong *)(regset[0]);
	if (step_over && (ex_frame[EFL] & TRACE)) {
		step_over = FALSE;
		ex_frame[EFL] &= NO_TRACE;
		replace_brkpts();
		enable_watchpts();
		goto done;
	}
	if (((wp_num = get_watch_num()) >= 0) &&
	    (watch_tbl[wp_num].trace)) {
		disable_watchpts();
		switch(watch_tbl[wp_num].type) {
		case TRACE_SIMPLE:
			break;
		case TRACE_EQUAL:
			switch(watch_tbl[wp_num].length) {
			case WP_BYTE:
				if((*(unsigned char *)watch_tbl[wp_num].adr ==
					(unsigned char)watch_tbl[wp_num].value)
				   || (ex_frame[EFL] & TRACE))
					break;
				enable_watchpts();
				goto done;
			case WP_WORD:
				if((*(unsigned short *)watch_tbl[wp_num].adr ==
					(unsigned short)watch_tbl[wp_num].value)
				   || (ex_frame[EFL] & TRACE))
					break;
				enable_watchpts();
				goto done;
			case WP_LONG:
				if((*(unsigned long *)watch_tbl[wp_num].adr ==
					(unsigned long)watch_tbl[wp_num].value)
				   || (ex_frame[EFL] & TRACE))
					break;
				enable_watchpts();
				goto done;
			}
			break;
		case TRACE_NOTEQ:
			switch(watch_tbl[wp_num].length) {
			case WP_BYTE:
				if((*(unsigned char *)watch_tbl[wp_num].adr !=
					(unsigned char)watch_tbl[wp_num].value)
				   || (ex_frame[EFL] & TRACE))
					break;
				enable_watchpts();
				goto done;
			case WP_WORD:
				if((*(unsigned short *)watch_tbl[wp_num].adr !=
					(unsigned short)watch_tbl[wp_num].value)
				   || (ex_frame[EFL] & TRACE))
					break;
				enable_watchpts();
				goto done;
			case WP_LONG:
				if((*(unsigned long *)watch_tbl[wp_num].adr !=
					(unsigned long)watch_tbl[wp_num].value)
				   || (ex_frame[EFL] & TRACE))
					break;
				enable_watchpts();
				goto done;
			}
			break;
		}
		if(watch_tbl[wp_num].access == EXEC) {
			dbg_printf("trace %b: at %l", 
				wp_num, ex_frame[EIP]);
		}
		else {
			dbg_printf("trace %b: %l <=", 
				wp_num, watch_tbl[wp_num].adr);
			dbg_printf("%l", 
				*(unsigned long *)watch_tbl[wp_num].adr);
			dbg_printf(" at %l", ex_frame[EIP]);
		}
		/* Clear the watch point (among other things) */
		enable_watchpts();
		if(watch_tbl[wp_num].type == TRACE_SIMPLE) {
			dbg_printf("\n");
			if(watch_tbl[wp_num].access == EXEC) {
				step_over = 1;
				ex_frame[EFL] |= TRACE;
				disable_watchpts();
				remove_brkpts();
			}
			goto done;
		}
	}
	gexp = ex_frame;
	if (ex_frame[TRAPNO] >= NUM_EX_MSG)
		dbg_printf("\n%s ", gen_msg);
	else
		dbg_printf("\n%s ", ex_msg[ex_frame[TRAPNO]]);
	if (((ex_frame[EFL] & TRACE) == 0) &&
	    ((brk_num = find_brkpt(ex_frame[EIP] - 1)) != -1)) {
		ex_frame[EIP] -= 1;
		dbg_printf("brkpt %b ", brk_num);
	}
	ex_frame[EFL] &= NO_TRACE;      /* could have been single step */
/*
 * Fix ESP to point to user stack, not exception stuff.
 * It is ignored by the popa instruction on return anyway.
 */
	ex_frame[ESP] += 5 * sizeof(unsigned long);
/*
 * This next test depends on the watch point being cleared by now if
 * it was a trace.
 */
	if ((wp_num = get_watch_num()) >= 0) {
		dbg_printf("wtchpt %b at ", wp_num);
		findsymname(watch_tbl[wp_num].adr, dbg_tellsymname);
		dbg_printf("\n");
	}
	dbg_printf("debugger entered from ");
	findsymname(ex_frame[EIP], dbg_tellsymname);
	dbg_printf("\n");
	remove_brkpts();
	disable_watchpts();
	prn_regs(ex_frame);
	while (TRUE) {
		while (dbg_getcharnowait() != -1);  /* flush uart buf */
		dbg_printf("d:"); /* display prompt */
		get_line(TRUE); /* get command line */
		cmd = *buf_ptr++;
		switch (cmd) {
		case '\0':
			break;
		case '?': /* display break points */
			dspbrkpts();
			break;
		case 'A': /* set access watch point */
		case 'a':
			set_watchpt(ACCESS);
			break;
		case 'B': /* set a breakpoint */
		case 'b':
			insertbkpt(ex_frame[EIP]);
			break;
		case 'C': /* clear a watch point */
		case 'c':
			clear_watchpt();
			break;
		case 'D': /* display memory contents */
		case 'd':
			dump();
			break;
		case 'E': /* examine/alter memory location */
		case 'e':
		case 'W': /* write to memory location */
		case 'w':
			examine(to_upper(cmd) == 'W');
			break;
		case 'F': /* set display format */
		case 'f':
			set_format();
			break;
		case 'G': /* continue from breakpoint */
		case 'g':
			if (*buf_ptr != '\0')
				ex_frame[EIP] = get_adr();
			if (find_brkpt(ex_frame[EIP]) != -1) {
				step_over = 1;
				ex_frame[EFL] |= TRACE;
			} else
				replace_brkpts();
			enable_watchpts();
			goto done;
		case 'H': /* print a list of debugger commands */
		case 'h':
			dbg_help();
			break;
		case 'I': /* do an inb or inw or inl */
		case 'i':
			do_in();
			break;
		case 'K': /* kill a breakpoint */
		case 'k':
			rmbkpt();
			break;
		case 'L': /* Lookup an address and return symbol name */
		case 'l':
			sym_lookup();
			break;
		case 'M': /* set a modify watch point */
		case 'm':
			set_watchpt(MODIFY);
			break;
		case 'N': /* switch to new terminal/debugger */
		case 'n':
			switch_new();
			break;
		case 'O': /* do an outb or outw or outl */
		case 'o':
			do_out();
			break;
		case 'R': /* examine/display registers */
		case 'r':
			ex_regs(ex_frame);
			break;
		case ESC:
		case 'S': /* single step */
		case 's':
			ex_frame[EFL] |= TRACE;
			goto done;
		case 'T': /* trace location */
		case 't':
			set_tracept(ACCESS);
			break;
		case 'X': /* trace execution */
		case 'x':
			set_tracept(EXEC);
			break;
		case 'Z': /* dump tss */
		case 'z':
			dump_tss();
			break;
		default:
			dbg_printf("%s\n", huh_msg);
			break;
		}
	}
done:
	/* Remote Console Support */
	smsg_flags.rcef = smsg_rcef;	/* restore output flag */
	rmcsantim(rmcsan);

	if (save_efl & PS_IE)
		asm(" sti ");
}

/*
 * debugger help routine
 */
dbg_help()
{
	dbg_printf("debugger commands:\n");
	dbg_printf("a{b|w|l} <address>        ");
	dbg_printf(" - Access watch point at address\n");
	dbg_printf("b <address>               ");
	dbg_printf(" - Set breakpoint at address\n");
	dbg_printf("c <watchpt_num>           ");
	dbg_printf(" - Clear given watch point (* for all)\n");
	dbg_printf("d [address]               ");
	dbg_printf(" - Dump memory in current format\n");
	dbg_printf("e <address>               ");
	dbg_printf(" - Examine/open a memory location\n");
	dbg_printf("f{b|w|l}                  ");
	dbg_printf(" - Set display format\n");
	dbg_printf("g [address]               ");
	dbg_printf(" - Continue execution at address\n");
	dbg_printf("h                         ");
	dbg_printf(" - Print this list\n");
	dbg_printf("i <address>               ");
	dbg_printf(" - Do an in instruction at address\n");
	dbg_printf("k <brkpt_num>             ");
	dbg_printf(" - Kill given breakpoint (* for all)\n");
	dbg_printf("l <address>               ");
	dbg_printf(" - Print nearest symbol before address\n");
	dbg_printf("m{b|w|l} <address>        ");
	dbg_printf(" - Modify watch point at address\n");
	dbg_printf("n{t|d}                    ");
	dbg_printf(" - Switch to new terminal|debugger\n");
	dbg_printf("o <address> <value>       ");
	dbg_printf(" - Do an out instruction at address\n");
	dbg_printf("r [%<reg> <value>]        ");
	dbg_printf(" - Set/display registers\n");
	dbg_printf("s                         ");
	dbg_printf(" - Single step (also ESC)\n");
	dbg_printf("t <address> [{=|!=} <val>]");
	dbg_printf(" - Set trace point at address\n");
	dbg_printf("w <address>               ");
	dbg_printf(" - Write to memory without read\n");
	dbg_printf("x <address>               ");
	dbg_printf(" - Set execution trace point at address\n");
	dbg_printf("z                         ");
	dbg_printf(" - Dump some values from the TSS\n");
	dbg_printf("?                         ");
	dbg_printf(" - Display watch/break points\n");
	dbg_printf("note: <address> can be symbolic name or hex value\n");
}

asm
real_cr3()
{
	movl	%cr3, %eax
}

dump_tss()
{
	dbg_printf("CR0: %l\n", _cr0());
	dbg_printf("CR2: %l\n", _cr2());
	dbg_printf("CR3: %l\n", real_cr3());
}


/*
 * display break/watch points
 */

dspbrkpts() {
	register int i;

	dbg_printf("brkpts:\n");
	for (i = 0; i < NUM_BRKPTS; i++)
		if (brk_tbl[i].adr != 0) {
			dbg_printf("%b at ", i);
			dbg_printf("%l ", brk_tbl[i].adr);
			findsymname(brk_tbl[i].adr, dbg_tellsymname);
			dbg_printf("\n");
		}
	dbg_printf("wtchpts:\n");
	for (i = 0; i < NUM_WATCHPTS; i++)
		if (watch_tbl[i].adr != NOWATCHPT) {
			dbg_printf("%b at ", i);
			dbg_printf("%l ", watch_tbl[i].adr);
			findsymname(watch_tbl[i].adr, dbg_tellsymname);
			dbg_printf(" ");
			if(watch_tbl[i].trace) {
				dbg_printf("trace");
				switch(watch_tbl[i].type) {
				case TRACE_SIMPLE:
					dbg_printf("\n");
					break;
				case TRACE_EQUAL:
					dbg_printf(" = %l\n",
						   watch_tbl[i].value);
					break;
				case TRACE_NOTEQ:
					dbg_printf(" != %l\n",
						   watch_tbl[i].value);
					break;
				}
			}
			else {
				switch (watch_tbl[i].length) {
				case WP_BYTE:
					dbg_printf("byte ");
					break;
				case WP_WORD:
					dbg_printf("word ");
					break;
				case WP_LONG:
					dbg_printf("long ");
					break;
				}
				switch (watch_tbl[i].access) {
				case ACCESS:
					dbg_printf("access\n");
					break;
				case MODIFY:
					dbg_printf("modify\n");
					break;
				}
			}
		}
}

/*
 * lookup an address
 */
sym_lookup()
{
	long	adr;

	adr = get_adr();
	findsymname(adr, dbg_tellsymname);
	dbg_printf("\n");
}


/*
 * dump memory
 */
static unsigned char * dump_sav;

static
dump() {
	register int i, j;
	unsigned char c;

	if (buf[1] != '\0')         /* is there dump address */
		dump_sav = (unsigned char *) get_adr(&buf[1]);
#ifdef notdef
/* only fails on 68000 series processors */
	if (!(format & 0x1))        /* if format word or long keep it even */
		if (((unsigned int) dump_sav) & 0x1)
			dump_sav -= 1;
#endif /*notdef*/
	for (i = 0; i < NUM_LINES; i++) {
		dbg_printf("%l: ", dump_sav);

		for (j = 0; j < NUM_BYTES; j += format) {
			switch (format) {
			case BYTE:
				dbg_printf("%b", *dump_sav);
				break;
			case WORD:
				dbg_printf("%w", *((unsigned short *) dump_sav));
				break;
			case LONG:
				dbg_printf("%l", *((unsigned int *) dump_sav));
				break;
			}
			dump_sav += format;
			dbg_printf(" ");
		}
		dump_sav -= NUM_BYTES;
		dbg_printf("  |");                      /* do ascii part */
		for (j = 0; j < NUM_BYTES; j++) {
			c = *dump_sav++;
			if ((c > 0x1f) && (c < 0x7f))
				dbg_putchar(c);
			else
				dbg_printf(".");
		}
		dbg_printf("|\n");
	}
}

/*
 *      examine and/or change a memory location
 */

static unsigned int   exam_sav;

examine(write_only)
	int	write_only;
{
	register unsigned long	adr,
				value = 0;
	register                char c;

	if (*buf_ptr == '\0')
		exam_sav += format;
	else
		exam_sav = get_adr();
	adr = exam_sav;
	while (TRUE) {
		dbg_printf("%l = ", adr);
		if (!write_only) {
			switch (format) {
			case BYTE:
				dbg_printf("%b", *((unsigned char *) adr));
				break;
			case WORD:
				dbg_printf("%w", *((unsigned short *) adr));
				break;
			case LONG:
				dbg_printf("%l", *((unsigned int *) adr));
				break;
			}
			dbg_printf(": ");
		}
		get_line(TRUE);
		c = *buf_ptr;
sw1:
		switch (c) {
		case '\0':
			return;
		case '+':
		case LF:
			exam_sav += format;
			adr = exam_sav;
			dbg_printf("\n");
			break;
		case '-':
			exam_sav -= format;
			adr = exam_sav;
			dbg_printf("\n");
			break;
		case '@':
			exam_sav = *((unsigned int *) exam_sav);
			adr = exam_sav;
			dbg_printf("\n");
			break;
		case 'l':
		case 'L':
			if(!write_only) {
				findsymname(*(ulong *)adr, dbg_tellsymname);
				dbg_printf("\n");
			}
			else
				dbg_printf("?\n");
			break;
		case 'b':
		case 'B':
			if(!write_only) {
				insertbkpt(*(unsigned long *)adr);
			}
			else
				dbg_printf("?\n");
			break;
		default:
			if (!is_hex_char(c)) {
				dbg_printf(huh_msg);
				return;
			}
			value = get_adr();
			switch (format) {
			case BYTE:
				*((unsigned char *)adr) = value;
				break;
			case WORD:
				*((unsigned short *)adr) = value;
				break;
			case LONG:
				*((unsigned int *)adr) = value;
				break;
			}
			c = *buf_ptr++;
			goto sw1;
		}
	}
}

/*
 *      set the format
 */
set_format() {
	skipblanks();
	switch (*buf_ptr++) {
	case 'B':
	case 'b':
		format = BYTE;
		dbg_printf("BYTE\n");
		break;
	case 'W':
	case 'w':
		format = WORD;
		dbg_printf("WORD\n");
		break;
	case 'L':
	case 'l':
		format = LONG;
		dbg_printf("LONG\n");
		break;
	}
}

/*
 *      switch to new terminal/debugger
 */
switch_new() {
	skipblanks();
	switch (*buf_ptr++) {
	case 'T':	/* new terminal */
	case 't':
		kdb_next_io();
		break;
	case 'D':
	case 'd':
		kdb_next_debugger();
		break;
	}
}

/*
 * do an in instruction from given I/O port.
 */
do_in()
{
	unsigned long	inaddr;

	inaddr = (unsigned long)get_adr();
	switch (format) {
	case BYTE:
		dbg_printf(" %b\n", inb(inaddr));
		break;
	case WORD:
		dbg_printf(" %w\n", inw(inaddr));
		break;
	case LONG:
		dbg_printf(" %l\n", inl(inaddr));
		break;
	}	
}

/*
 * do an out instruction to given I/O port.
 */
do_out()
{
	unsigned long	outaddr;

	outaddr = (unsigned long)get_adr();
	switch (format) {
	case BYTE:
		outb(outaddr, get_adr());
		break;
	case WORD:
		outw(outaddr, get_adr());
		break;
	case LONG:
		outl(outaddr, get_adr());
		break;
	}	
}

/*
 *      insert a break point
 */

insertbkpt(eip)
	unsigned char	*eip;
{
	register int i;
	register struct brk_tbl * tblptr = brk_tbl;

	for (i = 0; i < NUM_BRKPTS; i++) {  /* find an open slot */
		if (tblptr->adr == 0)
			break;
		tblptr++;
	}
	if (i >= NUM_BRKPTS) {              /* all slots filled */
		dbg_printf("breakpoint table is full\n");
		return;
	}
	if (*buf_ptr != '\0') /* put it in table  */
		tblptr->adr = (unsigned char *)get_adr();
	else
		tblptr->adr = eip;
}

/*
 *      remove one or all break points
 */

rmbkpt() {
	register int i;

	skipblanks();
	if (*buf_ptr == '*')
		for (i = 0; i < NUM_BRKPTS; i++)
			brk_tbl[i].adr = 0;
	else {
		i = str_to_hex(buf_ptr);
		brk_tbl[i].adr = 0;
	}
}

/*
 * watch point routines 
 */

get_watch_num()
{
	int	num;

	num = _dr6() & 0xf;
	if(num & 0x8)
		if(watch_tbl[3].adr != NOWATCHPT)
			return(3);
	if(num & 0x4)
		if(watch_tbl[2].adr != NOWATCHPT)
			return(2);
	if(num & 0x2)
		if(watch_tbl[1].adr != NOWATCHPT)
			return(1);
	if(num & 0x1)
		if(watch_tbl[0].adr != NOWATCHPT)
			return(0);
	return(-1);
}

disable_watchpts()
{
	_wdr7(0x400);
}

enable_watchpts()
{
	unsigned long inst_mask = 0x300;
	int	i;

	for(i=0; i<NUM_WATCHPTS; i++) {
		if(watch_tbl[i].adr != NOWATCHPT) {
			inst_mask |= (watch_tbl[i].length << (18+(4*i)));
			inst_mask |= (watch_tbl[i].access << (16+(i*4)));
			inst_mask |= (0x3 << (i*2));
		}
	}
	_wdr7(inst_mask);
	i = _dr6() & 0xFFFFFFF0;
	_wdr6(i);
}

set_watchreg(i, adr)
int	i;
unsigned char *adr;
{
	switch(i) {
	case 0:
		_wdr0(adr);
		break;
	case 1:
		_wdr1(adr);
		break;
	case 2:
		_wdr2(adr);
		break;
	case 3:
		_wdr3(adr);
		break;
	}
}

set_tracept(access)
int	access;
{
	register int i, l;

	for (i = 0; i < NUM_WATCHPTS; i++) {  /* find an open slot */
		if (watch_tbl[i].adr == NOWATCHPT)
			break;
	}
	if (i >= NUM_WATCHPTS) { /* all slots filled */
		dbg_printf("watch point table is full\n");
		return;
	}
	if (*buf_ptr != '\0') { /* put it in table  */
		watch_tbl[i].adr = (unsigned char *)get_adr();
		switch(format) {
		case BYTE:
			watch_tbl[i].length = WP_BYTE;
			break;
		case WORD:
			watch_tbl[i].length = WP_WORD;
			break;
		case LONG:
			watch_tbl[i].length = WP_LONG;
			break;
		}
		watch_tbl[i].trace = 1;
		watch_tbl[i].access = access;
		watch_tbl[i].type = TRACE_SIMPLE;
		skipblanks();
		if(*buf_ptr != '\0' && access != EXEC) {
			switch(*buf_ptr++) {
			case '=':
				watch_tbl[i].value = get_adr();
				watch_tbl[i].type = TRACE_EQUAL;
				break;
			case '!':
				if(*buf_ptr++ != '=') {
					dbg_printf(trace_msg);
					break;
				}
				watch_tbl[i].value = get_adr();
				watch_tbl[i].type = TRACE_NOTEQ;
				break;
			default:
				dbg_printf(trace_msg);
				break;
			}
		}
		if(access == EXEC)
			watch_tbl[i].length = WP_BYTE;
		set_watchreg(i, watch_tbl[i].adr);
	}
}

set_watchpt(access)
{
	register int i, l;

	for (i = 0; i < NUM_WATCHPTS; i++) {  /* find an open slot */
		if (watch_tbl[i].adr == NOWATCHPT)
			break;
	}
	if (i >= NUM_WATCHPTS) { /* all slots filled */
		dbg_printf("watch point table is full\n");
		return;
	}
	skipblanks();
	switch(*buf_ptr) { /* get watchpoint length */
	case 'L':
	case 'l':
		l = WP_LONG;
		buf_ptr++;
		break;
	case 'W':
	case 'w':
		l = WP_WORD;
		buf_ptr++;
		break;
	case 'B':
	case 'b':
		l = WP_BYTE;
		buf_ptr++;
		break;
	default:
		l = WP_LONG;
		break;
	}
	if (*buf_ptr != '\0') { /* put it in table  */
		watch_tbl[i].adr = (unsigned char *)get_adr();
		watch_tbl[i].length = l;
		watch_tbl[i].trace = 0;
		watch_tbl[i].access = access;
		set_watchreg(i, watch_tbl[i].adr);
	}
}

clear_watchpt() {
	register int i;

	skipblanks();
	if (*buf_ptr == '*')
		for (i = 0; i < NUM_WATCHPTS; i++)
			watch_tbl[i].adr = NOWATCHPT;
	else {
		i = str_to_hex(buf_ptr);
		watch_tbl[i].adr = NOWATCHPT;
	}
}

/*
 * open a register
 */
ex_regs(exp)
	unsigned long	*exp;
{
	unsigned char	*name;
	unsigned long	value;
	int		i;

	skipblanks();
	if (*buf_ptr++ == '%') {
		name = getname();
		for (i = 0; rinfo[i].reg_name != NULL; i++)
			if (str_match(name, rinfo[i].reg_name))
				break;
		if (rinfo[i].reg_name == NULL) {
			dbg_printf(huh_msg);
			return;
		}
		switch (rinfo[i].index) {
		case ESP:
		case CS:
		case SS:
		case DS:
		case ES:
		case FS:
		case GS:
			dbg_printf("not allowed to change reg %s\n",
				rinfo[i].reg_name);
			return;
		default:
			break;
		}
		skipblanks();
		value = get_adr();
		switch (rinfo[i].type) {
		case 4: /* change a long register */
			exp[rinfo[i].index] = value;
			break;
		case 3: /* change 16 bit register */
			exp[rinfo[i].index] = (exp[rinfo[i].index] &
				0xffff0000) | (value & 0x0000ffff);
			break;
		case 2: /* change a hi 8 bit register */
			exp[rinfo[i].index] = (exp[rinfo[i].index] &
				0xffff00ff) | ((value & 0x000000ff) << 8);
			break;
		case 1: /* change a lo 8 bit register */
			exp[rinfo[i].index] = (exp[rinfo[i].index] &
				0xffffff00) | (value & 0x000000ff);
			break;
		default:    /* illegal command */
			dbg_printf("%s", huh_msg);
		}
	}
	prn_regs(exp);
}

/*
 *      prn_regs() -- display the registers
 */

prn_regs(exp)
	unsigned long	*exp;
{
	register int i;

	dbg_printf("    "); /* header */
	for (i = 0; i < 8; i++) {
		dbg_printf("%s      ", rinfo[i].reg_name);
	}
	/* general purpose regs */
	dbg_printf("\n    ");
	for (i = 0; i < 8; i++)
		dbg_printf("%l ", exp[rinfo[i].index]);
	/* other registers */
	dbg_printf("\n    ");
	for (i = 8; i < 14; i++) {
		dbg_printf("%s       ", rinfo[i].reg_name);
	}
	dbg_printf("EIP      EFL");
	dbg_printf("\n    ");
	for (i = 8; i < 16; i++)
		dbg_printf("%l ", exp[rinfo[i].index]);
	dbg_printf("\n");
}

/*
 *      break point routines
 */

replace_brkpts()
{
	register int i;
	register struct brk_tbl * tblptr = brk_tbl;

	for (i = 0; i < NUM_BRKPTS; i++) {
		if (tblptr->adr != 0) {
			tblptr->instr = *tblptr->adr;
			*tblptr->adr = BRKPT;
		}
		tblptr++;
	}
}

find_brkpt(address)
	register unsigned char * address;
{
	register int i;
	register struct brk_tbl * tblptr = brk_tbl;

	for (i = 0; i < NUM_BRKPTS; i++)
		if ((tblptr++)->adr == address)
			return(i);
	return(-1);
}


remove_brkpts() {
	register int    i;
	register struct brk_tbl  *tblptr = brk_tbl;

	for (i = 0; i < NUM_BRKPTS; i++) {
		if (tblptr->adr != 0)
			*tblptr->adr = tblptr->instr;
		tblptr++;
	}
}

/*
 * utility subroutines
 */

str_to_hex(str)
	register unsigned char  *str;
{
	register long           num = 0;
	register unsigned char  c;

	for(;;) {
		c = *str++;
		if ((c >= '0') && (c <= '9'))
			num = (num << 4) + (c - '0');
		else if ((c >= 'A') && (c <= 'F'))
			num = (num << 4) + (c - 'A' + 10);
		else if ((c >= 'a') && (c <= 'f'))
			num = (num << 4) + (c - 'a' + 10);
		else
			return(num);
	}
}

/*
 * Take the next thing in the command buffer pointed to by buf_ptr and 
 * interpret it as an address, and return the resulting address.
 */
get_adr()
{
	unsigned char	*name;
	int		i;
	register int ret;

	skipblanks();
	switch (*buf_ptr) {
	case '\0':
		ret = 0;
		break;
	case '%':
		buf_ptr++;
		name = getname();
		for (i = 0; rinfo[i].reg_name != NULL; i++)
			if (str_match(name, rinfo[i].reg_name))
				break;
		if (rinfo[i].reg_name == NULL || rinfo[i].type != 4) {
			dbg_printf("bad reg %s, %eax assumed.\n", name);
			ret = gexp[EAX];
			break;
		}
		ret = gexp[rinfo[i].index];
		break;
	case '@':
		buf_ptr++;
		ret = *((int *)get_adr());
		break;
	default:
		ret = parse_addr();
		break;
	}
	return(ret);
}

/*
 * get a debugger input line.
 * if the specdelim flag is true then
 * the line will be terminated by LF, +, -, ESC, or @ in addition to CR.
 */
get_line(specdelim)
{
	register unsigned char * bufptr = buf;
	register int stop = 0;
	register unsigned char   c;

	buf_ptr = buf;
	for (;;) {
		c = dbg_getchar() & 0x7f;
		switch (c) {
		case CR:
			dbg_putchar(LF);
			*bufptr = '\0';
			return;
		case '+':
		case '-':
		case '@':
			if (!specdelim)
				goto nospec;
			dbg_putchar(c);
			/* fall thru here */
		case LF:
		case ESC:
			if (!specdelim)
				goto nospec;
			*bufptr++ = c;
			*bufptr = '\0';
			return;
		case DEL:
		case BS:
			if (stop > 0) {
				bufptr--;
				stop--;
				dbg_putchar(BS);
				dbg_putchar(' ');
				dbg_putchar(BS);
			}
			break;
		case NAK:
			while (stop > 0) {
				bufptr--;
				stop--;
				dbg_putchar(BS);
				dbg_putchar(' ');
				dbg_putchar(BS);
			}
			break;
		default :
nospec:
			if (c >= 0x20) {
				dbg_putchar(c);
				*bufptr++ = c;
				stop++;
			}
			break;
		}
	}
}

/*
 * parse a debugger address, we collect the token then
 * first try to parse it as a hex constant, if that
 * fails, then we try it as a symbol name and look it up
 * in the kernel symbol table.  if that fails we print an error
 * message and return 0.  Preceding a symbolic name with a 
 * double quote " will force its evaluation as a symbol, even
 * if the name would be a legal hex constant.
 */

static unsigned char	pbuf[64];

static long
parse_addr()
{
	long		ret;
	int		ishex;
	unsigned char	*p;
	unsigned char	c;

	p = pbuf;
	if (*buf_ptr == '"') {
		ishex = FALSE;
		buf_ptr++;
	} else
		ishex = TRUE;
	while (is_char(c = *buf_ptr)) { /* collect the token */
		if (!is_hex_char(c))
			ishex = FALSE;
		*p++ = *buf_ptr++;
	}
	*p = '\0';
	if (ishex)
		return str_to_hex(pbuf);
	else
		if ((ret = findsymaddr(pbuf)) == 0) {
			dbg_printf("no such symbol %s, %l assumed.\n",
					pbuf, KVBASE);
			ret = KVBASE;
		}
	return ret;
}

static unsigned char	namebuf[64];

static unsigned char *
getname()
{
	unsigned char	*namep;

	namep = namebuf;
	while (is_char(*buf_ptr)) /* collect the token */
		*namep++ = *buf_ptr++;
	*namep = '\0';
	return namebuf;
}


str_match(p1, p2)
	unsigned char	*p1;
	unsigned char	*p2;
{
	while (*p1 && *p2 && (to_upper(*p1) == to_upper(*p2))) {
		p1++;
		p2++;
	}
	if (*p1 || *p2)
		return FALSE;
	else
		return TRUE;
}

skipblanks()
{
	while (*buf_ptr == ' ' || *buf_ptr == TAB)
		buf_ptr++;
}

is_char(c)
	unsigned char	c;
{
	return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') || c == '_');
}

is_hex_char(c)
	unsigned char	c;
{
	return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
		(c >= 'a' && c <= 'f'));
}

to_upper(c)
	unsigned char	c;
{
	if (('a' <= c) && (c <= 'z'))
		c -= ('a' - 'A');
	return(c);
}


dbg_printf(cstr, a0)
char *cstr;    /* control string */
int a0;        /* first argument */
{
	int *ap = &a0;  /* argument pointer */
	char *tmpstr;   /* temporary for %s format */
	char cc, sc;    /* next control character */

	while ((cc = *cstr++) != (char) 0) {

		switch (cc) {

		default:
			dbg_putchar(cc);
			break;

		case '%':
			switch(cc = *cstr++) {

				/* formats this dbg_printf understands */
			case 's':
				/* string format */
				tmpstr = (char *)*ap++;
				while ((sc = *tmpstr++) != (char) 0)
				{
					if (sc == '\n')
						dbg_putchar('\r');
					dbg_putchar(sc);
				}
				break;

			case 'b':
				hexadecimal(*ap, 2, '0');
				ap++;
				break;
			case 'w':
				hexadecimal(*ap, 4, '0');
				ap++;
				break;
			case 'l':
				hexadecimal(*ap, 8, '0');
				ap++;
				break;
			case 'L':
				hexadecimal(*ap, 8, ' ');
				ap++;
				break;

				/* formats that dbg_printf does not understand */
			case '\0':
				/* % was end of string, return */
				return;
			default:
				/* Bad format, just parrot control string */
				dbg_putchar('%');
				dbg_putchar(cc);
				break;
			}
		}
	}
}


hexadecimal(number, places, preceed)
int number;    /* Number to format */
int places;    /* width of format field */
char preceed;  /* preceeding 0s/spaces */
{
	int i,digitfound,shift;
	unsigned char digit;

	digitfound = 0;
	for(i=0; i<places; i++) {
		shift = ((places - 1)-i) * 4;
		digit = (number >> shift) & 0xf;
		if( digit == 0 && !digitfound && i!=(places - 1))
			dbg_putchar(preceed);
		else {
			digitfound++;
			dbg_putchar((digit < 10) ? (digit+'0') : (digit+'a'-10));
		}
	}
}

dbg_putchar(c)
register int    c;
{
/*
 * we expect the putchar routine to add a cr to
 * the output if the char is \n.
 */
	DBG_PUTCHAR(c);
}

dbg_getchar()
{
	register int    c;

	while ((c = DBG_GETCHAR()) == -1) ;

	return c;
}

dbg_getcharnowait()
{
	return DBG_GETCHAR();
}
