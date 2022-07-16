#ident	"@(#)kdb.c	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/kdb.c	1.3.1.5"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/user.h"
#include "sys/reg.h"
#include "sys/regset.h"

#ifdef MB2
#include "sys/mps.h"
#include "sys/immu.h"
#include "sys/ics.h"
extern ics_slot_t ics_slotmap[];
long FPIServChan;
unsigned char	FPIClients [ICS_MAX_SLOT] = {0};
#endif	/* MB2 */

#include "sys/cmn_err.h"
#include "sys/kdebugger.h"
#include "sys/xdebug.h"
#include "../db_as.h"

char *debugger_init;
unsigned dbg_putc_count;

gregset_t *regset[NREGSET];

#define ALIGN(p)  ((char *)(((ulong)(p) + sizeof(ulong) - 1) & \
						~(sizeof(ulong) - 1)))

/*
 * For machines with an interrupt button.
 */
kdbintr()
{
#ifdef MB2
	int i;
	unsigned char	val;

	if ((ics_myslotid()) == 0)  {
		for (i = 0; i < ICS_MAX_SLOT; i++) {
			if (FPIClients[i]) {
				val = ics_read(i,ICS_GeneralControl);
				ics_write(i,ICS_GeneralControl,(val | ICS_SWNMI_MASK));
			}
		}
	}
#endif
	(*cdebugger) (DR_USER, NO_FRAME);
}

#ifdef MB2
/*
 * For machines with an interrupt button that sends a message.
 */
kdbmesg(mbp)
mps_msgbuf_t *mbp;
{
	mps_free_msgbuf(mbp);
	(*cdebugger) (DR_USER, NO_FRAME);
}

#define	FPI_PORT_ID			(unsigned short)0x521
#define FPILocateServerC	0x01
#define FPIArmServerC 		0x02
#define NMISource			0x01

unsigned short	FPIServerFound; 
unsigned short	FPIServerArmed; 
int				do_fpi_locate ;
mb2socid_t 		FPIServer;

int
FPIServerMsg(tmsg)
struct msgbuf *tmsg;
{
	struct FPILocateServerResp {
		unchar Type;			/* FPILocateServerC */
		unchar Status;			/* FPISuccess, FPIFailure */
		unchar fill[18];
	}	*lmsgr;
	unsigned char	our_tid;
	mb2socid_t 	FPIClient;
	mps_msgbuf_t 	*mbp;

	lmsgr = (struct FPILocateServerResp *)mps_msg_getudp(tmsg);

	/*
	 * if we are in slot 0, this is a FPI Server request
	 */
	if ((ics_myslotid()) == 0)  {
		FPIClient = mps_mk_mb2socid(mps_msg_getsrcmid(tmsg), mps_msg_getsrcpid(tmsg));  
		if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)0))
			cmn_err (CE_PANIC,"kdb: FPIServerMsg cannot get message buffers\n");
		if (lmsgr->Type == FPILocateServerC) 
			lmsgr->Status = 0;
		else {
			if (lmsgr->Type == FPIArmServerC) {
				if (lmsgr->Status == 0)  /* actually command */
					FPIClients[mps_msg_getsrcmid(tmsg)] = 0;
				else {
					if (lmsgr->Status == NMISource)  {
						FPIClients[mps_msg_getsrcmid(tmsg)] = 1;
						lmsgr->Status = 0;
					}
					else
						lmsgr->Status = 0x80;
				}
			}
			else 
				lmsgr->Status = 0x80;
		}

		/* now send a reply */

		if ((our_tid = mps_get_tid(FPIServChan)) == 0)
			cmn_err(CE_NOTE, "kdb: FPIServerMsg cannot obtain tid\n");
		else {
			mps_mk_unsol(mbp, FPIClient, our_tid, 
			(unsigned char *)&lmsgr, sizeof(struct FPILocateServerResp));
			if (mps_AMPsend(FPIServChan, mbp) == -1L) {
				mps_msg_showmsg(mbp);
				cmn_err(CE_PANIC, 
					"kdb: FPIServerMsg send failure: chan=0x%x\n",FPIServChan);
			}
			mps_free_tid(FPIServChan, our_tid);
		}
	}
	else {
		if (lmsgr->Status == 0) {
			if (lmsgr->Type == FPILocateServerC) {
				FPIServerFound = 1;
				FPIServer = mps_mk_mb2socid(mps_msg_getsrcmid(tmsg), FPI_PORT_ID);  
			}
			if (lmsgr->Type == FPIArmServerC) {
				FPIServerArmed = 1;
			}
		}
		else
			cmn_err(CE_NOTE, "FPIlocate: status = 0x%x from FPI server\n", 
				lmsgr->Status);

		mps_free_msgbuf(tmsg);
	}
}

