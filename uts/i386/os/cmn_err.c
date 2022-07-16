/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:cmn_err.c	1.3.1.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/time.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/vnode.h"
#include "sys/resource.h"
#include "sys/cred.h"
#include "sys/proc.h"
#include "sys/exec.h"
#include "sys/tty.h"
#include "sys/reg.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/seg.h"
#include "sys/inline.h"
#include "sys/bootinfo.h"
#include "sys/xdebug.h"
#include "sys/iobuf.h"
#include "sys/strlog.h"
#include "sys/debug.h"

/* general argument list for cmn_err and panic */

# define ARGS	a0,a1,a2,a3,a4,a5

/*	A delay is required when outputting many
**	lines to the console if it is a dmd 5620
**	terminal.  The following value has been
**	chosen empirically.  If your console is
**	a normal terminal, set the delay to 0.
**	Note that dmd_delay can be set to 0
**	on the running system with demon.
*/

#define	DMD_DELAY	0x0

STATIC void printn();

STATIC int panic_level = 0;
STATIC int dmd_delay = DMD_DELAY;

void
dodmddelay()
{
	int	delay;

	if (dmd_delay) {
		delay = dmd_delay;
		while (delay--) ;
	}
}

/*	Save output in a buffer where we can look at it
**	with demon or crash.  If the message begins with
**	a '!', then only put it in the buffer, not out
**	to the console.
*/

extern	char	putbuf[];
extern	int	putbufsz;
int		putbufndx=0;
extern short	prt_where;		/* defined in startup.c */

/* prt_flag is set in main.c when we know that (for instance) the PIC and spl
** tables are initialized.  Until it gets set, it is disastrous to call any
** spl function except splhi().  There is at least one path that gets to one
** of the spl functions, and this causes an EXTOVRFLT.  We prevent this by
** *not* doing console i/o until after prt_flag is set (except in the event of
** a panic, in which case we flush putbuf to the display, and continue the
** panic).
**
** When prt_flag is not set (i.e., it is not OK to write to the console),
** things are done differently.
**	'\0' will appear at the end of messages in putbuf (so flush_putbuf()
**		can re-parse the leading `!' or `^' from cmn_err() messages;
**		this may confuse crash, but when flush_putbuf() is called,
**		putbuf will be rewritten in place the way crash expects to
**		find it);
**	all messages passed to either printf() or cmn_err() are stored in
**		putbuf[] *with* the leading `!' or `^' (even if overridden by
**		prt_where); this is only true until flush_putbuf() is called.
*/
short		prt_flag = 0;
short		old_prt_flag = 0;

#define	output(c) \
	if ((!old_prt_flag) || (prt_where & PRW_BUF)) { \
		putbuf[putbufndx++ % putbufsz] = c; \
		if (old_prt_flag) \
			wakeup(putbuf); \
	} \
	if (prt_where & PRW_CONS) \
		if (old_prt_flag) \
			putchar(c);

#ifdef LAYERS
#define	loutput(c, tp) \
	if ((!old_prt_flag) || (prt_where & PRW_BUF)) { \
		putbuf[putbufndx++ % putbufsz] = c; \
		if (old_prt_flag) \
			wakeup(putbuf); \
	} \
	if (prt_where & PRW_CONS) \
		if (old_prt_flag) \
			putc(c, &tp->t_outq);
#endif

char	*panicstr ;

extern int	*save_r0ptr ;
extern int	sdata[];
extern int	bssSIZE[];

/* This routine is intended to be called exactly once.  It may be called from
** main() in the normal case, after the system is almost fully initialized.
** Or, it may be called just before processing a panic.  In either case, it
** displays the contents of putbuf[] as though the output was generated at the
** calls to either printf() or cmn_err().  It correctly fails to display
** strings marked with the `!' (as cmn_err() would).  Because it uses the
** output() macro (after setting putbufndx to 0), putbuf[] will look fine to
** crash and related friends.
*/

