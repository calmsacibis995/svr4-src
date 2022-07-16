#ident	"@(#)dbintrp.c	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/dbintrp.c	1.3.1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/inline.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/regset.h"
#include "sys/kdebugger.h"
#include "debugger.h"
#include "../db_as.h"
#include "dbcmd.h"

extern gregset_t *regset[NREGSET];

char *findsymname();
void dbprintf();

ushort dbtos;
struct item dbstack[DBSTKSIZ];
static struct variable variable[VARTBLSIZ];
static char *outformat = "%lx";
static ushort dbobase;
static char *typename[] =
	{"NULL","NUMBER","STRING","NAME" };
char dbverbose;
int dbsingle;
int db_pass_calls;
static int exit_db;

extern struct brkinfo db_brk[MAX_BRKNUM+1];

extern int	db_hard_eol;
extern uint	db_max_args;

as_addr_t db_cur_addr;
#define db_cur_as	db_cur_addr.a_as	/* Current address space */
#define db_as_number	db_cur_addr.a_mod	/* Numeric address space modifier */

int	db_cur_size;	/* Current operand size */
int	db_cur_rset;	/* Current register set # */

static unsigned	cur_brk_type;

static void c_pbrk();

static int doname();


asm int
real_cr3()
{
	movl	%cr3, %eax
}


static void
push(n)
    unsigned n;
{
    if ((DBSTKSIZ - (dbtos + n)) <  1) {
	dberror("stack overflow on push");
	return;
    }
    dbtos += n;
}


static void
pop(n)
    unsigned n;
{
    if (n > dbtos) {
	dberror("not enough items on stack to pop");
	return;
    }
    while (n-- > 0) {
	dbtos--;
	if (dbstack[dbtos].type == STRING)
	    dbstrfree(dbstack[dbtos].value.string);
	dbstack[dbtos].type = NULL;
    }
}


void
dbprintitem(ip, raw)
    struct item *ip;
    int	raw;
{
    ushort t = ip->type;

    if (t > TYPEMAX) {
	dbprintf("*** logic error - illegal item type number = %x\n", t);
	return;
    }
    if (dbverbose && !raw)
	dbprintf("%s = ", typename[t]);

    switch (t) {
    case (int) NULL:
	break;
    case NUMBER:
	dbprintf(outformat, ip->value.number);
	break;
    case STRING:
	if (!raw) {
	    dbprintf("\"%s\"", ip->value.string);
	    break;
	}
	/* Fall through ... */
    case NAME:
	dbprintf("%s", ip->value.string);
	break;
    }
    if (!raw)
	dbprintf("\n");
}


static void
c_obsolete(arg)
{
	dberror("command no longer supported");
}


static int
c_read(arg)
{
	u_char	val[4];

	db_cur_addr.a_addr = dbstack[dbtos-1].value.number;
	if (db_read(db_cur_addr, val, db_cur_size) == -1) {
		dberror("invalid address");
		return -1;
	}
	switch (db_cur_size) {
	case 1:
		dbstack[dbtos-1].value.number = *(u_char *)val;
		break;
	case 2:
		dbstack[dbtos-1].value.number = *(u_short *)val;
		break;
	case 4:
		dbstack[dbtos-1].value.number = *(u_long *)val;
		break;
	}
	return 0;
}

static void
c_write(arg)
{
	u_char	val[4];

	switch (db_cur_size) {
	case 1:
		*(u_char *)val = dbstack[dbtos-2].value.number;
		break;
	case 2:
		*(u_short *)val = dbstack[dbtos-2].value.number;
		break;
	case 4:
		*(u_long *)val = dbstack[dbtos-2].value.number;
		break;
	}
	db_cur_addr.a_addr = dbstack[dbtos-1].value.number;
	if (db_write(db_cur_addr, val, db_cur_size) == -1) {
		dberror("invalid address");
		return;
	}
	dbtos -= 2;
}

static void
c_findsym(arg)
{
	findsymname(dbstack[--dbtos].value.number, dbtellsymname);
}

static void
c_pvfs(arg)
{
	db_cur_addr.a_addr = dbstack[--dbtos].value.number;
	db_pvfs(db_cur_addr);
}

static void
c_pvnode(arg)
{
	db_cur_addr.a_addr = dbstack[--dbtos].value.number;
	db_pvnode(db_cur_addr);
}

static void
c_pinode(arg)
{
	db_cur_addr.a_addr = dbstack[--dbtos].value.number;
	db_pinode(db_cur_addr);
}

static void
c_puinode(arg)
{
	db_cur_addr.a_addr = dbstack[--dbtos].value.number;
	db_puinode(db_cur_addr);
}

static void
c_pprnode(arg)
{
	db_cur_addr.a_addr = dbstack[--dbtos].value.number;
	db_pprnode(db_cur_addr);
}

static void
c_psnode(arg)
{
	db_cur_addr.a_addr = dbstack[--dbtos].value.number;
	db_psnode(db_cur_addr);
}

static void
c_op_not(arg)
{
	dbstack[dbtos-1].value.number = !dbstack[dbtos-1].value.number;
}

static void
c_op_incr(arg)
{
	dbstack[dbtos-1].value.number++;
}

static void
c_op_decr(arg)
{
	dbstack[dbtos-1].value.number--;
}