/*
 *	This is code to implement a broadcast locate protocol.
 *	This should ideally be modified to use the location service when
 *	that gets implemented.
*/

void
FPIlocate()
{	

	static struct FPILocArmServerReq	{
		unchar Type;			/* FPILocateServerC */
		unchar Command;			/* Command */
		unchar fill[18];
	} lmsg = { 0 };

	unsigned char		our_tid;
	unsigned short		start_chan; 
	unsigned short		loop_cnt;
	long				FPIChannel ;
	mps_msgbuf_t 			*mbp;

	FPIChannel = 0;
	FPIServerFound = 0;
	FPIServerArmed = 0;

	/* until TKI learns to allocate a free channel keep trying */

	for (start_chan = 0xA000; start_chan < 0xffff; start_chan++) { 
		FPIChannel = mps_open_chan(start_chan, FPIServerMsg, MPS_SRLPRIO);
		if (FPIChannel != -1L) 
			break;
	}
	if (FPIChannel == -1L) {
		cmn_err(CE_PANIC, "FPIlocate: cannot open broadcast channel\n");
	}

	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)0))
		cmn_err (CE_PANIC, "FPIlocate: cannot get message buffers\n");
	lmsg.Type = FPILocateServerC;
	mps_mk_brdcst(mbp, FPI_PORT_ID, (unsigned char *)&lmsg, sizeof(lmsg));
	
	if (mps_AMPsend(FPIChannel, mbp) == -1)
		cmn_err(CE_PANIC, "FPIlocate: Broadcast failure. mbp=%x\n", mbp);

	for (loop_cnt = 0; loop_cnt < 2000; loop_cnt++) {
		if (FPIServerFound)
			break;
		spinwait(1);
	}

	if (!FPIServerFound) {
		dri_printf("FPI server not found\n");
		return;
	}

	/* now arm the FPI server */

	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)0))
		cmn_err (CE_PANIC, "FPILocate: Cannot get message buffers\n");
	lmsg.Type = FPIArmServerC;
	lmsg.Command = NMISource;
	our_tid = mps_get_tid((long)FPIChannel); 
	mps_mk_unsol(mbp, FPIServer, our_tid, (unsigned char *)&lmsg, sizeof(lmsg));
	if (mps_AMPsend_rsvp(FPIChannel, mbp, 0, 0) == -1L) {
		mps_msg_showmsg(mbp);
		cmn_err(CE_PANIC, "FPILocate: send failure: chan=0x%x\n", FPIChannel);
	}
	for (loop_cnt = 0; loop_cnt < 1000; loop_cnt++) {
		if (FPIServerArmed)
			break;
		spinwait(1);			
	}
	mps_free_tid(FPIChannel, our_tid);
	mps_close_chan(FPIChannel);
	if (!FPIServerArmed) 
		dri_printf("FPI server not armed\n");
	dri_printf("FPI server found and initialized\n");
	return ;
}
#endif	/* MB2 */


/*
 * First-time initialization.
 */