flush_putbuf()
{
	register int oldndx = putbufndx;
	register int i;

	prt_where = PRW_CONS;
	putbufndx = 0;
	for (i = 0; i < oldndx; ++i)
	{
		switch (putbuf[i])
		{
		case '^':
			++i;
			prt_where = PRW_CONS;
			break;
		case '!':
			++i;
			prt_where = PRW_BUF;
			break;
		default:
			prt_where = PRW_BUF | PRW_CONS;
			break;
		}
		while (i < oldndx && putbuf[i])
		{
			output(putbuf[i]);
			/* don't want `i++' in macro call! */
			++i;
		}
	}
}

#ifndef NODEBUGGER
extern void (*cdebugger)();
#endif

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much suspended.
 * Printf should not be used for chit-chat.
 *
 * MODIFICATION HISTORY:
 *	26 Feb 1988	greggo
 *		added support for %b and %c options.
 */
int prec;
int hadprec;

/*PRINTFLIKE1*/
void 
#ifdef __STDC__
printf(char *fmt, ... )
#else
printf(fmt)
	char *fmt;
#endif
{
	xprintf(fmt, (u_int) ((u_int *)&fmt + 1));
}

xprintf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	register int	c;
	register uint	*adx;
	register char	*s;
	int nb;		/* number of bytes to print.  used for %b support */

	adx = (u_int *)x1;

loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			if (!old_prt_flag)
				output(c);
			return;
		}
		output(c);
	}
	hadprec = 0;
	if ( *fmt == '.' ) {
		prec = 0;
		hadprec = 1;
		while ( '0' <= *++fmt && *fmt <= '9' )
			prec = prec * 10 + *fmt - '0';
	}
	if ((c = *fmt++) == 'l') c = *fmt++;
	if (c <= 'Z' && c >= 'A') c += 'a' - 'A';
	nb = (c == 'b') ? 1 : 4;
	if (c == 'd' || c == 'u' || c == 'o' || c == 'x' || c == 'b')
	    printn((long)*adx, c=='o'? 8: (c=='x'? 16: (c=='b'? 16:10)), nb);
	else if (c == 's') {
		s = (char *)*adx;
		while (c = *s++) {
			output(c);
		}
	}
	else if (c == 'c') {
		output( *adx & 0xff );
	}
	adx++;
	goto loop;
}

STATIC void
printn(n, b, nbytes)
	long n;
	register b;
{
	register i, nd, c;
	int	flag;
	int	plmax;
	char d[12];

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
	if (nbytes == 1)
		plmax = 2;
	else
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		output('-');
	}
	for (i=0;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = (char)nd;
		n = n/b;
		if ((n==0) && (b==10) && (flag==0))  /* zero fill hex, octal */
			break;
	}
	if (i==plmax)
		i--;
	if ( hadprec ) {
	    int npad;

	    npad = prec - i - 1;
	    while ( npad-- > 0 ) {
		output( ' ' );
	    }
	}
	for (;i>=0;i--) {
		output("0123456789ABCDEF"[d[i]]);
	}
}

/*
 * Panic is called on unresolvable fatal errors.
 */

void
#ifdef __STDC__
panic(char *str, ...)
#else
panic(str)
char	*str;
#endif
{
	int			x;
	int			i ;

	if (!prt_flag)
	{
		old_prt_flag = prt_flag;
		prt_flag = 1;
		flush_putbuf();
	}
	prt_where = PRW_CONS | PRW_BUF;
	panic_level = 1 ;
	xprintf("\nPANIC: ") ;
	xprintf(str, (u_int) ((u_int *)&str + 1)) ;
	xprintf("\n") ;
					/* save panic string (needed for  */
					/* routines elsewhere		  */
	panicstr = str;
					/* get execution level */
	x = splhi();
	if (old_prt_flag)
		splx(x);

#ifndef NODEBUGGER
	(*cdebugger)(DR_PANIC, NO_FRAME);/* return to kernel debugger */
#endif
	panic_act(panicstr);	/* call to rmc driver */
	sysdump();
	splhi();
	halt: goto halt;
}

/*
 * prdev prints a warning message.
 * dev is a block special device argument.
 */