static void
c_op_mul(arg)
{
	dbstack[dbtos-2].value.number *= dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_div(arg)
{
	dbstack[dbtos-2].value.number /= dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_mod(arg)
{
	dbstack[dbtos-2].value.number %= dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_plus(arg)
{
	dbstack[dbtos-2].value.number += dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_minus(arg)
{
	dbstack[dbtos-2].value.number -= dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_rshift(arg)
{
	unsigned	n = dbstack[dbtos-1].value.number & 0x1f;

	dbstack[dbtos-2].value.number >>= n;
	--dbtos;
}

static void
c_op_lshift(arg)
{
	unsigned	n = dbstack[dbtos-1].value.number & 0x1f;

	dbstack[dbtos-2].value.number <<= n;
	--dbtos;
}

static void
c_op_lt(arg)
{
	dbstack[dbtos-2].value.number =
		dbstack[dbtos-2].value.number < dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_gt(arg)
{
	dbstack[dbtos-2].value.number =
		dbstack[dbtos-2].value.number > dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_eq(arg)
{
	dbstack[dbtos-2].value.number =
		dbstack[dbtos-2].value.number == dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_ne(arg)
{
	dbstack[dbtos-2].value.number =
		dbstack[dbtos-2].value.number != dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_and(arg)
{
	dbstack[dbtos-2].value.number &= dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_xor(arg)
{
	dbstack[dbtos-2].value.number ^= dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_or(arg)
{
	dbstack[dbtos-2].value.number |= dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_log_and(arg)
{
	dbstack[dbtos-2].value.number =
		dbstack[dbtos-2].value.number && dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_op_log_or(arg)
{
	dbstack[dbtos-2].value.number =
		dbstack[dbtos-2].value.number || dbstack[dbtos-1].value.number;
	--dbtos;
}

static void
c_assign(arg)	/* assignment to a variable (or macro) */
{
	int	i, n;
	char	*s;

	i = dbgetitem(&dbstack[dbtos]); /* get variable name*/
	if (dbverbose) {
	    dbprintf("%x: ", dbtos);
	    dbprintitem(&dbstack[dbtos], 0);
	}
	if (i == EOF) {
	    exit_db = 1;
	    return;
	}
	if (dbtypecheck(dbtos, NAME))
	    return;
	s = dbstack[dbtos].value.string;
	if (doname(s, 1)) {
	    dberror("name already used");
	    return;
	}
	if (arg == VAR_MACRO && dbtypecheck(dbtos - 1, STRING))
	    return;
	for (i = 0, n = VARTBLSIZ; i < VARTBLSIZ; i++) {
	    if (variable[i].name == NULL) {
		if (n == VARTBLSIZ)
		    n = i;          /* save free table slot */
	    }
	    else if (strcmp(variable[i].name, s) == 0)    /* name found */
		break;
	}
	if (i == VARTBLSIZ) {       /* name not found */
	    if (n == VARTBLSIZ) {
		dberror("variable table overflow");
		return;
	    }
	    i = n;                  /* new slot */
	    variable[i].name = dbstrdup(s);
	}
	else if (variable[i].item.type == STRING)   /* existing string */
	    dbstrfree(variable[i].item.value.string); /* free string */
	variable[i].item = dbstack[dbtos-1];
	variable[i].type = arg;
	dbtos--;
}

static void
c_print(arg)
{
	if (dbverbose) {
		dbprintf("%x: ", dbtos-1);
	}
	dbprintitem(&dbstack[dbtos-1], arg);
	if (arg)
		pop(1);
}

static void
c_print_n(arg)
{
	int	i, n_vals;

	n_vals = dbstack[dbtos-1].value.number;
	if (n_vals < 0) {
		dberror("negative number of arguments");
		return;
	}
	if (dbstackcheck(n_vals + 1, 0))
		return;
	for (i = n_vals; i-- > 0;)
		dbprintitem(&dbstack[dbtos - i - 2], 1);
	dbtos -= n_vals + 1;
}

static void
c_pop(arg)
{
	pop(arg ? arg : dbtos);
}

static void
c_dup(arg)
{
	dbstack[dbtos] = dbstack[dbtos-1];
	if (dbstack[dbtos].type == STRING) {
		char	*s = dbstrdup(dbstack[dbtos].value.string);

		dbstack[dbtos].value.string = s;
	}
	dbtos++;
}

static void
c_verbose(arg)
{
	dbverbose = arg;
}

static void
c_inbase(arg)
{
	if (arg == 0) {
		arg = dbstack[--dbtos].value.number;
		if (arg < 2 || arg > 16) {
			dberror("illegal input base value - 2 thru 16 accepted");
			return;
		}
	}
	dbibase = arg;
}

static void
c_outbase(arg)
{
	if (arg == 0) {
		arg = dbstack[--dbtos].value.number;
		if (arg < 2 || arg > 16) {
			dberror("illegal output base value - 2 thru 16 accepted");
			return;
		}
	}
	dbobase = arg;
}

static void
c_dumpstack(arg)
{
	register u_int	i;

	for (i = 0; i < dbtos; i++) {
		if (dbverbose)
			dbprintf("%x: ", i);
		dbprintitem(&dbstack[i], 0);
	}
}

static void
c_vars(arg)
{
	register u_int	i;

	for (i = 0; i < VARTBLSIZ; i++) {
		if (variable[i].name != NULL) {
			if (dbverbose)
				dbprintf("%x: ", i);
			dbprintf("%s %s ", variable[i].name,
				variable[i].type == VAR_MACRO ? "::" : "=");
			dbprintitem(&variable[i].item, 0);
		}
	}
}

static void
c_sysdump(arg)
{
	sysdump();
}

static void
c_dump(arg)
{
	db_cur_addr.a_addr = dbstack[dbtos-2].value.number;
	db_dump(db_cur_addr, dbstack[dbtos-1].value.number);
	dbtos -= 2;
}

static void
c_stacktrace(arg)
{
	db_pstack(arg? dbstack[--dbtos].value.number : -1, arg, dbprintf);
}

static void
c_stackargs(arg)
{
	if (dbstack[--dbtos].value.number >= 0)
		db_max_args = dbstack[dbtos].value.number;
	else
		dberror("value for stackargs must not be negative");
}

static void
c_kstackdump(arg)
{
	stackdump();
}

static void
c_cr0(arg)
{
	dbstack[dbtos].value.number = _cr0();
	dbstack[dbtos++].type = T_NUMBER;
}

static void
c_cr2(arg)
{
	dbstack[dbtos].value.number = _cr2();
	dbstack[dbtos++].type = T_NUMBER;
}

static void
c_cr3(arg)
{
	dbstack[dbtos].value.number = real_cr3();
	dbstack[dbtos++].type = T_NUMBER;
}

static int
_load_regset()
{
	if (db_cur_as != AS_UVIRT)
		db_as_number = -1;
	db_pstack(db_as_number, 0, NULL);

	if (regset[db_cur_rset] == NULL) {
		dberror("no such register set");
		return -1;
	}
	return 0;
}

static void
c_getreg(arg)
{
	if (_load_regset() == -1)
		return;

	dbstack[dbtos].value.number = (*regset[db_cur_rset])[arg & ~REG_TYPE];
	if ((arg & ~REG_TYPE) == ESP) {
		/* Special case, to account for value of ESP before a trap */
		dbstack[dbtos].value.number += ESP_OFFSET;
	}
	switch (arg & REG_TYPE) {
	case REG_X:
		dbstack[dbtos].value.number &= 0xFFFF;
		break;
	case REG_H:
		dbstack[dbtos].value.number &= 0xFF00;
		dbstack[dbtos].value.number >>= 8;
		break;
	case REG_L:
		dbstack[dbtos].value.number &= 0xFF;
		break;
	}
	dbstack[dbtos++].type = T_NUMBER;
}

static void
c_setreg(arg)
{
	unsigned	val = dbstack[--dbtos].value.number;

	if (_load_regset() == -1)
		return;

	if ((arg & ~REG_TYPE) == ESP) {
		/* Special case, to account for value of ESP before a trap */
		val -= ESP_OFFSET;
	}
	switch (arg & REG_TYPE) {
	case REG_E:
		(*regset[db_cur_rset])[arg & ~REG_TYPE] = val;
		break;
	case REG_X:
		*(short *)&(*regset[db_cur_rset])[arg & ~REG_TYPE] = val;
		break;
	case REG_H:
		*((char *)&(*regset[db_cur_rset])[arg & ~REG_TYPE] + 1) = val;
		break;
	case REG_L:
		*(char *)&(*regset[db_cur_rset])[arg & ~REG_TYPE] = val;
		break;
	}
}

static void
c_const(arg)
{
	dbstack[dbtos].value.number = arg;
	dbstack[dbtos++].type = T_NUMBER;
}

static void
c_brk(arg)
{
	unsigned	brknum, type;
	struct brkinfo	*brkp;
	char		*cmds;
	unsigned long	addr;

	if (arg) {
		brknum = dbstack[--dbtos].value.number;
		if (brknum > MAX_BRKNUM) {
			dberror("breakpoint number too big");
			return;
		}
	} else {
		/* Find first unused breakpoint */
		for (brknum = 0; brknum <= MAX_BRKNUM; ++brknum) {
			if (db_brk[brknum].state == BRK_CLEAR)
				break;
		}
		if (brknum > MAX_BRKNUM) {
			dberror("all breakpoints in use");
			return;
		}
	}

	brkp = &db_brk[brknum];

	if (dbstack[dbtos-1].type == STRING) {
		if (dbstackcheck(2, 0))
			return;
		cmds = dbstack[--dbtos].value.string;
	} else
		cmds = NULL;

	if (dbtypecheck(dbtos - 1, NUMBER))
		return;
	brkp->addr = dbstack[--dbtos].value.number;

	if ((brkp->type = cur_brk_type) != BRK_INST)
		brkp->type |= (db_cur_size - 1) << 2;

	if (brkp->cmds != NULL)
		dbstrfree(brkp->cmds);
	brkp->cmds = cmds;

	brkp->state = BRK_ENABLED;
	brkp->tcount = 0;

	c_pbrk(brknum);
}

static void
c_trace(arg)
{
	unsigned	brknum;

	brknum = dbstack[--dbtos].value.number;
	if (brknum > MAX_BRKNUM) {
		dberror("breakpoint number too big");
		return;
	}
	db_brk[brknum].tcount = dbstack[--dbtos].value.number;
	c_pbrk(brknum);
}

static void
_brkstate(brknum, state)
	unsigned	brknum, state;
{
	static struct brkinfo	null_brk;

	if (state == BRK_CLEAR)
		db_brk[brknum] = null_brk;
	else if (db_brk[brknum].state != BRK_CLEAR)
		db_brk[brknum].state = state;
}

static void
c_brkstate(arg)
{
	unsigned	brknum;

	brknum = dbstack[--dbtos].value.number;
	if (brknum > MAX_BRKNUM) {
		dberror("breakpoint number too big");
		return;
	}
	_brkstate(brknum, arg);
	c_pbrk(brknum);
}

static void
c_allbrkstate(arg)
{
	unsigned	brknum;

	for (brknum = 0; brknum <= MAX_BRKNUM; brknum++)
		_brkstate(brknum, arg);
}

static void
c_pbrk(arg)
{
	unsigned	brknum, endnum;
	struct brkinfo	*brkp;
	u_long		addr, sym_addr;
	char		*name;
	unsigned	any_set = 0;

	if (arg == -1) {
		brknum = 0;
		endnum = MAX_BRKNUM;
	} else
		brknum = endnum = arg;

	do {
		brkp = &db_brk[brknum];

		if (arg == -1 && brkp->state == BRK_CLEAR)
			continue;

		++any_set;

		addr = brkp->addr;
		dbprintf("%d: 0x%lx", brknum, addr);
		if ((name = findsymname(addr, NULL)) != NULL) {
			dbprintf("(%s", name);
			if ((sym_addr = findsymaddr(name)) != addr)
				dbprintf("+0x%lx", addr - sym_addr);
			dbprintf(")");
		}

		switch (brkp->state) {
		case BRK_CLEAR:
			dbprintf(" OFF ");
			break;
		case BRK_ENABLED:
			dbprintf(" ON ");
			break;
		case BRK_DISABLED:
			dbprintf(" DISABLED ");
			break;
		}
		if (brkp->tcount)
			dbprintf(" 0x%x ", brkp->tcount);

		switch (brkp->type & 3) {
		case BRK_INST:
			dbprintf("/i");
			break;
		case BRK_MODIFY:
			dbprintf("/m");
			break;
		case BRK_ACCESS:
			dbprintf("/a");
			break;
		}

		if (brkp->type != BRK_INST) {
			switch (brkp->type & 0xC) {
			case 0:
				dbprintf("b");
				break;
			case 4:
				dbprintf("w");
				break;
			case 0xC:
				dbprintf("l");
				break;
			}
		}

		if (brkp->cmds)
			dbprintf(" \"%s\" ", brkp->cmds);

		dbprintf("\n");

	} while (brknum++ != endnum);

	if (!any_set)
		dbprintf("No breakpoints set\n");
}

static void
c_old_brk(arg)	/* for old-style brk0-brk3 commands */
{
	unsigned	type;

	type = dbstack[--dbtos].value.number;

	dbstack[dbtos++].value.number = arg;

	if (type == 47)	/* .clr */
		c_brkstate(BRK_CLEAR);
	else {
		cur_brk_type = type & 3;
		db_cur_size = ((type & 0xC) >> 2) + 1;
		c_brk(1);
	}
}

static void
c_vtop(arg)
{
	addr_t	vaddr;
	paddr_t	paddr;

	if ((db_cur_as = (db_as_t)arg) == AS_UVIRT) {
		proc_t	*proc;
		int	n = dbstack[--dbtos].value.number;

		if ((proc = pid_entry(n)) == NULL) {
			dberror("no such process");
			return;
		}
		vaddr = (addr_t)dbstack[--dbtos].value.number;
		paddr = db_uvtop(vaddr, proc);
		dbprintf("User Process %x", n);
	} else { /* db_cur_as == AS_KVIRT */
		vaddr = (addr_t)dbstack[--dbtos].value.number;
		paddr = db_kvtop(vaddr);
		dbprintf("Kernel");
	}
	dbprintf(" Virtual %x ", vaddr);
	if (paddr == (paddr_t)-1)
		dbprintf("not mapped\n");
	else
		dbprintf("= Physical %x\n", paddr);
}

static void
c_sleeping(arg)
{
	db_sleeping();
}

static void
c_ps(arg)
{
	db_ps();
}

static void
c_newterm(arg)
{
	kdb_next_io();
}

static void
c_newdebug(arg)
{
	kdb_next_debugger();
}

static void
c_dis(arg)
{
	db_cur_addr.a_addr = dbstack[dbtos-2].value.number;
	db_dis(db_cur_addr, dbstack[dbtos-1].value.number);
	dbtos -= 2;
}

static void
c_single(arg)
{
	if (arg == 0)
		arg = dbstack[--dbtos].value.number;
	if ((dbsingle = arg) != 0)
		exit_db = 1;
	db_pass_calls = 0;
}

static void
c_pass_calls(arg)
{
	if (arg == 0)
		arg = dbstack[--dbtos].value.number;
	if ((dbsingle = arg) != 0) {
		exit_db = 1;
		db_pass_calls = 1;
	} else
		db_pass_calls = 0;
}

static void
c_call(arg)
{
	int	(*func)();
	int	i, n_args, ret;

	func = (int (*)())dbstack[dbtos-2].value.number;
	n_args = dbstack[dbtos-1].value.number;
	if (n_args < 0) {
		dberror("negative number of arguments");
		return;
	}
	if (n_args > 5) {
		dberror("more than 5 arguments not supported");
		return;
	}
	if (dbstackcheck(n_args + 2, 0))
		return;
	for (i = 0; i < n_args; i++) {
		if (dbtypecheck(dbtos - i - 3, NUMBER))
			return;
	}
	dbtos -= n_args + 2;
	ret = (*func)(dbstack[dbtos].value.number, dbstack[dbtos+1].value.number,
		dbstack[dbtos+2].value.number, dbstack[dbtos+3].value.number,
		dbstack[dbtos+4].value.number);
	asm("cli");
	dbstack[dbtos].value.number = ret;
	dbstack[dbtos++].type = NUMBER;
}

static void
c_then(arg)
{
	int	t;

	if (dbstack[--dbtos].value.number) {
		/* "True" case - just return and execute next cmd(s) */
		return;
	}

	/* "False" case - ignore all input up to an "endif" or end of "line" */
	db_hard_eol = 1;
	for (;;) {
		t = dbgetitem(&dbstack[dbtos]);
		if (dbverbose) {
			dbprintf("%x: ", dbtos);
			dbprintitem(&dbstack[dbtos], 0);
		}
		if (t == NAME) {
			if (strcmp(dbstack[dbtos].value.string, "endif") == 0)
				break;
		} else if (t == EOL)
			break;
		else if (t == EOF) {
			exit_db = 1;
			break;
		}
	}
	db_hard_eol = 0;
}

static void
c_endif(arg)
{
	/* No-op - place marker for end of "then" scope */
}

static void
c_quit(arg)
{
	db_flush_input();
	exit_db = 1;
}

static char *cmds_help_text[] = {
"Value stack:   clrstk dup P p PP pop stk",
"Arithmetic:    + - * / % >> << > < == != & | ^ && || ! ++ --",
"Read/Write:    dump r w",
"Get Registers: %<reg> cr[0|2|3]		Set Registers: w%<reg>",
"   <reg> is one of: eax ax ah al ebx bx bh bl ecx cx ch cl edx dx dh dl efl fl",
"		    esi si edi di ebp bp esp sp eip ip cs ds es fs gs err trap",
"Variables:     = :: vars",
"Breakpoints:   B b brkoff brkon brksoff brkson clrbrk clrbrks trace ?brk",
"Single Step:   S s SS ss",
"Kernel Data:   pinode pprnode ps psnode puinode pvfs pvnode sleeping",
"Kernel Stack:  pstack stack stackdump stackargs",
"Numeric Base:  ibase ibinary idecimal ihex ioctal obase odecimal ohex ooctal",
"Address Space: kvtop uvtop",
"Help:	       cmds help",
"Flow Control:  endif then q",
"Miscellaneous: call dis findsym nonverbose newdebug newterm sysdump verbose",
"",
"Suffixes:      /l = 4-byte long (default), /w = 2-byte word, /b = byte",
"	       / or /k = kernel virtual (default), /p = physical, /io = I/O",
"	       /u# = user process # virtual, /rs# = register-set #",
"	       breakpoint: /a = data access, /m = data modify, /i = instruction",
NULL
};
static char *main_help_text[] = {
"cmds			display a list of all commands",
"{ARGS} ADDR COUNT call	call the function at ADDR with COUNT arguments",
"ADDR COUNT dump		show COUNT bytes starting at ADDR",
"ADDR r			read 1, 2 or 4 bytes from virtual address ADDR",
"VAL ADDR w		write VAL to virtual address ADDR (1, 2 or 4 bytes)",
"ADDR [STRING] NUM B	set breakpoint #NUM at ADDR w/optional command string",
"			use suffixes /a for access or /m for modify breakpoints",
"ADDR [STRING] b		same as \"B\", but use first free breakpoint #",
"NUM brkoff		temporarily disable breakpoint #NUM",
"NUM brkon		re-enable breakpoint #NUM",
"COUNT NUM trace		skip over COUNT instances of breakpoint #NUM",
"s			single step one instruction (\"S\" passes over calls)",
"COUNT ss		single step COUNT instructions (\"SS\" passes over calls)",
"stack			show stack trace of current process",
"PROC pstack		show stack trace of process PROC (slot # or addr)",
"ADDR findsym		print name of symbol nearest ADDR",
"ADDR COUNT dis		disassemble COUNT instructions starting at ADDR",
"VAL = VAR		store VAL in a variable named VAR",
"STRING :: VAR		define variable VAR as a macro of commands from STRING",
"q -or- ^D		exit from debugger",
NULL
};
static char **help_text[] = {
	main_help_text,
	cmds_help_text
};

static void
c_help(arg)
{
	register char	**msgp = help_text[arg];

	dbprintf("\n");
	while (*msgp != NULL)
		dbprintf("%s\n", *msgp++);
	dbprintf("\n");
}


struct cmdentry cmdtable[] = {

{ "r",		(void (*)())c_read,	0,	S_1_0,	1,	T_NUMBER },
{ "r1",		NULL,		(int)"r/b" },
{ "r2",		NULL,		(int)"r/w" },
{ "r4",		NULL,		(int)"r/l" },
{ "w",		c_write,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "w1",		NULL,		(int)"w/b" },
{ "w2",		NULL,		(int)"w/w" },
{ "w4",		NULL,		(int)"w/l" },
{ "rp1",	NULL,		(int)"r/b/p" },
{ "rp2",	NULL,		(int)"r/w/p" },
{ "rp4",	NULL,		(int)"r/l/p" },
{ "wp1",	NULL,		(int)"w/b/p" },
{ "wp2",	NULL,		(int)"w/w/p" },
{ "wp4",	NULL,		(int)"w/l/p" },
{ "rio1",	NULL,		(int)"r/b/io" },
{ "rio2",	NULL,		(int)"r/w/io" },
{ "rio4",	NULL,		(int)"r/l/io" },
{ "wio1",	NULL,		(int)"w/b/io" },
{ "wio2",	NULL,		(int)"w/w/io" },
{ "wio4",	NULL,		(int)"w/l/io" },
{ "!",		c_op_not,	0,	S_1_0,	1,	T_NUMBER },
{ "++",		c_op_incr,	0,	S_1_0,	1,	T_NUMBER },
{ "--",		c_op_decr,	0,	S_1_0,	1,	T_NUMBER },
{ "*",		c_op_mul,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "/",		c_op_div,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "%",		c_op_mod,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "+",		c_op_plus,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "-",		c_op_minus,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ ">>",		c_op_rshift,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "<<",		c_op_lshift,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "<",		c_op_lt,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ ">",		c_op_gt,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "==",		c_op_eq,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "!=",		c_op_ne,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "&",		c_op_and,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "^",		c_op_xor,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "|",		c_op_or,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "&&",		c_op_log_and,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "||",		c_op_log_or,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "=",		c_assign,	VAR_VAR,	S_1_0,	0 },
{ "::",		c_assign,	VAR_MACRO,	S_1_0, 	0 },
{ "p",		c_print,	0,	S_1_0,	0 },
{ "P",		c_print,	1,	S_1_0,	0 },
{ "PP",		c_print_n,	0,	S_1_0,	1,	T_NUMBER },
{ "pop",	c_pop,		1,	0,	0 },
{ "clrstk",	c_pop,		0,	0,	0 },
{ "dup",	c_dup,		0,	S_0_1,	0 },
{ "nonverbose",	c_verbose,	0,	0,	0 },
{ "verbose",	c_verbose,	1,	0,	0 },
{ "ibase",	c_inbase,	0,	S_1_0,	1,	T_NUMBER },
{ "ibinary",	c_inbase,	2,	0,	0 },
{ "ioctal",	c_inbase,	8,	0,	0 },
{ "idecimal",	c_inbase,	10,	0,	0 },
{ "ihex",	c_inbase,	16,	0,	0 },
{ "obase",	c_outbase,	0,	S_1_0,	1,	T_NUMBER },
{ "ooctal",	c_outbase,	8,	0,	0 },
{ "odecimal",	c_outbase,	10,	0,	0 },
{ "ohex",	c_outbase,	16,	0,	0 },
{ "stk",	c_dumpstack,	0,	0,	0 },
{ "vars",	c_vars,		0,	0,	0 },
{ "findsym",	c_findsym,	0,	S_1_0,	1,	T_NUMBER },
{ "pvfs",	c_pvfs,		0,	S_1_0,	1,	T_NUMBER },
{ "pvnode",	c_pvnode,	0,	S_1_0,	1,	T_NUMBER },
{ "pinode",	c_pinode,	0,	S_1_0,	1,	T_NUMBER },
{ "puinode",	c_puinode,	0,	S_1_0,	1,	T_NUMBER },
{ "pprnode",	c_pprnode,	0,	S_1_0,	1,	T_NUMBER },
{ "psnode",	c_psnode,	0,	S_1_0,	1,	T_NUMBER },
{ "sysdump",	c_sysdump,	0,	0,	0 },
{ "dump",	c_dump,		0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "stack",	c_stacktrace,	0,	0,	0 },
{ "pstack",	c_stacktrace,	1,	S_1_0,	1,	T_NUMBER },
{ "stackargs",	c_stackargs,	1,	S_1_0,	1,	T_NUMBER },
{ "stackdump",	c_kstackdump,	0,	0,	0 },
{ "B",		c_brk,		1,	S_2_0,	1,	T_NUMBER },
{ "b",		c_brk,		0,	S_1_0,	0 },
{ "brkoff",	c_brkstate,	BRK_DISABLED,	S_1_0,	1,	T_NUMBER },
{ "brkon",	c_brkstate,	BRK_ENABLED,	S_1_0,	1,	T_NUMBER },
{ "brksoff",	c_allbrkstate,	BRK_DISABLED,	0,	0 },
{ "brkson",	c_allbrkstate,	BRK_ENABLED,	0,	0 },
{ "clrbrk",	c_brkstate,	BRK_CLEAR,	S_1_0,	1,	T_NUMBER },
{ "clrbrks",	c_allbrkstate,	BRK_CLEAR,	0,	0 },
{ "trace",	c_trace,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "?brk",	c_pbrk,		-1,	0,	0 },
{ ".i",		c_const,	0,	S_0_1,	0 },	/* bkwd compat */
{ ".a",		c_const,	3,	S_0_1,	0 },	/* bkwd compat */
{ ".m",		c_const,	1,	S_0_1,	0 },	/* bkwd compat */
{ ".aw",	c_const,	7,	S_0_1,	0 },	/* bkwd compat */
{ ".mw",	c_const,	5,	S_0_1,	0 },	/* bkwd compat */
{ ".al",	c_const,	15,	S_0_1,	0 },	/* bkwd compat */
{ ".ml",	c_const,	13,	S_0_1,	0 },	/* bkwd compat */
{ ".clr",	c_const,	47,	S_0_1,	0 },	/* bkwd compat */
{ "brk0",	c_old_brk,	0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "brk1",	c_old_brk,	1,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "brk2",	c_old_brk,	2,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "brk3",	c_old_brk,	3,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "trc0",	NULL,		(int)"0 trace" },
{ "trc1",	NULL,		(int)"1 trace" },
{ "trc2",	NULL,		(int)"2 trace" },
{ "trc3",	NULL,		(int)"3 trace" },
{ "db?",	NULL,		(int)"?brk" },
{ "help",	c_help,		0,	0,	0 },
{ "?",		NULL,		(int)"help" },
{ "cmds",	c_help,		1,	0,	0 },
{ "sleeping",	c_sleeping,	0,	0,	0 },
{ "ps",		c_ps,		0,	0,	0 },
{ "newterm",	c_newterm,	0,	0,	0 },
{ "newdebug",	c_newdebug,	0,	0,	0 },
{ "dis",	c_dis,		0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "s",		c_single,	1,	0,	0 },
{ "ss",		c_single,	0,	S_1_0,	1,	T_NUMBER },
{ "S",		c_pass_calls,	1,	0,	0 },
{ "SS",		c_pass_calls,	0,	S_1_0,	1,	T_NUMBER },
{ "%eax",	c_getreg,	EAX+REG_E,	S_0_1,	0 },
{ "%ax",	c_getreg,	EAX+REG_X,	S_0_1,	0 },
{ "%ah",	c_getreg,	EAX+REG_H,	S_0_1,	0 },
{ "%al",	c_getreg,	EAX+REG_L,	S_0_1,	0 },
{ "%ebx",	c_getreg,	EBX+REG_E,	S_0_1,	0 },
{ "%bx",	c_getreg,	EBX+REG_X,	S_0_1,	0 },
{ "%bh",	c_getreg,	EBX+REG_H,	S_0_1,	0 },
{ "%bl",	c_getreg,	EBX+REG_L,	S_0_1,	0 },
{ "%ecx",	c_getreg,	ECX+REG_E,	S_0_1,	0 },
{ "%cx",	c_getreg,	ECX+REG_X,	S_0_1,	0 },
{ "%ch",	c_getreg,	ECX+REG_H,	S_0_1,	0 },
{ "%cl",	c_getreg,	ECX+REG_L,	S_0_1,	0 },
{ "%edx",	c_getreg,	EDX+REG_E,	S_0_1,	0 },
{ "%dx",	c_getreg,	EDX+REG_X,	S_0_1,	0 },
{ "%dh",	c_getreg,	EDX+REG_H,	S_0_1,	0 },
{ "%dl",	c_getreg,	EDX+REG_L,	S_0_1,	0 },
{ "%edi",	c_getreg,	EDI+REG_E,	S_0_1,	0 },
{ "%di",	c_getreg,	EDI+REG_X,	S_0_1,	0 },
{ "%esi",	c_getreg,	ESI+REG_E,	S_0_1,	0 },
{ "%si",	c_getreg,	ESI+REG_X,	S_0_1,	0 },
{ "%ebp",	c_getreg,	EBP+REG_E,	S_0_1,	0 },
{ "%bp",	c_getreg,	EBP+REG_X,	S_0_1,	0 },
{ "%esp",	c_getreg,	ESP+REG_E,	S_0_1,	0 },
{ "%sp",	c_getreg,	ESP+REG_X,	S_0_1,	0 },
{ "%eip",	c_getreg,	EIP+REG_E,	S_0_1,	0 },
{ "%ip",	c_getreg,	EIP+REG_X,	S_0_1,	0 },
{ "%efl",	c_getreg,	EFL+REG_E,	S_0_1,	0 },
{ "%fl",	c_getreg,	EFL+REG_X,	S_0_1,	0 },
{ "%cs",	c_getreg,	CS+REG_X,	S_0_1,	0 },
{ "%ds",	c_getreg,	DS+REG_X,	S_0_1,	0 },
{ "%es",	c_getreg,	ES+REG_X,	S_0_1,	0 },
{ "%fs",	c_getreg,	FS+REG_X,	S_0_1,	0 },
{ "%gs",	c_getreg,	GS+REG_X,	S_0_1,	0 },
{ "%err",	c_getreg,	ERR+REG_E,	S_0_1,	0 },
{ "%trap",	c_getreg,	TRAPNO+REG_E,	S_0_1,	0 },
{ ".trap",	NULL,		(int)"%trap" },
{ "w%eax",	c_setreg,	EAX+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%ax",	c_setreg,	EAX+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%ah",	c_setreg,	EAX+REG_H,	S_1_0,	1,	T_NUMBER },
{ "w%al",	c_setreg,	EAX+REG_L,	S_1_0,	1,	T_NUMBER },
{ "w%ebx",	c_setreg,	EBX+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%bx",	c_setreg,	EBX+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%bh",	c_setreg,	EBX+REG_H,	S_1_0,	1,	T_NUMBER },
{ "w%bl",	c_setreg,	EBX+REG_L,	S_1_0,	1,	T_NUMBER },
{ "w%ecx",	c_setreg,	ECX+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%cx",	c_setreg,	ECX+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%ch",	c_setreg,	ECX+REG_H,	S_1_0,	1,	T_NUMBER },
{ "w%cl",	c_setreg,	ECX+REG_L,	S_1_0,	1,	T_NUMBER },
{ "w%edx",	c_setreg,	EDX+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%dx",	c_setreg,	EDX+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%dh",	c_setreg,	EDX+REG_H,	S_1_0,	1,	T_NUMBER },
{ "w%dl",	c_setreg,	EDX+REG_L,	S_1_0,	1,	T_NUMBER },
{ "w%edi",	c_setreg,	EDI+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%di",	c_setreg,	EDI+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%esi",	c_setreg,	ESI+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%si",	c_setreg,	ESI+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%ebp",	c_setreg,	EBP+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%bp",	c_setreg,	EBP+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%esp",	c_setreg,	ESP+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%sp",	c_setreg,	ESP+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%eip",	c_setreg,	EIP+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%ip",	c_setreg,	EIP+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%efl",	c_setreg,	EFL+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%fl",	c_setreg,	EFL+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%cs",	c_setreg,	CS+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%ds",	c_setreg,	DS+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%es",	c_setreg,	ES+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%fs",	c_setreg,	FS+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%gs",	c_setreg,	GS+REG_X,	S_1_0,	1,	T_NUMBER },
{ "w%err",	c_setreg,	ERR+REG_E,	S_1_0,	1,	T_NUMBER },
{ "w%trap",	c_setreg,	TRAPNO+REG_E,	S_1_0,	1,	T_NUMBER },
{ "cr0",	c_cr0,		0,	S_0_1,	0 },
{ "cr2",	c_cr2,		0,	S_0_1,	0 },
{ "cr3",	c_cr3,		0,	S_0_1,	0 },
{ "call",	c_call,		0,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "kvtop",	c_vtop,	(int)AS_KVIRT,	S_1_0,	1,	T_NUMBER },
{ "uvtop",	c_vtop,	(int)AS_UVIRT,	S_2_0,	2,	T_NUMBER, T_NUMBER },
{ "q",		c_quit,		0,	0,	0 },
{ "then",	c_then,		0,	0,	0 },
{ "endif",	c_endif,	0,	0,	0 },
{ "db0",	c_obsolete,	0,	0,	0 },
{ "db1",	c_obsolete,	0,	0,	0 },
{ "db2",	c_obsolete,	0,	0,	0 },
{ "db3",	c_obsolete,	0,	0,	0 },
{ "db6",	c_obsolete,	0,	0,	0 },
{ "db7",	c_obsolete,	0,	0,	0 },
{ "wdb0",	c_obsolete,	0,	0,	0 },
{ "wdb1",	c_obsolete,	0,	0,	0 },
{ "wdb2",	c_obsolete,	0,	0,	0 },
{ "wdb3",	c_obsolete,	0,	0,	0 },
{ "wdb6",	c_obsolete,	0,	0,	0 },
{ "wdb7",	c_obsolete,	0,	0,	0 },
{ "saveregs",	c_obsolete,	0,	0,	0 },
{ "useregs",	c_obsolete,	0,	0,	0 },
{ NULL }
};

		/* NOTE: suffix names must not be > 2 characters */
struct suffix_entry suffix_table[] = {
{ "k",		AS_KVIRT,	0,	0,		0 },
{ "p",		AS_PHYS,	0,	0,		0 },
{ "io", 	AS_IO,		0,	0,		0 },
{ "u",		AS_UVIRT,	0,	0,		SFX_NUM },
{ "b",		0,		1,	0,		0 },
{ "w",		0,		2,	0,		0 },
{ "l",		0,		4,	0,		0 },
{ "i",		0,		0,	BRK_INST,	0 },
{ "a",		0,		0,	BRK_ACCESS,	0 },
{ "m",		0,		0,	BRK_MODIFY,	0 },
{ "rs",		0,		0,	0,		SFX_NUM|SFX_RSET },
{ NULL }
};


dbstackcheck(down, up)
    unsigned	down, up;
{
    if (down > dbtos) {
	dberror("not enough items on stack");
	return 1;
    }
    if ((DBSTKSIZ - (dbtos + up)) <  1) {
	dberror("stack overflow");
	return 1;
    }
    return 0;
}

dbtypecheck(x, t)
    unsigned int x, t;
{
    if (dbstack[x].type == t)
	return 0;
    dberror("bad operand type");
    if (t > TYPEMAX || dbstack[x].type > TYPEMAX)
	dbprintf("*** logic error - illegal item type number in typecheck()\n");
    else
	dbprintf("(operand at stack location %x is a %s and should be a %s)\n",
		x, typename[dbstack[x].type], typename[t]);
    return 1;
}

dbmasktypecheck(x, t)
    unsigned int x, t;
{
    if (t & T_NUMBER && dbstack[x].type == NUMBER) return 0;
    if (t & T_NAME   && dbstack[x].type == NAME)   return 0;
    if (t & T_STRING && dbstack[x].type == STRING) return 0;
    dberror("bad operand type");
    if (!(t & (T_NUMBER|T_NAME|T_STRING)) || dbstack[x].type > TYPEMAX)
	dbprintf("*** logic error - illegal item type number in typecheck()\n");
    else
	dbprintf("(operand at stack location %x is a %s and should be a %s %s %s)\n",
		x, typename[dbstack[x].type],
		    t&T_NUMBER ? typename[NUMBER] : "",
		    t&T_NAME   ? typename[NAME]   : "",
		    t&T_STRING ? typename[STRING] : "" );
    return 1;
}

static int
dosuffix(s)
    char *s;
{
    register struct suffix_entry *p;
    register char	*s_c, *p_c;

    if (*s == '\0') {
	/* Special case; null suffix */
	db_cur_as = AS_KVIRT;
	return 1;
    }

    do {
	    for (p = suffix_table - 1;;) {
		if ((++p)->name == NULL) {
		    dberror("unknown suffix");
		    return 0;
		}
		for (s_c = s, p_c = p->name; *p_c != '\0'; ++s_c, ++p_c) {
		    if (*s_c != *p_c)
			break;
		}
		if (*p_c == '\0') {
			s = s_c;
			break;
		}
	    }

	    if (p->as)
		db_cur_as = p->as;
	    if (p->size)
		db_cur_size = p->size;
	    if (p->brk)
		cur_brk_type = p->brk;
	    if (p->flags & SFX_NUM) {
		register unsigned	n = 0;
		register char		*dp;
		register char		c;
		extern char		db_xdigit[];

		for (; (c = (*s | LCASEBIT)) != '\0'; ++s) {
		    for (dp = db_xdigit; *dp != '\0' && *dp != c; ++dp)
			;
		    if (*dp == '\0')
			break;
		    n = (n << 4) + (dp - db_xdigit);
		}
		if (p->flags & SFX_RSET) {
		    if (n >= NREGSET) {
			dberror("illegal register-set number");
			return 0;
		    }
		    db_cur_rset = n;
		} else
		    db_as_number = n;
	    }
    } while (*s != '\0');

    return 1;
}

static int
doname(name, check)
    char *name;     /* name to process */
    int check;      /* if set, just check for existence of name */
{
    register struct cmdentry *p;
    pte_t *pte;
    int save_pfn;
    char *s;
    u_int i;

    for (p = cmdtable; p->name != NULL; p++) {
	if (strcmp(name, p->name) == 0) {
		/* Found a command */
	    if (!check) {   /* check -> no action, testing */
		if (p->func == NULL) { /* alias */
		    dbcmdline((char *)p->arg);
		} else
		    /* Check for stack growth limits */
		if (!dbstackcheck(S_DOWN(p->stackcheck),S_UP(p->stackcheck))) {
			/* Check for correct parameter types */
		    for (i = 0; i < p->parmcnt; i++) {
			if (dbmasktypecheck(dbtos-1-i, p->parmtypes[i]))
			    return 1;
		    }
		    (*p->func) (p->arg);
		}
	    }
	    return 1;
	}
    }

    if (check)          /* check only applies to builtin operation names */
	return 0;

    for (i = 0; i < VARTBLSIZ; i++) {
	if ((s = variable[i].name) != NULL && strcmp(name, s) == 0) {
	    if (dbstackcheck(0, 1))
		return 1;
	    if (variable[i].type == VAR_MACRO) {
		dbcmdline(variable[i].item.value.string);
		return 0;
	    }
	    dbstack[dbtos] = variable[i].item;
	    if (dbstack[dbtos].type == STRING &&
		  (dbstack[dbtos].value.string = dbstrdup(dbstack[dbtos].value.string))
		     == NULL)
		return 1;
	    dbtos++;
	    return 0;
	}
    }

    if (dbextname(name))        /* try external names */
	return 0;

    dberror("name not found");
    return 1;
}

dbinterp(cmds)
    char *cmds;
{
    short t, do_suffix;
    struct item	suffix;
    char *p, *name;

    if (cmds)
	dbcmdline(cmds);

    do {
	db_cur_as = 0;
	db_cur_size = 0;
	db_cur_rset = 0;
	do_suffix = 0;
	cur_brk_type = BRK_INST;

	if ((t = dbgetitem(&dbstack[dbtos])) == NAME)
	    name = dbstrdup(dbstack[dbtos].value.string);
	if (dbverbose) {
	    dbprintf("%x: ", dbtos);
	    dbprintitem(&dbstack[dbtos], 0);
	}

	if (t == NAME && name[0] == '/' && name[1] != '\0') {
	    suffix = dbstack[dbtos];
	    t = (int) NULL;
	    goto got_suffix;
	}

	while (*(p = dbpeek()) == '/') {
	    /* handle suffix */
	    (void) dbgetitem(&suffix);
got_suffix:
	    do_suffix = 1;
	    if (!dosuffix(suffix.value.string + 1)) {
		t = (int) NULL;
		do_suffix = 0;
		break;
	    }
	}

	if (db_cur_as == 0)
	    db_cur_as = AS_KVIRT;
	if (db_cur_size == 0)
	    db_cur_size = 4;

	switch (t) {
	case EOF:
	    exit_db = 1;
	    break;
	case NUMBER:
	case STRING:
	    push(1);
	    break;
	case NAME:
	    if (doname(name, 0))
		do_suffix = 0;
 	    dbstrfree(name);
	    break;
	}

	if (do_suffix && !dbstackcheck(0, 1)) {
	    if (c_read(0) != -1)
		c_print(0);
	}
    } while (!exit_db);

    exit_db = 0;
}