kdbinit()
{
#ifdef MB2
unsigned short ics_nmi_reg;
#define CSM_SLOT 0
#define CSM_MDR 0x3a

	/* 
	 * First check if we are in slot 0, if so front panel interrupts 
	 * arrive directly on the master PIC
	 */

	do_fpi_locate = 0;

	if ((ics_myslotid()) == 0) 
		return;

	/* if a CSM/001 isn't in slot 0 */

	if (strcmp("CSM/001", ics_slotmap[CSM_SLOT].s_pcode) == 0){
		do_fpi_locate = 0;
		if(mps_open_chan(MPS_FP_PORT, kdbmesg, MPS_BLKPRIO) == -1) {
			cmn_err(CE_WARN,"db: Cannot initialize front panel interrupt.");
		} else {
			ics_write(CSM_SLOT,CSM_MDR,(unsigned char)mps_lhid());
		}
	} else { 
		do_fpi_locate = 1;
		ics_nmi_reg = ics_read(ics_myslotid(),ICS_NMIEnable);
		ics_write(ics_myslotid(),ICS_NMIEnable,(ics_nmi_reg | ICS_SWNMI_MASK));
	}
#endif	/* MB2 */

	/* Get initial command string from end of symbol table */
	debugger_init = symtable + ((ulong *)symtable)[0] + 4 * sizeof(ulong);
}

kdbstart()
{
#ifdef MB2
	/*
	 * Initialization that needs the message space.
	 */

	/*
	 * if we are in slot 0, open the FPI Server channel to listen to 
	 */
	if ((ics_myslotid()) == 0)  {
		FPIServChan = mps_open_chan(FPI_PORT_ID, FPIServerMsg, MPS_SRLPRIO);
		if (FPIServChan == -1L) 
			cmn_err(CE_PANIC,"kdb: kdbstart cannot open FPI Server channel\n");
		else
			cmn_err(CE_CONT, "kdb: Front Panel Interrupt server initialized\n");
	}
	else {
		if (do_fpi_locate)
			FPIlocate();
	}
#endif /* MB2 */

	/* Call debugger to execute initial command */
	if (debugger_init && *debugger_init)
		(*cdebugger) (DR_INIT, NO_FRAME);
}


static struct kdebugger *cur_debugger;

void
kdb_register(debugger)
	struct kdebugger *debugger;
{
	if ((debugger->kdb_next = debuggers) == NULL) {
		debuggers = debugger->kdb_next = debugger;
		cdebugger = (cur_debugger = debugger)->kdb_entry;
	} else
		(debugger->kdb_prev = debuggers->kdb_prev)->kdb_next = debugger;

	debuggers->kdb_prev = debugger;
}

void
kdb_next_debugger()
{
	cur_debugger = cur_debugger->kdb_next;
	cdebugger = cur_debugger->kdb_entry;
}

void
kdb_next_io()
{
	if (++cdbg_io >= &dbg_io[ndbg_io])
		cdbg_io = dbg_io;
}


/*
 * findsymname looks thru symtable to find the routine name which begins
 * closest to value.  The format of symtable is a list of
 * (long, null-terminated string) pairs; the list is sorted
 * on the longs.  symtable is populated by patching the kernel using
 * a program called unixsyms.
 */

char *
findsyminfo(value, loc_p, valid_p)
	ulong	value;
	ulong *	loc_p;
	uint *	valid_p;
{
	char *p = symtable + 2 * sizeof(ulong);
	char *namep, *oldnamep, *oldp = NULL;

	while (*(namep = p) != '\0') {
		while (*p++);		/* jump past string */
		if (*(ulong *)(p = ALIGN(p)) > value)
			break;
		oldp = p;
		oldnamep = namep;
		p += sizeof(ulong);
	}
	if (oldp != NULL) {
		namep = oldnamep;		/* name string */
		*loc_p = *(ulong *)oldp;	/* address */
	} else {
		namep = symtable + 2 * sizeof(ulong);
		for (p = namep; *p++;) ;
		*loc_p = *(ulong *)ALIGN(p);
	}
	*valid_p = (oldp != NULL);
	return namep;
}

char *
findsymname(value, tell)
	ulong	value;
	void	(*tell)();   /* function to print name and location (or NULL) */
{
	char *	p;
	ulong	loc;
	uint	valid;

	p = findsyminfo(value, &loc, &valid);
	if (tell)
		(*tell) (p, value, loc);
	return valid? p : NULL;
}