void
prdev(str, dev)
	char *str;
	dev_t dev;
{
	register major_t maj;

	maj = getmajor(dev);
	if (maj >= bdevcnt) {
		cmn_err(CE_WARN,"%s on bad dev 0x%x\n", str, dev);
		return;
	}
	if ( *bdevsw[maj].d_flag & D_OLD)
		(*bdevsw[maj].d_print)(cmpdev(dev), str);
	else
		(*bdevsw[maj].d_print)(dev, str);
}


putchar(c)
register int c;
{
	(*conssw.co)(c);
}

/*
 * Deverr prints a diagnostic from a device driver.
 * It prints: error on device name (major/minor), block number,
 * and two arguments, usually error status.
 */
deverr(dp, o1, o2, dn)
    struct iobuf *dp;
    char *dn;		/* device name */
{
    register struct buf *bp;

	bp=dp->b_actf;
	cmn_err(CE_WARN, "error on dev %s (%u/%u)", 
		dn,major(bp->b_dev),minor(bp->b_dev));
	cmn_err(CE_WARN, ", block=%D cmd=%x status=%x\n", bp->b_blkno, o1, o2);
}

/*
 * Seterror sets u.u_error to the parameter value. Used by
 * loadable device drivers.
 */
seterror(errno)
char	errno;
{
	u.u_error = errno;
}

#ifdef USEMEYET
/*
 * Old-style drivers only
 * prcom prints a diagnostic from a device driver.
 * prt is device dependent print routine.
 */

void
prcom(prt, bp, er1, er2)
int (*prt)();
register struct buf *bp;
{
	(*prt)(bp->b_dev, "\nDevice error");
	cmn_err(CE_NOTE,"bn = %D er = %o,%o\n", bp->b_blkno, er1, er2);
}

/* return contents of r0 */

#if defined(lint)
extern int r0();
#else
int r0() {}
#endif
#endif

# define IUCONSOLE	0
# define SEC		300000	/* approx # loops per second */

/*PRINTFLIKE2*/
void
#ifdef __STDC__
cmn_err(int level, char *fmt, ... )
#else
cmn_err(level, fmt)
	int level;
	char *fmt;
#endif
{
	xcmn_err(level, &fmt);
}

xcmn_err(level, fmtp)
int	level;
char	**fmtp;
{
	register char	*fmt = *fmtp;
	register u_int	nextarg = (u_int) ((u_int *)fmtp + 1);
	register int	*ip;
	register int	i;
	register int	x;
	register int	r4save;
	register int	r3save;
	int		r0save;
	int		r1save;
	int		r2save;

	/*	Save the volatile registers as quickly as
	**	possible.  Sure hope the compiler sets up
	**	the stack frame in the obvious way.  This
	**	is used only for a panic but we must do
	**	the save now.
	*/

#ifdef USEMEYET
	asm(" MOVW %r0,0(%fp)");
	asm(" MOVW %r1,4(%fp)");
	asm(" MOVW %r2,8(%fp)");
#endif

	/*	Set up to print to putbuf, console, or both
	**	as indicated by the first character of the
	**	format.
	*/

	if (level == CE_PANIC)
	{
		if (!prt_flag)
		{
			old_prt_flag = prt_flag;
			prt_flag = 1;
			flush_putbuf();
		}
	}
	if (*fmt == '!') {
		prt_where = PRW_BUF;
		if (prt_flag)
			fmt++;
	} else if (*fmt == '^') {
		prt_where = PRW_CONS;
		if (prt_flag)
			fmt++;
	} else {
		prt_where = PRW_BUF | PRW_CONS;
	}

	switch (level) {
		case CE_CONT :
			errprintf(fmt, nextarg) ;
			break ;

		case CE_NOTE :
			errprintf("\nNOTICE: ") ;
			errprintf(fmt, nextarg) ;
			errprintf("\n") ;
			break ;

		case CE_WARN :
			if (prt_where & PRW_CONS)
			{
				warn_alm(fmtp);	/* call to rmc driver */
			}
			errprintf("\nWARNING: ") ;
			errprintf(fmt, nextarg) ;
			errprintf("\n") ;
			break ;

		case CE_PANIC :
			switch (panic_level) {
				case 0 :
					x = splhi() ;
					prt_where = PRW_CONS | PRW_BUF;
					panic_level = 1 ;
					printf("\nPANIC: ") ;
					xprintf(fmt, nextarg) ;
					printf("\n") ;
					if (old_prt_flag)
						splx(x) ;
					break;

				case 1 :
					prt_where = PRW_CONS | PRW_BUF;
					panic_level = 2 ;
					printf("\nDOUBLE PANIC: ") ;
					xprintf(fmt, nextarg) ;
					printf("\n") ;
					break;

				default :
					panic_level = 3 ;
					goto halt; /* Prevent recursion */
			}
# ifndef NODEBUGGER
			(*cdebugger)(DR_PANIC, NO_FRAME);
# endif
			panicstr=fmt; /* side-effects */
			panic_act(panicstr);	/* call to rmc driver */
			sysdump();
			splhi();
halt:		goto halt;

		default :
			cmn_err(CE_PANIC,
			  "unknown level: cmn_err(level=%d, msg=\"%s\")",
			  level, fmt);
	}
	/* In case someone later calls printf directly, set to print both. */
	prt_where = PRW_CONS | PRW_BUF;
}

