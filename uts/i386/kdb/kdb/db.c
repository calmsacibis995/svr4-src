#ident	"@(#)db.c	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/db.c	1.3.1.2"

/*	AT&T 80x86 Kernel Debugger	*/

#include "sys/types.h"
#include "sys/param.h"
#include "sys/user.h"
#include "sys/reg.h"
#include "sys/regset.h"
#include "sys/cmn_err.h"
#include "sys/sysmsg.h"
#include "sys/kdebugger.h"
#include "sys/xdebug.h"
#include "sys/debug.h"
#include "debugger.h"
#include "../db_as.h"

extern gregset_t *regset[NREGSET];

#ifndef E425M
/* Remote Console Support */
extern struct smsg_flags smsg_flags;
static int smsg_rcef;
static int rmcsan;
#endif /* not E425M */

label_t dbsave;
int db_msg_pending;
int db_show_exit;

int dbactive;
ulong *ex_frame;	/* Pointer to exception stack frame */

extern int panic_level;
int save_panic_level;

static int	db_reason;
static unsigned db_d6;
static unsigned	enter;
static char	*do_cmds = NULL;

struct brkinfo db_brk[MAX_BRKNUM+1];
int db_brknum;
unsigned long db_brkaddr;

extern int dbsingle;
extern int db_pass_calls;

static as_addr_t bpt3_addr;
static u_char	bpt3_opc;

char *findsymname();
long findsymaddr();
void dbprintf();

static void load_brks();

int db_master();

void _debugger();
struct kdebugger db_kdb = {
        _debugger
};

void
db_init()
{
        kdb_register(&db_kdb);
}

void
_debugger(reason)
	int	reason;
{
	unsigned	d6;
	int		oldpri;

	if (!dbactive) {
#if !DEBUG
		if (kdb_security && (reason == DR_USER || reason == DR_OTHER))
#else
		if (kdb_security && reason == DR_USER)
#endif
			return;
		oldpri = splhi();
	} else
		splhi();

	d6 = _dr6();	/* read debug status register */
	_wdr6(0);		/* clear it for next time */
	_wdr7(0);		/* disable breakpoints to prevent recursion */

	flushtlb();

	{
		if (dbactive) {
			dbprintf("\nRestarting DEBUGGER\n");
			longjmp(&dbsave);
		}

		db_d6 = d6;

		if ((db_reason = reason) == DR_SECURE_USER)
			db_reason = DR_USER;

		db_stacktrace(NULL, NULL);
		ex_frame = (ulong *)(regset[0]);

		if (setjmp(&dbsave)) {
			/* make sure we don't disassemble
			   if we're restarted */
			db_reason = DR_USER;
		}

		if (db_master()) {
			/* Bail out if we had to revert to slave mode */
			splx(oldpri);
			return;
		}
	}

	load_brks();
	if (ex_frame != NULL) {
		((flags_t *)&ex_frame[EFL])->fl_rf = 1;
		((flags_t *)&ex_frame[EFL])->fl_tf = 0;
		if (dbsingle) {
			((flags_t *)&ex_frame[EFL])->fl_tf = 1;
			if (db_pass_calls) {
				uint		n;

				bpt3_addr.a_addr = ex_frame[EIP];
				bpt3_addr.a_as = AS_KVIRT;
				if ((n = db_is_call(bpt3_addr)) > 0) {
					((flags_t *)&ex_frame[EFL])->fl_tf = 0;
					bpt3_addr.a_addr += n;
					db_read(bpt3_addr, &bpt3_opc, 1);
					*(char *)&n = OPC_INT3;
					db_write(bpt3_addr, (char *)&n, 1);
				} else
					bpt3_addr.a_addr = 0;
			}
		}
	}

	dbactive = 0;

	splx(oldpri);
}