void
db_sym_and_off(addr, prf)
	ulong	addr;
	void	(*prf)();
{
	char *	p;
	ulong	sym_addr;
	uint	valid;

	p = findsyminfo(addr, &sym_addr, &valid);
	if (!valid) {
		(*prf) ("?0x%x?", addr);
		return;
	}
	(*prf) ("%s", p);
	if (addr != sym_addr)
		(*prf) ("+0x%lx", addr - sym_addr);
}

ulong
findsymval(value)
	ulong	value;
{
	ulong	loc;
	uint	valid;

	(void) findsyminfo(value, &loc, &valid);
	return valid? loc : (ulong)0;
}

/*
 * findsymaddr looks thru symtable to find the address of name.
 */

ulong
findsymaddr(name)
	char *name;
{
	char *p = symtable + 2 * sizeof(ulong);
	char *namep;

	while (*p != '\0') {
		for (namep = name; *namep && *p; namep++, p++) {
			if (*namep != *p)
				break;
		}
		if (*namep == '\0' && *p == '\0')
			return *(ulong *)ALIGN(p + 1);
		while (*p++);		/* jump past rest of name */
		p = ALIGN(p) + sizeof(ulong);	/* jump past address */
	}
	return 0L;
}


ulong	db_st_startfp;
ulong	db_st_startsp;
ulong	db_st_startpc;
ulong	db_st_offset;
uint	db_max_args = 3;

extern char	stext[], sdata[];

static void nframe(), iframe();
void db_frameregs();

#define G(x,i) (((unsigned long *)((long)(x) + db_st_offset))[i])
#define STACKLIM ((unsigned long)&u + KSTKSZ)
#define INSTACK(lower,value) ((lower) <= (value) && (value) < STACKLIM)
#define INTEXT(value) ((value) >= (ulong)stext && (value) < (ulong)sdata)