#ifdef USE_ME_YET

printputbuf()
{
	register int		pbi;
	register int		cc;
	register int		lim;
	register struct tty	*tp;
	register int		opl;
	int			delay;
	struct tty		*errlayer();

	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	opl = splhi();

	if (conlayers() == 1) {
		if (cfreelist.c_next == NULL) {
			splx(opl);
			return;
		}
		tp = errlayer();  /* get tty pointer to error layer */
	} else {
		tp = NULL;
	}

	pbi = putbufndx % putbufsz;
	lim = putbufsz;

	while (1) {
		if (pbi < lim  &&  (cc = putbuf[pbi++])) {
			if (tp)
				putc(cc, tp->t_outq);
			else
				iuputchar(cc);
			if (cc == '\n'  &&  dmd_delay) {
				delay = dmd_delay;
				while (delay--) ;
			}
		} else {
			if (lim == putbufndx % putbufsz) {
				break;
			} else {
				lim = putbufndx % putbufsz;
				pbi = 0;
			}
		}
	}

	if (tp)
		xtvtproc(tp, T_OUTPUT);	/* Output buffer. */
	splx(opl);

	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}
#endif

#ifdef LAYERS
lprintf(fmt, x1)
register char	*fmt;
unsigned	x1;
{
	register		c;
	register uint		*adx;
	char			*s;
	register struct tty	*tp;
	register int		sr;	/* saved interrupt level */
	struct tty		*errlayer();

	tp = errlayer();	/* get tty pointer to error layer */
	sr = splhi();		/* ??? */
	if (cfreelist.c_next == NULL) { /* anywhere to buffer output? */
		splx(sr);		/* back to where we were */
		return;			/* nope, just return	*/
	}
	adx = (u_int *)x1;
loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			xtvtproc(tp, T_OUTPUT);	/* ??? */
			splx(sr);		/* back to where we were */
			return;
		}
		loutput(c, tp);
	}
	c = *fmt++;
	if (c == 'd' || c == 'u' || c == 'o' || c == 'x')
		lprintn((long)*adx, c=='o'? 8: (c=='x'? 16:10), tp);
	else if (c == 's') {
		s = (char *)*adx;
		while (c = *s++) {
			loutput(c, tp);
		}
	} else if (c == 'D') {
		lprintn(*(long *)adx, 10, tp);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}

lprintn(n, b, tp)
long n;
register b;
register struct tty *tp;
{
	register i, nd, c;
	int	flag;
	int	plmax;
	char d[12];

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		loutput('-', tp);
	}
	for (i=0;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = nd;
		n = n/b;
		if ((n==0) && (flag==0))
			break;
	}
	if (i==plmax)
		i--;
	for (;i>=0;i--) {
		loutput("0123456789ABCDEF"[d[i]], tp);
	}
}
#endif