int
db_master()
{
	if (!dbactive) {
		save_panic_level = panic_level;
		panic_level = 0;
	}

	dbactive = 1;

#ifndef E425M
	/* Remote Console Support
	 *
	 * Don't have the kernel debugger write to both
	 * the alternate and the remote console.
	 * Turn off the sanity timer; we're going to be here awhile.
	 */
	if ((smsg_rcef = smsg_flags.rcef) != 0)
		smsg_flags.rcef = 0;
	rmcsan = rmcsantim(0);
#endif /* not E425M */

	if (db_reason == DR_STEP) {	/* called due to single-step */
		if (dbsingle)
			--dbsingle;
		if (ex_frame) {
			as_addr_t	addr;
			u_char		opc;
			flags_t		efl;

			addr.a_addr = ex_frame[EIP] - 1;
			addr.a_as = AS_KVIRT;
			db_read(addr, &opc, 1);
			if (opc == OPC_PUSHFL) {
				/* The pushfl will have pushed the single-
				   step flag bit onto the stack, so we have
				   to turn it off. */
				addr.a_addr = ex_frame[ESP] + ESP_OFFSET;
				db_read(addr, (char *)&efl, sizeof(efl));
				efl.fl_tf = 0;
				db_write(addr, (char *)&efl, sizeof(efl));
			}
		}
	} else if (db_reason == DR_BPT3) {  /* called due to inserted bkpt */
		if (db_pass_calls && dbsingle) {
			--dbsingle;
			db_reason = DR_STEP;
		}
	} else
		dbsingle = 0;

	if (bpt3_addr.a_addr) {
		db_write(bpt3_addr, &bpt3_opc, 1);
		if (ex_frame)
			ex_frame[EIP] = bpt3_addr.a_addr;
		bpt3_addr.a_addr = 0;
	}

	enter = !dbsingle;
	do_cmds = NULL;
	db_show_exit = 0;

	if (db_reason == DR_BPT1) {	/* called due to trap 1 breakpoint */
		db_reason = DR_OTHER;
		for (db_brknum = 0; db_brknum < 4; db_brknum++) {
			if (!(db_d6 & (1 << db_brknum)))
				continue;
			switch (db_brknum) {
			case 0:	db_brkaddr = _dr0();  break;
			case 1:	db_brkaddr = _dr1();  break;
			case 2:	db_brkaddr = _dr2();  break;
			case 3:	db_brkaddr = _dr3();  break;
			}
			db_reason = DR_BPT1;
			db_msg_pending = 1;
			if (db_brk[db_brknum].state != BRK_ENABLED) {
				db_brk_msg(0);
				dbprintf(" - wasn't set!\n");
			} else {
				if (db_brk[db_brknum].tcount) {
					db_brk_msg(0);
					dbprintf(" (%d left)\n",
						 --db_brk[db_brknum].tcount);
					enter = 0;
				} else
					do_cmds = db_brk[db_brknum].cmds;
			}
			break;
		}
	} else if (db_reason == DR_INIT) {
		if (*(do_cmds = debugger_init) == '\0')
			enter = 0;
	}

	if (db_reason == DR_BPT1 || db_reason == DR_INIT) {
		if (enter && !do_cmds)
			db_brk_msg(1);
	} else {
		if (db_reason != DR_STEP) {
			dbprintf("\nDEBUGGER:");
			if (db_reason == DR_BPT3)
				dbprintf(" Unexpected INT 3 Breakpoint!");
			dbprintf("\n");
		}
		if (db_reason != DR_USER && ex_frame != NULL)
			db_show_frame();
	}
	if (enter) {
		db_show_exit = !do_cmds;
		dbinterp(do_cmds);
		if (dbsingle == 0 && db_show_exit)
			dbprintf("DEBUGGER exiting\n");
	}

#ifndef E425M
	/* Remote Console Support */
	smsg_flags.rcef = smsg_rcef;	/* restore output flag */
	rmcsantim(rmcsan);
#endif /* not E425M */

	panic_level = save_panic_level;

	return 0;
}


db_show_frame()
{
	as_addr_t	addr;

	db_st_offset = 0;
	db_frameregs(ex_frame, 0, dbprintf);
	addr.a_addr = ex_frame[EIP];
	addr.a_as = AS_KVIRT;
	db_dis(addr, 1);
}

db_brk_msg(eol)
{
	db_msg_pending = 0;
	dbprintf("\nDEBUGGER: breakpoint %d at 0x%lx", db_brknum, db_brkaddr);
	if (eol) {
		dbprintf("\n");
		if (ex_frame)
			db_show_frame();
	}
	db_show_exit = 1;
}

dbextname(name)          /* look for external names in kernel debugger */
    char *name;
{
    if ((dbstack[dbtos].value.number = findsymaddr(name)) != NULL) {
	if (dbstackcheck(0, 1))
	    return 1;
	dbstack[dbtos].type = NUMBER;
	dbtos++;
	return 1;
    }
    return 0;   /* not found */
}

void dbtellsymname(name, addr, sym_addr)
	char	*name;
	ulong	addr, sym_addr;
{
	dbprintf("%s", name);
	if (addr != sym_addr)
		dbprintf("+0x%lx", addr - sym_addr);
	dbprintf("\n");
}


/* kernel stack dump - addresses increase right to left and bottom to top */

stackdump(dummy)
{
    ushort *tos;            /* top-of-stack pointer */
    ushort *sp;             /* stack pointer */

    tos = (ushort *)&dummy - 4;	    /* bp */
    sp = tos;                       /* for stack selector */
    *(ushort *)&sp = KSTKSZ * 2;    /* offset of high end of stack */

    do {
	sp -= 8;
	dbprintf("%4x %4x  %4x %4x  %4x %4x  %4x %4x    %4x\n",
		*(sp+7),*(sp+6),*(sp+5),*(sp+4),
		*(sp+3),*(sp+2),*(sp+1),*(sp),
		(ushort)sp);
    } while (sp > tos);
    dbprintf("bp at %x\n", (ushort)tos);
}


static u_long	brk_dr7_mask[] = {
	0xF0202, 0xF00208, 0xF000220, 0xF0000280
};

static void
load_brks()
{
	unsigned	dr7, brknum;
	struct brkinfo	*brkp;

	dr7 = 0;
	brkp = &db_brk[brknum = MAX_BRKNUM];
	do {
		if (brkp->state != BRK_ENABLED)
			continue;
		dr7 |= brkp->type << (16 +  4 * brknum);
		dr7 |= 2 << (2 * brknum);
		if (brkp->type != BRK_INST)
			dr7 |= 0x200;
	} while (--brkp, brknum-- != 0);

	_wdr0(db_brk[0].addr);
	_wdr1(db_brk[1].addr);
	_wdr2(db_brk[2].addr);
	_wdr3(db_brk[3].addr);
	_wdr7(dr7);
}


int
db_is_call(addr)
	as_addr_t	addr;
{
	u_char	opc;

	if (db_read(addr, &opc, sizeof(opc)) == -1)
		return 0;
	switch (opc) {
	case OPC_CALL_REL:
		return 5;
	case OPC_CALL_DIR:
		return 7;
	}
	return 0;
}