db_stacktrace(prf, dbg_entry)
	void	(*prf)();
	ulong	dbg_entry;
{
	ulong	pc;		/* program counter (eip) in current function */
	ulong	prevpc;		/* program counter (eip) in previous function */
	ulong	fp;		/* frame ptr (ebp) for current function */
	ulong	prevfp;		/* frame ptr (ebp) for previous function */
	ulong	sp;		/* stack ptr (esp) for current function */
	ulong	sptry;		/* trial stack ptr (esp) for current function */
	ulong	spclose;	/* stack ptr for closest bad direct call */
	ulong	ap;		/* argument ptr for current function */
	ulong	ap_lim;		/* end of argument ptr for current function */
	ulong	fn_entry;	/* entry point for current function */
	ulong	fn_start;	/* start of current function (from symbols) */
	as_addr_t pctry;	/* trial program counter for previous function */
	int	fake_fp;	/* flag: fp isn't really bp for this frame */
	int	skip_frames = 0;/* flag: skip frames inside of debugger */
	int	dist;		/* distance for out-of-range direct calls */
	int	bestdist;	/* best distance for o-o-r direct calls */
	int	ktrap;		/* interrupt/trap was from kernel mode */
	int	narg;		/* # arguments */
	int     rs;		/* register save set */
	char	tag;		/* call-type tag: '*' indirect or '~' close */

	if (db_st_startfp == 0) {
		db_st_offset = 0;
		db_get_stack();
	}
	fp = db_st_startfp;
	sp = db_st_startsp;
	pc = db_st_startpc;

	if (dbg_entry && prf) {
		skip_frames = 1;
		dbg_entry = findsymval(dbg_entry);
	}

	for (rs = 0; rs < NREGSET;)
		regset[rs++] = NULL;

	pctry.a_as = AS_KVIRT;
	rs = 0;
	while (INTEXT(pc) && INSTACK(sp - sizeof(ulong), fp)) {
		prevfp = G(fp, 0);
		if (INSTACK(fp, prevfp) ||
		    (prevfp == 0 &&
		     (!INSTACK(sp, (ulong)&((ulong *)fp)[UESP]) ||
		      !INSTACK(sp, G(fp, ESP))))) {
			/* look through the stack for a valid ret addr */
			sptry = spclose = 0;
			fn_start = findsymval(pc);
			/* first try at the next saved frame; if it matches
			   as a direct call, assume it's the right one */
			pctry.a_addr = G(fp + sizeof(ulong), 0);
			if (INTEXT(pctry.a_addr) &&
			    db_is_after_call(pctry, &fn_entry)) {
				if (fn_entry == 0)
					sptry = fp + sizeof(ulong);
				else if (fn_start <= fn_entry &&
					 fn_entry <= pc) {
					sp = fp + sizeof(ulong);
					goto found_frame;
				}
			}
			while (sp <= fp + sizeof(ulong)) {
				pctry.a_addr = G(sp, 0);
				if (INTEXT(pctry.a_addr) &&
				    db_is_after_call(pctry, &fn_entry)) {
					if (fn_entry == 0) {
						if (sptry == 0)
							sptry = sp;
					} else {
						if (fn_start <= fn_entry &&
						    fn_entry <= pc)
							break;
						dist = fn_entry - fn_start;
						if (dist < 0)
							dist = -dist;
						if (spclose == 0 ||
						    dist < bestdist) {
							spclose = sp;
							bestdist = dist;
						}
					}
				}
				sp += sizeof(ulong);
			}
found_frame:
			tag = ' ';
			if (sp > fp + sizeof(ulong)) {
				if ((sp = sptry) == 0 &&
				    (sp = spclose) == 0)
					fn_entry = prevpc = ap = 0;
				else {
					fn_entry = pc;
					tag = (sptry? '*' : '~');
				}
			}
			fake_fp = 0;
			if (fn_entry != 0) {
				prevpc = G(sp, 0);
				if (sp < fp) {
					prevfp = fp;
					fp = sp - sizeof(ulong);
					fake_fp = 1;
				}
				ap = sp + sizeof(ulong);
				if ((ap_lim = prevfp) < ap || ap_lim > STACKLIM)
					ap_lim = STACKLIM;
				narg = (ap_lim - ap) / sizeof(ulong);
				if (narg > db_max_args)
					narg = db_max_args;
			}
			if (prf) {
				if (skip_frames) {
				    if (fn_start == dbg_entry) {
					(*prf) ("DEBUGGER ENTERED FROM ");
					if (ap) {
					    switch (*(int *)ap) {
					    case DR_USER:
					    case DR_SECURE_USER:
						(*prf) ("USER REQUEST");
						break;
					    case DR_BPT1:
					    case DR_BPT3:
						(*prf) ("BREAKPOINT");
						break;
					    case DR_STEP:
						(*prf) ("SINGLE-STEP");
						break;
					    case DR_PANIC:
						(*prf) ("PANIC");
						break;
					    default:
						db_sym_and_off(prevpc, prf);
						break;
					    }
					}
					(*prf) ("\n");
					skip_frames = 0;
				    }
				} else
					nframe(fn_entry, tag,
						fp, fake_fp,
						prevpc, ap, narg, prf);
			}
			pc = prevpc;
			fp = prevfp;
			sp += sizeof(ulong);
		} else {
			/*
			 * trap/interrupt stack frame
			 */
			skip_frames = 0;
			ktrap = !(G(fp, EFL) & PS_VM) && !(G(fp, CS) & SEL_LDT);
			if (prf)
				iframe(pc, fp, rs, prf, ktrap);
			regset[rs++] = (gregset_t *)&G(fp, 0);
			if (rs >= NREGSET)
				rs = NREGSET - 1;
			pc = G(fp, EIP);
			sp = (ulong)&((ulong *)fp)[ktrap? UESP : SS + 1];
			fp = G(fp, EBP);
		}
	}

	db_st_startfp = 0;
}

#define LINE_WIDTH	80
#define FUNC_WIDTH	(LINE_WIDTH - 1 - 28)