errprintf(fmt, nextarg)
char	*fmt;
u_int	nextarg;
{

#ifdef LAYERS
	if (conlayers() == 1)
		lprintf(fmt, nextarg) ;
	else
#endif
		xprintf(fmt, nextarg) ;
}

#ifdef LAYERS
conlayers()		/* is the console running layers?? */
{
	struct tty	*tp;
	extern int	xtin();

	/* console has major dev. entry of 0 */
	/* get pointer to console tty structure */

	tp = cdevsw[0].d_ttys;

	/* use console line discipline and linesw */
	/* to really determine if layers is running */

	if (linesw[tp->t_line].l_input == xtin)
		return(1);	/* true, layers is running */
	else
		return(0);	/* false, layers is not running */
}

#endif

/*	Called by the ASSERT macro in debug.h when an
**	assertion fails.
*/

/*PRINTFLIKE1*/
void
#ifdef __STDC__
dri_printf(char *fmt, ... )
#else
dri_printf(fmt)
	char *fmt;
#endif
{
	xcmn_err(CE_CONT, &fmt);
}

int
assfail(a, f, l)
register char *a, *f;
{
	/*	Save all registers for the dump since crash isn't
	 *	very smart at the moment.
	 */
	
	register int	r6, r5, r4, r3;

	cmn_err(CE_PANIC, "assertion failed: %s, file: %s, line: %d",
		a, f, l);
}


void
nomemmsg(func, count, contflg, lockflg)
register char	*func;
register int	count;
register int	contflg;
register int	lockflg;
{
	cmn_err(CE_NOTE,
		"!%s - Insufficient memory to%s %d%s page%s - %s",
		func, (lockflg ? " lock" : " allocate"),
		count, (contflg ? " contiguous" : ""),
		(count == 1 ? "" : "s"),
		"system call failed");
}

/*
 * value to pass to oemreboot().  Configurable panic (via uadmin())
 * allows the system to stay down or reboot.
 *
 *  zero 	-> no auto reboot
 *  non-zero	-> automatic reboot
 */
int bootpanic = 0;