static void
nframe(pc, tag, fp, fake_fp, prevpc, ap, narg, prf)
	ulong		pc, fp, prevpc, ap;
	unsigned	fake_fp, narg;
	char		tag;
	void		(*prf)();
{
	dbg_putc_count = 0;

	(*prf) ("%c", tag);
	db_sym_and_off(pc, prf);
	(*prf) ("(");
	while (ap && narg-- > 0) {
		(*prf) ("%x", G(ap, 0));
		ap += sizeof(ulong);
		if (narg > 0)
			(*prf) (" ");
	}
	(*prf) (")");
	while (dbg_putc_count < FUNC_WIDTH)
		(*prf) (".");

	if (fake_fp)
		(*prf) (".(ebp:%08x) ", fp);
	else
		(*prf) ("..ebp:%08x  ", fp);
	if (prevpc)
		(*prf) ("ret:%08x\n", prevpc);
	else
		(*prf) ("\n");
}

static void
iframe(pc, fp, rs, prf, ktrap)
	ulong		pc, fp;
	unsigned	rs;
	void		(*prf)();
	int		ktrap;
{
	ulong		fn_start;
	extern int	cmnint(), cmntrap(), sys_call(), sig_clean();

	fn_start = findsymval(pc);
	if (fn_start == (ulong)cmnint)
		(*prf) ("INTERRUPT 0x%x", G(fp, TRAPNO));
	else if (fn_start == (ulong)cmntrap)
		(*prf) ("TRAP 0x%x (err 0x%x)", G(fp, TRAPNO), G(fp, ERR));
	else if (fn_start == (ulong)sys_call)
		(*prf) ("SYSTEM CALL");
	else if (fn_start == (ulong)sig_clean)
		(*prf) ("SIGNAL RETURN");
	else {
		(*prf) ("?TRAP TO ");
		db_sym_and_off(pc, prf);
		(*prf) (" (trap 0x%x, err 0x%x)", G(fp, TRAPNO), G(fp, ERR));
	}
	(*prf) (" from %x:%x (ebp:%x",
			G(fp, CS) & 0xFFFF, G(fp, EIP), fp);
	if (ktrap)
		(*prf) (")\n");
	else {
		(*prf) (", ss:esp: %x:%x)\n",
				G(fp, SS) & 0xFFFF, G(fp, UESP));
	}
	db_frameregs(fp, rs, prf);
}

void
db_frameregs(fp, rs, prf)
	ulong		fp;
	unsigned	rs;
	void		(*prf)();
{
	(*prf) ("   eax:%8x ebx:%8x ecx:%8x edx:%8x efl:%8x ds:%4x\n",
		G(fp, EAX), G(fp, EBX), G(fp, ECX), G(fp, EDX),
		G(fp, EFL), G(fp, DS) & 0xFFFF);
	(*prf) ("   esi:%8x edi:%8x esp:%8x ebp:%8x regset:%2d    es:%4x\n",
		G(fp, ESI), G(fp, EDI), G(fp, ESP) + ESP_OFFSET,
		G(fp, EBP), rs, G(fp, ES) & 0xFFFF);
}


int
db_is_after_call(addr, dst_addr_p)
	as_addr_t	addr;
	caddr_t		*dst_addr_p;
{
	u_char	opc[7], *opp;

	addr.a_addr -= 7;
	if (db_read(addr, opc, sizeof(opc)) == -1)
		return 0;
	addr.a_addr += 7;
	if (opc[2] == OPC_CALL_REL) {
		*dst_addr_p = addr.a_addr + *(caddr_t *)(opc + 3);
		if (INTEXT((ulong)*dst_addr_p))
			return 1;
	}
	if (opc[0] == OPC_CALL_DIR) {
		*dst_addr_p = *(caddr_t *)(opc + 1);
		if (INTEXT((ulong)*dst_addr_p))
			return 1;
	}
	for (opp = opc + 5; opp >= opc; opp--) {
		if (*opp != OPC_CALL_IND)
			continue;
		if ((opp[1] & 0x38) == 0x10) {
			*dst_addr_p = (caddr_t)0;
			return 1;
		}
	}
	return 0;
}