sysdump()
{
	static	buf_t	bp;
	uint	i, j, sz;
	caddr_t	vaddr;
	extern  struct  dscr gdt[];

	bp.b_proc=u.u_procp;
	bp.b_dev =cmpdev(dumpdev);
	bp.b_edev =dumpdev;

	/* Force a task switch from the current task (to the current task)
		so our state gets saved in the uarea */
	setdscrbase(&gdt[seltoi(JTSSSEL)], (uint)u.u_tss);
	gdt[seltoi(JTSSSEL)].a_acc0007=TSS3_KACC1;
	/* It sure would be nice to figure out some way of not hard-coding this */
	/* asm("	jmp	JTSSSEL,0");	*/
	asm("	ljmp	$0x170,$0");

	printf("Trying to dump %d Pages\n", physmem);
	spl0();
	/* Since memory may be in more than one piece, use memsegs */
	i = j = 0;
	while (i < bootinfo.memavailcnt && 
			(sz = bootinfo.memavail[i].extent) != 0) {
		vaddr = (caddr_t)xphystokv(bootinfo.memavail[i].base);
#if 0
	while (i < MAX_MEM_SEG && (sz = bootinfo.memsegs[i].seg_size) != 0) {
		vaddr = (caddr_t)xphystokv(bootinfo.memsegs[i].base_paddr);
#endif
		while (sz > 0 & !(bp.b_flags & B_ERROR)) {
			bp.b_blkno = ptod(j++);
			bp.b_un.b_addr = vaddr;
			sz -= (bp.b_bcount = NBPP);
			bp.b_flags = (B_BUSY | B_PHYS | B_WRITE);
			(*bdevsw[getmajor(dumpdev)].d_strategy)(&bp);
			while ((bp.b_flags & B_DONE) == 0) ;
			vaddr += NBPP;
			if (!(j&0xf)) printf(".");
		}
		i++;
	}
	printf("\n%d Pages dumped\n\n", j);
	oemreboot(bootpanic);
#if 0
	printf("Reboot the system now.\n");
	rtnfirm();
#endif
}


/*
** Print a register dump at most once in a panic.
** Record values of the registers for crash-dump analysis.
** Waits for slower terminals to catch up.
*/
int *snap_regptr;

snap(r0ptr,title)
int *r0ptr;
char *title;
{
	int i;
	int x;
	
	if (snap_regptr)
		return;
	snap_regptr = r0ptr;

	printf("%s:\n",title);
#if i386
	printf("cr0 0x%.8x     cr2  0x%.8x     cr3 0x%.8x     tlb  0x%.8x\n",
	       _cr0(), _cr2(), _cr3(), querytlb() );
	for(i=0;i<SEC;i++) ;
#endif
	printf("ss  0x%.8x     uesp 0x%.8x     efl 0x%.8x     ipl  0x%.8x\n",
	       0xffff&r0ptr[SS], r0ptr[UESP], r0ptr[EFL], x = splhi());
	if (old_prt_flag)
		splx(x);
	for(i=0;i<SEC;i++) ;
	printf("cs  0x%.8x     eip  0x%.8x     err 0x%.8x     trap 0x%.8x\n",
	       0xffff&r0ptr[CS], r0ptr[EIP], r0ptr[ERR], r0ptr[TRAPNO]);
	for(i=0;i<SEC;i++) ;
	printf("eax 0x%.8x     ecx  0x%.8x     edx 0x%.8x     ebx  0x%.8x\n",
	       r0ptr[EAX], r0ptr[ECX], r0ptr[EDX], r0ptr[EBX]);
	for(i=0;i<SEC;i++) ;
	printf("esp 0x%.8x     ebp  0x%.8x     esi 0x%.8x     edi  0x%.8x\n",
	       r0ptr[ESP], r0ptr[EBP], r0ptr[ESI], r0ptr[EDI]);
	for(i=0;i<SEC;i++) ;
	printf("ds  0x%.8x     es   0x%.8x     fs  0x%.8x     gs   0x%.8x\n",
	       0xffff&r0ptr[DS], 0xffff&r0ptr[ES], 0xffff&r0ptr[FS], 0xffff&r0ptr[GS]);
}


#if i386 /* May not work on 80486 */
/*
** Use the test registers to poke at the tlb
** and guess at where cr2 is really.
*/
querytlb()
{
	asm("movl	%cr2,%eax");

	/*
	** Only the defined bits, and consider bit 11 (which says give me valid
	** entries) as part of address.
	** Command (to lookup) is contained in bit 0.
	*/
	asm("andl	$0xfffff000,%eax");
	asm("orl	$0x00000801,%eax");

	/* Do the lookup */
	asm("movl	%eax,%tr6");
	asm("movl	%tr7,%eax");
	asm("movl	%tr6,%edx");

	/* Was it a hit? Was a valid entry? */
	/* asm("testl	$0x00000010,%eax"); */
	/* asm("jz		tlbinvalid"); */
	/* asm("testl	$0x00000800,%eax"); */
	/* asm("jz	tlbinvalid"); */

	/*
	** Take of the undefined bits on each result.
	** Store the two entries into one word
	** so they can be returned by ORing them together.
	*/
	asm("andl	$0xfffff01c,%eax");
	asm("andl	$0x00000fa1,%edx");
	asm("orl	%edx,%eax");
	asm("jmp	tlbvalid");

	asm("tlbinvalid:");
	asm("subl	%eax,%eax");
	asm("tlbvalid:");
}
#endif


#ifdef NOTDEF
/*
 * function for printing debugging information in printf
 */
say( string, value )
	char * string;
	unsigned int value;
{
	int i;

	putchar( '<' );
	while ( *string ) 
		putchar( *string++ );
	for ( i = 8; --i >= 0; )
		putchar( "0123456789abcdef"[ 0xf & ( value >> ( 4 * i ) ) ] );
	putchar( '>' ); putchar( '\n' );
}
#endif

#ifdef DEBUG
sysin() {}
sysout() {}
sysok() {}
sysoops() {}

/*
 ** set a breakpoint here to ge back to demon at regular intervals.
 */
catchmenow() {}
#endif /* DEBUG */
