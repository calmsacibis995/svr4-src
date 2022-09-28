/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:tty.c	1.8.10.1"

/*
 * This file contains code for the crash function tty.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <ctype.h>
#ifdef i386
#define MAINSTORE 0
#else
#include <sys/sbd.h>
#endif
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "string.h"
#define _KERNEL
#include "sys/stream.h"
#undef _KERNEL
#include "sys/termio.h"
#include "sys/strtty.h"
#include "sys/termio.h"
#include "crash.h"
#include "sys/pp_dep.h"
#include "sys/queue.h"
#include "sys/strppc.h"
#include "sys/eucioctl.h"
#include "sys/ldterm.h"
#include "sys/kd.h"
#include "sys/proc.h"
#define _KERNEL	/* Sigh... */
#include "sys/xque.h"
#include "sys/ws/ws.h"
#undef _KERNEL


short print_header;	
short is_workstation;
wstation_t ws;


/* get address of strtty structure */
long
getaddr(type)
char *type;
{
	struct syment *sp;
	char temp[32];

	/* Check for workstation model */
	strcpy(temp, type);
	strcat(temp, "ws");
	if (islower(temp[0]))
		temp[0] = toupper(temp[0]);
	if ((sp = symsrch(temp)) != NULL) {
		is_workstation = 1;
		readmem((long)sp->n_value,1,-1,(char *)&ws,sizeof ws, temp);
		return(sp->n_value);
	}
	is_workstation = 0;

	strcpy(temp, type);
	strcat(temp, "_tty");
	if ((sp = symsrch(temp)) == NULL)
		error("tty struct for %s not found in symbol table\n", type);
	return(sp->n_value);
}

/* get tty count */
int
getcount(type)
char *type;
{
	struct syment *sp;
	char temp[32];
	int count = 0;

	/* Unfortunately, the naming conventions aren't standardized.
	   Try a bunch of different names. */

	/* Check for workstation model */
	if (is_workstation) {
		/* This doesn't work yet:
			return ws.w_maxchan;
		   so we have to hard code it for now. */
		return WS_MAXCHAN;
	}

	/* Try xxx_cnt [This is what *should* be used] */
	strcpy(temp, type);
	strcat(temp, "_cnt");
	if ((sp = symsrch(temp)) != NULL)
		goto found;

	/* Try xxx_count */
	strcpy(temp, type);
	strcat(temp, "_count");
	if ((sp = symsrch(temp)) != NULL)
		goto found;

	/* Try num_xxx */
	strcpy(temp, "num_");
	strcat(temp, type);
	if ((sp = symsrch(temp)) != NULL)
		goto found;

	/* Try numxxx */
	strcpy(temp, "num");
	strcat(temp, type);
	if ((sp = symsrch(temp)) != NULL)
		goto found;

	/* Try NXXX */
	strcpy(temp, "N");
	strcat(temp, type);
	{ char *p;
		for (p = temp; *p != '\0'; ++p) {
			if (islower(*p))
				*p = toupper(*p);
		}
	}
	if ((sp = symsrch(temp)) != NULL)
		goto found;

	/* Try NUMXXX */
	strcpy(temp, "NUM");
	strcat(temp, type);
	{ char *p;
		for (p = temp; *p != '\0'; ++p) {
			if (islower(*p))
				*p = toupper(*p);
		}
	}
	if ((sp = symsrch(temp)) != NULL)
		goto found;

	printf("count of %s not found; assuming 1\n", type);
	return 1;
found:
	readmem((long)sp->n_value,1,-1,(char *)&count,sizeof count, "tty_cnt");
	return count;
}

/* get arguments for tty function */
int
gettty()
{
	int slot = -1;
	int i;
	int full = 0;
	int all = 0;
	int phys = 0;
	int count = 0;
	int line_disp = 0;
	char *type = "";
	char *ld = "";
	long addr = -1;
	long arg2 = -1;
	long arg1 = -1;
	int c;
	char *sheading = "SLOT     T_IN    T_OUT      RDQ MIN STATE\n";

	optind = 1;
	while( ( c = getopt( argcnt, args, "efplw:t:")) != EOF) {
		switch( c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 't' :	type = optarg;
					break;
			case 'w' :	redirect();
					break;
			case 'l' :	line_disp = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if ((strcmp(type, "") == 0) && !args[optind]) {
		/* System defaults */
#ifdef AT386
		addr = getaddr("kd");
		count = getcount("kd");
		print_header = 0;
		fprintf(fp, "kd TABLE SIZE = %d\n", count);

		if ( !full)
			fprintf( fp, "%s", sheading);

		for ( slot = 0; slot < count; slot++)
			prstty( all, full, slot, phys, addr, "", line_disp, sheading);

		addr = getaddr("asy");
		count = getcount("asy");
		print_header = 0;
		fprintf(fp, "\nasy TABLE SIZE = %d\n", count);

		if ( !full)
			fprintf( fp, "%s", sheading);

		for (slot = 0; slot < count; slot++)
			prstty( all, full, slot, phys, addr, "", line_disp, sheading);
#endif /* AT386 */
#if defined(MB1) || defined(MB2)
		addr = getaddr("8251");
		count = getcount("8251");
		print_header = 0;
		fprintf(fp, "8251 TABLE SIZE = %d\n", count);

		if ( !full)
			fprintf( fp, "%s", sheading);

		for ( slot = 0; slot < count; slot++)
			prstty( all, full, slot, phys, addr, "", line_disp, sheading);
#endif /* MB1 || MB2 */
	} else {
		if (strcmp(type, "") != 0) {
			addr = getaddr( type);
			count = getcount( type);

			fprintf(fp,"%s TABLE SIZE = %d\n",type,count);
		} else
			is_workstation = 0;
		if(!full) {
			fprintf(fp,"%s",sheading);
			print_header = 0;
		} else
			print_header = 1;
		if(args[optind]) {
			all = 1;
			do {
				getargs(count,&arg1,&arg2);
				if ( arg1 == -1) 
					continue;
				if ( arg2 != -1)
					for( slot = arg1; slot <= arg2; slot++)
						prstty( all, full, slot, phys, addr, type, line_disp, sheading);
				else {
					if ((unsigned)arg1 < (unsigned)count)
						slot = arg1;
					else addr = arg1;
					prstty( all, full, slot, phys, addr, type, line_disp, sheading);
				}
			slot = arg1 = arg2 = -1;
			if ( strcmp(type,"") != 0 )
				addr = getaddr(type);
			} while(args[++optind]);
		}
		else
			for(slot = 0; slot < count; slot++)
				prstty( all, full, slot, phys, addr, "", line_disp, sheading);
	}
}

/* print streamstty table */
prstty( all, full, slot, phys, addr, type, ld_type, heading)
int all,full,slot,phys;
long addr;
char *type;
int ld_type;	/* the line disicipline */
char *heading;
{
	struct strtty tbuf;
	struct queue q;
	struct syment	*symsrch();

	long base;

	int count, idx;

	if (is_workstation) {
		channel_t	*chp;
		readmem((long)(ws.w_chanpp + slot),1,-1,(char *)&chp,sizeof chp,
				"workstation channel pointer");
		if (chp == NULL) {
			memset((char *)&tbuf, 0, sizeof tbuf);
			goto got_tbuf;
		}
		addr = (long)&chp->ch_strtty;
		phys = 0;
		idx = -1;
	} else
		idx = slot;
	if ((phys || !Virtmode) && (idx == -1))
		readmem(addr&~MAINSTORE,0,-1,(char *)&tbuf,sizeof tbuf,
			"tty structure");
	else if (idx == -1)
		readmem((long)addr,1,-1,(char *)&tbuf,
			sizeof tbuf,"tty structure");
	else
		readmem((long)(addr + idx * sizeof tbuf),1,-1,
			(char *)&tbuf,sizeof tbuf,"tty structure");
got_tbuf:

	if ( !tbuf.t_state && !all)
		return;

	if ( full || ( ld_type && print_header)) {
		print_header = 0;
		fprintf( fp, "%s", heading);
	}

	if ( ( slot == -1) && ( strcmp( type, "") != 0)) {
		base = getaddr( type);
		count = getcount( type);
		slot = getslot( addr, base, sizeof tbuf, phys, count);
	}

	if ( slot == -1)
		fprintf( fp, "  - ");
	else
		fprintf( fp, "%4d", slot);

	fprintf( fp, "%9x", tbuf.t_in);
	fprintf( fp, "%9x", tbuf.t_out);
	fprintf( fp, "%9x", tbuf.t_rdqp);
	fprintf( fp, "%4u", tbuf.t_dev);
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		tbuf.t_state & TIMEOUT ? " tout" : "",
		tbuf.t_state & WOPEN ? " wtop" : "",
		tbuf.t_state & ISOPEN ? " isop" : "",
		tbuf.t_state & TBLOCK ? " tblk" : "",
		tbuf.t_state & CARR_ON ? " carr" : "",
		tbuf.t_state & WIOC ? " wioc" : "",
		tbuf.t_state & WGETTY ? " wget" : "",
		tbuf.t_state & TTSTOP ? " stop" : "",
		tbuf.t_state & EXTPROC ? " xprc" : "",
		tbuf.t_state & TACT ? " tact" : "",
		tbuf.t_state & CLESC ? " lesc" : "",
		tbuf.t_state & RTO ? " rto" : "",
		tbuf.t_state & TTIOW ? " ttiw" : "",
		tbuf.t_state & TTXON ? " ttxn" : "",
		tbuf.t_state & TTXOFF ? " ttxf" : "",
		tbuf.t_state & BUSY ? " busy" : "");

	if ( !full) {
		if ( ld_type && tbuf.t_state) {
			readmem( (long)(tbuf.t_rdqp), 1, -1, (char *)&q, sizeof( struct queue), "");
			if ( !prsldterm( full, q.q_next))
					print_header = 1;
		}
		return;
	}

	fprintf(fp,"\n\tt_dstat: %s%s%s%s%s%s%s%s\n",
		(tbuf.t_dstat&SETOPT) ? " setopt" : "",
		(tbuf.t_dstat&SPLITMSG) ? " splitm" : "",
		(tbuf.t_dstat&WENTRY) ? " wentry" : "",
		(tbuf.t_dstat&SUPBUF) ? " supbuf" : "",
		(tbuf.t_dstat&WRTQDRAIN) ? " wrtqdr" : "",
		(tbuf.t_dstat&CLDRAIN) ? " cldran" : "",
		(tbuf.t_dstat&OPDRAIN) ? " opdran" : "",
		(tbuf.t_dstat&WBREAK) ? " wbreak" : "");

	fprintf(fp,"\tintr: ");
	putch((char)tbuf.t_cc[VINTR]);
	fprintf(fp,"\tquit: ");
	putch((char)tbuf.t_cc[VQUIT]);
	fprintf(fp,"\terase: ");
	putch((char)tbuf.t_cc[VERASE]);
	fprintf(fp,"\tkill: ");
	putch((char)tbuf.t_cc[VKILL]);
	fprintf(fp,"\n");
	fprintf(fp,"\teof/vmin: ");
	putch((char)tbuf.t_cc[VEOF]);
	fprintf(fp,"\teol/vtim: ");
	putch((char)tbuf.t_cc[VEOL]);
	fprintf(fp,"\tveol2: ");
	putch((char)tbuf.t_cc[VEOL2]);
	fprintf(fp,"\tswtch: ");
	putch((char)tbuf.t_cc[VSWTCH]);
	fprintf(fp,"\n");
	fprintf(fp,"\tstart: ");
	putch((char)tbuf.t_cc[VSTART]);
	fprintf(fp,"\tstop: ");
	putch((char)tbuf.t_cc[VSTOP]);
	fprintf(fp,"\tsusp: ");
	putch((char)tbuf.t_cc[VSUSP]);
	fprintf(fp,"\tdsusp: ");
	putch((char)tbuf.t_cc[VDSUSP]);
	fprintf(fp,"\n");
	fprintf(fp,"\treprint: ");
	putch((char)tbuf.t_cc[VREPRINT]);
	fprintf(fp,"\tdiscard: ");
	putch((char)tbuf.t_cc[VDISCARD]);
	fprintf(fp,"\twerase: ");
	putch((char)tbuf.t_cc[VWERASE]);
	fprintf(fp,"\tlnext: ");
	putch((char)tbuf.t_cc[VLNEXT]);
	fprintf(fp,"\n");

	fprintf(fp,"\tt_iflag: %s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(tbuf.t_iflag&IGNBRK) ? " ignbrk" : "",
		(tbuf.t_iflag&BRKINT) ? " brkint" : "",
		(tbuf.t_iflag&IGNPAR) ? " ignpar" : "",
		(tbuf.t_iflag&PARMRK) ? " parmrk" : "",
		(tbuf.t_iflag&INPCK)  ? " inpck"  : "",
		(tbuf.t_iflag&ISTRIP) ? " istrip" : "",
		(tbuf.t_iflag&INLCR)  ? " inlcr"  : "",
		(tbuf.t_iflag&IGNCR)  ? " icncr"  : "",
		(tbuf.t_iflag&ICRNL)  ? " icrnl"  : "",
		(tbuf.t_iflag&IUCLC)  ? " iuclc"  : "",
		(tbuf.t_iflag&IXON)   ? " ixon"   : "",
		(tbuf.t_iflag&IXANY)  ? " ixany"  : "",
		(tbuf.t_iflag&IXOFF)  ? " ixoff"  : "");
	fprintf(fp,"\tt_oflag: %s%s%s%s%s%s%s%s",
		(tbuf.t_oflag&OPOST)  ? " opost"  : "",
		(tbuf.t_oflag&OLCUC)  ? " olcuc"  : "",
		(tbuf.t_oflag&ONLCR)  ? " onlcr"  : "",
		(tbuf.t_oflag&OCRNL)  ? " ocrnl"  : "",
		(tbuf.t_oflag&ONOCR)  ? " onocr"  : "",
		(tbuf.t_oflag&ONLRET) ? " onlret" : "",
		(tbuf.t_oflag&OFILL)  ? " ofill"  : "",
		(tbuf.t_oflag&OFDEL)  ? " ofpel"  : "");
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(tbuf.t_oflag&NLDLY)  == NL0  ? " nl0"  : "",
		(tbuf.t_oflag&NLDLY)  == NL1  ? " nl1"  : "",
		(tbuf.t_oflag&CRDLY)  == CR0  ? " cr0"  : "",
		(tbuf.t_oflag&CRDLY)  == CR1  ? " cr1"  : "",
		(tbuf.t_oflag&CRDLY)  == CR2  ? " cr2"  : "",
		(tbuf.t_oflag&CRDLY)  == CR3  ? " cr3"  : "",
		(tbuf.t_oflag&TABDLY) == TAB0 ? " tab0" : "",
		(tbuf.t_oflag&TABDLY) == TAB1 ? " tab1" : "",
		(tbuf.t_oflag&TABDLY) == TAB2 ? " tab2" : "",
		(tbuf.t_oflag&TABDLY) == TAB3 ? " tab3" : "",
		(tbuf.t_oflag&BSDLY)  == BS0  ? " bs0"  : "",
		(tbuf.t_oflag&BSDLY)  == BS1  ? " bs1"  : "",
		(tbuf.t_oflag&VTDLY)  == VT0  ? " vt0"  : "",
		(tbuf.t_oflag&VTDLY)  == VT1  ? " vt1"  : "",
		(tbuf.t_oflag&FFDLY)  == FF0  ? " ff0"  : "",
		(tbuf.t_oflag&FFDLY)  == FF1  ? " ff1"  : "");
	fprintf(fp,"\tt_cflag: %s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		(tbuf.t_cflag&CBAUD)==B0    ? " b0"    : "",
		(tbuf.t_cflag&CBAUD)==B50   ? " b50"   : "",
		(tbuf.t_cflag&CBAUD)==B75   ? " b75"   : "",
		(tbuf.t_cflag&CBAUD)==B110  ? " b110"  : "",
		(tbuf.t_cflag&CBAUD)==B134  ? " b134"  : "",
		(tbuf.t_cflag&CBAUD)==B150  ? " b150"  : "",
		(tbuf.t_cflag&CBAUD)==B200  ? " b200"  : "",
		(tbuf.t_cflag&CBAUD)==B300  ? " b300"  : "",
		(tbuf.t_cflag&CBAUD)==B600  ? " b600"  : "",
		(tbuf.t_cflag&CBAUD)==B1200 ? " b1200" : "",
		(tbuf.t_cflag&CBAUD)==B1800 ? " b1800" : "",
		(tbuf.t_cflag&CBAUD)==B2400 ? " b2400" : "",
		(tbuf.t_cflag&CBAUD)==B4800 ? " b4800" : "",
		(tbuf.t_cflag&CBAUD)==B9600 ? " b9600" : "",
		(tbuf.t_cflag&CBAUD)==B19200 ? " b19200" : "");
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s\n",
		(tbuf.t_cflag&CSIZE)==CS5   ? " cs5"   : "",
		(tbuf.t_cflag&CSIZE)==CS6   ? " cs6"   : "",
		(tbuf.t_cflag&CSIZE)==CS7   ? " cs7"   : "",
		(tbuf.t_cflag&CSIZE)==CS8   ? " cs8"   : "",
		(tbuf.t_cflag&CSTOPB) ? " cstopb" : "",
		(tbuf.t_cflag&CREAD)  ? " cread"  : "",
		(tbuf.t_cflag&PARENB) ? " parenb" : "",
		(tbuf.t_cflag&PARODD) ? " parodd" : "",
		(tbuf.t_cflag&HUPCL)  ? " hupcl"  : "",
		(tbuf.t_cflag&CLOCAL) ? " clocal" : "");
	fprintf(fp,"\tt_lflag: %s%s%s%s%s%s%s%s\n",
		tbuf.t_lflag & ISIG   ? " isig"   : "",
		tbuf.t_lflag & ICANON ? " icanon" : "",
		tbuf.t_lflag & XCASE  ? " xcase"  : "",
		tbuf.t_lflag & ECHO   ? " echo"   : "",
		tbuf.t_lflag & ECHOE  ? " echoe"  : "",
		tbuf.t_lflag & ECHOK  ? " echok"  : "",
		tbuf.t_lflag & ECHONL ? " echonl" : "",
		tbuf.t_lflag & NOFLSH ? " noflsh" : "");

	fprintf( fp, "\tt_line %x\n", tbuf.t_line);
	fprintf( fp, "\tPointer to ioctl block %x\n", tbuf.t_ioctlp);
	fprintf( fp, "\tPointer to large input block %x\n", tbuf.t_lbuf);
	fprintf(fp,"\n");

	if ( ld_type) {
		readmem( (long)(tbuf.t_rdqp), 1, -1, (char *)&q, sizeof( struct queue), "");
		if ( !prsldterm( full, q.q_next))
			print_header = 1;
	}

	return;
}
/*
 *  print streams line discipline(ldterm) module information 
 *  Return 0 if there is Line discipline information to be printed
 *	   1 if there isn't line discipline information to be printed
 */
prsldterm( all, addr)
int all;
long addr;
{
	char *heading = "\tMODNAME   MODID  MESSAGE   ENDMSG   MSGLEN STATE\n";
	char mname[9];	/* Buffer for the module name */

	struct ldterm_mod ldterm;
	struct queue q;
	struct qinit qinfo;
	struct module_info minfo;


	/*
	 * Wade through the link-lists to extract the line discipline
	 * name and id
	 */
	readmem( addr, 1, -1, (char *)&q, sizeof( struct queue), "");
	/*
	 * q_next is zero at the stream head, i.e. no line disicipline
	 * pushed
	 */
	if ( !q.q_next)
		return( 0);
	readmem( (long)(q.q_qinfo), 1, -1, (char *)&qinfo, sizeof( struct qinit), "");
	readmem( (long)(qinfo.qi_minfo), 1, -1, (char *)&minfo, sizeof( struct module_info), "");

	readmem( (long)(minfo.mi_idname), 1, -1, mname, 8, "");
	mname[8] = '\0';

	readmem( (long)(q.q_ptr), 1, -1, (char *)&ldterm, sizeof( struct ldterm_mod), "");

	fprintf( fp, "%s", heading);
	fprintf( fp, "\t%-9s%6d%9x%9x%9x ", mname, minfo.mi_idnum,
		ldterm.t_message, ldterm.t_endmsg, ldterm.t_msglen);

	fprintf( fp, "%s%s%s%s%s%s%s\n",
		ldterm.t_state & TS_XCLUDE ? " xclud" : "",
		ldterm.t_state & TS_TTSTOP ? " tstop" : "",
		ldterm.t_state & TS_TBLOCK ? " blk" : "",
		ldterm.t_state & TS_QUOT ? " quot" : "",
		ldterm.t_state & TS_ERASE ? " erase" : "",
		ldterm.t_state & TS_SLNCH ? " slnch" : "",
		ldterm.t_state & TS_PLNCH ? " plnch" : "",
		ldterm.t_state & TS_TTCR ? " ttcr" : "",
		ldterm.t_state & TS_NOCANON ? " nocan" : "",
		ldterm.t_state & TS_RESCAN ? " rescn" : "",
		ldterm.t_state & TS_RTO ? " rto" : "",
		ldterm.t_state & TS_TACT ? " act" : "",
		ldterm.t_state & TS_MEUC ? " meuc" : "",
		ldterm.t_state & TS_WARNED ? " warn" : "");

	if ( !all)
		return( 0);

	fprintf( fp, "\tpointer to echo buffer %9x\n", ldterm.t_echomp);
	fprintf( fp, "\toutput row %d\toutput column %d\n", ldterm.t_line, ldterm.t_col);
	fprintf( fp, "\tnumber of chars echoed %d at column postion %d\n", ldterm.t_rocount, ldterm.t_rocol);
	fprintf( fp, "\tnumber of bytes requested by M_READ %d\n", ldterm.t_rd_request);
	fprintf( fp, "\n\tEffective termios\n"); 

	fprintf(fp,"\tintr: ");
	putch((char)ldterm.t_modes.c_cc[VINTR]);
	fprintf(fp,"\tquit: ");
	putch((char)ldterm.t_modes.c_cc[VQUIT]);
	fprintf(fp,"\terase: ");
	putch((char)ldterm.t_modes.c_cc[VERASE]);
	fprintf(fp,"\tkill: ");
	putch((char)ldterm.t_modes.c_cc[VKILL]);
	fprintf(fp,"\n");
	fprintf(fp,"\teof/vmin: ");
	putch((char)ldterm.t_modes.c_cc[VEOF]);
	fprintf(fp,"\teol/vtime: ");
	putch((char)ldterm.t_modes.c_cc[VEOL]);
	fprintf(fp,"\teol2: ");
	putch((char)ldterm.t_modes.c_cc[VEOL2]);
	fprintf(fp,"\tswtch: ");
	putch((char)ldterm.t_modes.c_cc[VSWTCH]);
	fprintf(fp,"\n");
	fprintf(fp,"\tstart: ");
	putch((char)ldterm.t_modes.c_cc[VSTART]);
	fprintf(fp,"\tstop: ");
	putch((char)ldterm.t_modes.c_cc[VSTOP]);
	fprintf(fp,"\tsusp: ");
	putch((char)ldterm.t_modes.c_cc[VSUSP]);
	fprintf(fp,"\tdsusp: ");
	putch((char)ldterm.t_modes.c_cc[VDSUSP]);
	fprintf(fp,"\n");
	fprintf(fp,"\treprint: ");
	putch((char)ldterm.t_modes.c_cc[VREPRINT]);
	fprintf(fp,"\tdiscard: ");
	putch((char)ldterm.t_modes.c_cc[VDISCARD]);
	fprintf(fp,"\twerase: ");
	putch((char)ldterm.t_modes.c_cc[VWERASE]);
	fprintf(fp,"\tlnext: ");
	putch((char)ldterm.t_modes.c_cc[VLNEXT]);
	fprintf(fp,"\n");

	fprintf(fp,"\tiflags: %s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(ldterm.t_modes.c_iflag&IGNBRK) ? " ignbrk" : "",
		(ldterm.t_modes.c_iflag&BRKINT) ? " brkint" : "",
		(ldterm.t_modes.c_iflag&IGNPAR) ? " ignpar" : "",
		(ldterm.t_modes.c_iflag&PARMRK) ? " parmrk" : "",
		(ldterm.t_modes.c_iflag&INPCK)  ? " inpck"  : "",
		(ldterm.t_modes.c_iflag&ISTRIP) ? " istrip" : "",
		(ldterm.t_modes.c_iflag&INLCR)  ? " inlcr"  : "",
		(ldterm.t_modes.c_iflag&IGNCR)  ? " icncr"  : "",
		(ldterm.t_modes.c_iflag&ICRNL)  ? " icrnl"  : "",
		(ldterm.t_modes.c_iflag&IUCLC)  ? " iuclc"  : "",
		(ldterm.t_modes.c_iflag&IXON)   ? " ixon"   : "",
		(ldterm.t_modes.c_iflag&IXANY)  ? " ixany"  : "",
		(ldterm.t_modes.c_iflag&IXOFF)  ? " ixoff"  : "");

	fprintf(fp,"\toflags: %s%s%s%s%s%s%s%s",
		(ldterm.t_modes.c_oflag&OPOST)  ? " opost"  : "",
		(ldterm.t_modes.c_oflag&OLCUC)  ? " olcuc"  : "",
		(ldterm.t_modes.c_oflag&ONLCR)  ? " onlcr"  : "",
		(ldterm.t_modes.c_oflag&OCRNL)  ? " ocrnl"  : "",
		(ldterm.t_modes.c_oflag&ONOCR)  ? " onocr"  : "",
		(ldterm.t_modes.c_oflag&ONLRET) ? " onlret" : "",
		(ldterm.t_modes.c_oflag&OFILL)  ? " ofill"  : "",
		(ldterm.t_modes.c_oflag&OFDEL)  ? " ofpel"  : "");

	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(ldterm.t_modes.c_oflag&NLDLY)  == NL0  ? " nl0"  : "",
		(ldterm.t_modes.c_oflag&NLDLY)  == NL1  ? " nl1"  : "",
		(ldterm.t_modes.c_oflag&CRDLY)  == CR0  ? " cr0"  : "",
		(ldterm.t_modes.c_oflag&CRDLY)  == CR1  ? " cr1"  : "",
		(ldterm.t_modes.c_oflag&CRDLY)  == CR2  ? " cr2"  : "",
		(ldterm.t_modes.c_oflag&CRDLY)  == CR3  ? " cr3"  : "",
		(ldterm.t_modes.c_oflag&TABDLY) == TAB0 ? " tab0" : "",
		(ldterm.t_modes.c_oflag&TABDLY) == TAB1 ? " tab1" : "",
		(ldterm.t_modes.c_oflag&TABDLY) == TAB2 ? " tab2" : "",
		(ldterm.t_modes.c_oflag&TABDLY) == TAB3 ? " tab3" : "",
		(ldterm.t_modes.c_oflag&BSDLY)  == BS0  ? " bs0"  : "",
		(ldterm.t_modes.c_oflag&BSDLY)  == BS1  ? " bs1"  : "",
		(ldterm.t_modes.c_oflag&VTDLY)  == VT0  ? " vt0"  : "",
		(ldterm.t_modes.c_oflag&VTDLY)  == VT1  ? " vt1"  : "",
		(ldterm.t_modes.c_oflag&FFDLY)  == FF0  ? " ff0"  : "",
		(ldterm.t_modes.c_oflag&FFDLY)  == FF1  ? " ff1"  : "");

	fprintf(fp,"\tlflag: %s%s%s%s%s%s%s%s\n",
		ldterm.t_modes.c_lflag & ISIG   ? " isig"   : "",
		ldterm.t_modes.c_lflag & ICANON ? " icanon" : "",
		ldterm.t_modes.c_lflag & XCASE  ? " xcase"  : "",
		ldterm.t_modes.c_lflag & ECHO   ? " echo"   : "",
		ldterm.t_modes.c_lflag & ECHOE  ? " echoe"  : "",
		ldterm.t_modes.c_lflag & ECHOK  ? " echok"  : "",
		ldterm.t_modes.c_lflag & ECHONL ? " echonl" : "",
		ldterm.t_modes.c_lflag & NOFLSH ? " noflsh" : "");

	fprintf( fp, "\n\tApparent termios\n"); 

	fprintf(fp,"\tintr: ");
	putch((char)ldterm.t_amodes.c_cc[VINTR]);
	fprintf(fp,"\tquit: ");
	putch((char)ldterm.t_amodes.c_cc[VQUIT]);
	fprintf(fp,"\terase: ");
	putch((char)ldterm.t_amodes.c_cc[VERASE]);
	fprintf(fp,"\tkill: ");
	putch((char)ldterm.t_amodes.c_cc[VKILL]);
	fprintf(fp,"\n");
	fprintf(fp,"\teof/vmin: ");
	putch((char)ldterm.t_amodes.c_cc[VEOF]);
	fprintf(fp,"\teol/vtime: ");
	putch((char)ldterm.t_amodes.c_cc[VEOL]);
	fprintf(fp,"\teol2: ");
	putch((char)ldterm.t_amodes.c_cc[VEOL2]);
	fprintf(fp,"\tswtch: ");
	putch((char)ldterm.t_amodes.c_cc[VSWTCH]);
	fprintf(fp,"\n");
	fprintf(fp,"\tstart: ");
	putch((char)ldterm.t_amodes.c_cc[VSTART]);
	fprintf(fp,"\tstop: ");
	putch((char)ldterm.t_amodes.c_cc[VSTOP]);
	fprintf(fp,"\tsusp: ");
	putch((char)ldterm.t_amodes.c_cc[VSUSP]);
	fprintf(fp,"\tdsusp: ");
	putch((char)ldterm.t_amodes.c_cc[VDSUSP]);
	fprintf(fp,"\n");
	fprintf(fp,"\treprint: ");
	putch((char)ldterm.t_amodes.c_cc[VREPRINT]);
	fprintf(fp,"\tdiscard: ");
	putch((char)ldterm.t_amodes.c_cc[VDISCARD]);
	fprintf(fp,"\twerase: ");
	putch((char)ldterm.t_amodes.c_cc[VWERASE]);
	fprintf(fp,"\tlnext: ");
	putch((char)ldterm.t_amodes.c_cc[VLNEXT]);
	fprintf(fp,"\n");

	fprintf(fp,"\tiflags: %s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(ldterm.t_amodes.c_iflag&IGNBRK) ? " ignbrk" : "",
		(ldterm.t_amodes.c_iflag&BRKINT) ? " brkint" : "",
		(ldterm.t_amodes.c_iflag&IGNPAR) ? " ignpar" : "",
		(ldterm.t_amodes.c_iflag&PARMRK) ? " parmrk" : "",
		(ldterm.t_amodes.c_iflag&INPCK)  ? " inpck"  : "",
		(ldterm.t_amodes.c_iflag&ISTRIP) ? " istrip" : "",
		(ldterm.t_amodes.c_iflag&INLCR)  ? " inlcr"  : "",
		(ldterm.t_amodes.c_iflag&IGNCR)  ? " icncr"  : "",
		(ldterm.t_amodes.c_iflag&ICRNL)  ? " icrnl"  : "",
		(ldterm.t_amodes.c_iflag&IUCLC)  ? " iuclc"  : "",
		(ldterm.t_amodes.c_iflag&IXON)   ? " ixon"   : "",
		(ldterm.t_amodes.c_iflag&IXANY)  ? " ixany"  : "",
		(ldterm.t_amodes.c_iflag&IXOFF)  ? " ixoff"  : "");

	fprintf(fp,"\toflags: %s%s%s%s%s%s%s%s",
		(ldterm.t_amodes.c_oflag&OPOST)  ? " opost"  : "",
		(ldterm.t_amodes.c_oflag&OLCUC)  ? " olcuc"  : "",
		(ldterm.t_amodes.c_oflag&ONLCR)  ? " onlcr"  : "",
		(ldterm.t_amodes.c_oflag&OCRNL)  ? " ocrnl"  : "",
		(ldterm.t_amodes.c_oflag&ONOCR)  ? " onocr"  : "",
		(ldterm.t_amodes.c_oflag&ONLRET) ? " onlret" : "",
		(ldterm.t_amodes.c_oflag&OFILL)  ? " ofill"  : "",
		(ldterm.t_amodes.c_oflag&OFDEL)  ? " ofpel"  : "");

	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(ldterm.t_amodes.c_oflag&NLDLY)  == NL0  ? " nl0"  : "",
		(ldterm.t_amodes.c_oflag&NLDLY)  == NL1  ? " nl1"  : "",
		(ldterm.t_amodes.c_oflag&CRDLY)  == CR0  ? " cr0"  : "",
		(ldterm.t_amodes.c_oflag&CRDLY)  == CR1  ? " cr1"  : "",
		(ldterm.t_amodes.c_oflag&CRDLY)  == CR2  ? " cr2"  : "",
		(ldterm.t_amodes.c_oflag&CRDLY)  == CR3  ? " cr3"  : "",
		(ldterm.t_amodes.c_oflag&TABDLY) == TAB0 ? " tab0" : "",
		(ldterm.t_amodes.c_oflag&TABDLY) == TAB1 ? " tab1" : "",
		(ldterm.t_amodes.c_oflag&TABDLY) == TAB2 ? " tab2" : "",
		(ldterm.t_amodes.c_oflag&TABDLY) == TAB3 ? " tab3" : "",
		(ldterm.t_amodes.c_oflag&BSDLY)  == BS0  ? " bs0"  : "",
		(ldterm.t_amodes.c_oflag&BSDLY)  == BS1  ? " bs1"  : "",
		(ldterm.t_amodes.c_oflag&VTDLY)  == VT0  ? " vt0"  : "",
		(ldterm.t_amodes.c_oflag&VTDLY)  == VT1  ? " vt1"  : "",
		(ldterm.t_amodes.c_oflag&FFDLY)  == FF0  ? " ff0"  : "",
		(ldterm.t_amodes.c_oflag&FFDLY)  == FF1  ? " ff1"  : "");

	fprintf(fp,"\tlflag: %s%s%s%s%s%s%s%s\n",
		ldterm.t_amodes.c_lflag & ISIG   ? " isig"   : "",
		ldterm.t_amodes.c_lflag & ICANON ? " icanon" : "",
		ldterm.t_amodes.c_lflag & XCASE  ? " xcase"  : "",
		ldterm.t_amodes.c_lflag & ECHO   ? " echo"   : "",
		ldterm.t_amodes.c_lflag & ECHOE  ? " echoe"  : "",
		ldterm.t_amodes.c_lflag & ECHOK  ? " echok"  : "",
		ldterm.t_amodes.c_lflag & ECHONL ? " echonl" : "",
		ldterm.t_amodes.c_lflag & NOFLSH ? " noflsh" : "");

	fprintf( fp, "\n\tDriver's termios\n"); 

	fprintf(fp,"\tintr: ");
	putch((char)ldterm.t_dmodes.c_cc[VINTR]);
	fprintf(fp,"\tquit: ");
	putch((char)ldterm.t_dmodes.c_cc[VQUIT]);
	fprintf(fp,"\terase: ");
	putch((char)ldterm.t_dmodes.c_cc[VERASE]);
	fprintf(fp,"\tkill: ");
	putch((char)ldterm.t_dmodes.c_cc[VKILL]);
	fprintf(fp,"\n");
	fprintf(fp,"\teof/vmin: ");
	putch((char)ldterm.t_dmodes.c_cc[VEOF]);
	fprintf(fp,"\teol/vtime: ");
	putch((char)ldterm.t_dmodes.c_cc[VEOL]);
	fprintf(fp,"\teol2: ");
	putch((char)ldterm.t_dmodes.c_cc[VEOL2]);
	fprintf(fp,"\tswtch: ");
	putch((char)ldterm.t_dmodes.c_cc[VSWTCH]);
	fprintf(fp,"\n");
	fprintf(fp,"\tstart: ");
	putch((char)ldterm.t_dmodes.c_cc[VSTART]);
	fprintf(fp,"\tstop: ");
	putch((char)ldterm.t_dmodes.c_cc[VSTOP]);
	fprintf(fp,"\tsusp: ");
	putch((char)ldterm.t_dmodes.c_cc[VSUSP]);
	fprintf(fp,"\tdsusp: ");
	putch((char)ldterm.t_dmodes.c_cc[VDSUSP]);
	fprintf(fp,"\n");
	fprintf(fp,"\treprint: ");
	putch((char)ldterm.t_dmodes.c_cc[VREPRINT]);
	fprintf(fp,"\tdiscard: ");
	putch((char)ldterm.t_dmodes.c_cc[VDISCARD]);
	fprintf(fp,"\twerase: ");
	putch((char)ldterm.t_dmodes.c_cc[VWERASE]);
	fprintf(fp,"\tlnext: ");
	putch((char)ldterm.t_dmodes.c_cc[VLNEXT]);
	fprintf(fp,"\n");

	fprintf(fp,"\tiflags: %s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(ldterm.t_dmodes.c_iflag&IGNBRK) ? " ignbrk" : "",
		(ldterm.t_dmodes.c_iflag&BRKINT) ? " brkint" : "",
		(ldterm.t_dmodes.c_iflag&IGNPAR) ? " ignpar" : "",
		(ldterm.t_dmodes.c_iflag&PARMRK) ? " parmrk" : "",
		(ldterm.t_dmodes.c_iflag&INPCK)  ? " inpck"  : "",
		(ldterm.t_dmodes.c_iflag&ISTRIP) ? " istrip" : "",
		(ldterm.t_dmodes.c_iflag&INLCR)  ? " inlcr"  : "",
		(ldterm.t_dmodes.c_iflag&IGNCR)  ? " icncr"  : "",
		(ldterm.t_dmodes.c_iflag&ICRNL)  ? " icrnl"  : "",
		(ldterm.t_dmodes.c_iflag&IUCLC)  ? " iuclc"  : "",
		(ldterm.t_dmodes.c_iflag&IXON)   ? " ixon"   : "",
		(ldterm.t_dmodes.c_iflag&IXANY)  ? " ixany"  : "",
		(ldterm.t_dmodes.c_iflag&IXOFF)  ? " ixoff"  : "");

	fprintf(fp,"\toflags: %s%s%s%s%s%s%s%s",
		(ldterm.t_dmodes.c_oflag&OPOST)  ? " opost"  : "",
		(ldterm.t_dmodes.c_oflag&OLCUC)  ? " olcuc"  : "",
		(ldterm.t_dmodes.c_oflag&ONLCR)  ? " onlcr"  : "",
		(ldterm.t_dmodes.c_oflag&OCRNL)  ? " ocrnl"  : "",
		(ldterm.t_dmodes.c_oflag&ONOCR)  ? " onocr"  : "",
		(ldterm.t_dmodes.c_oflag&ONLRET) ? " onlret" : "",
		(ldterm.t_dmodes.c_oflag&OFILL)  ? " ofill"  : "",
		(ldterm.t_dmodes.c_oflag&OFDEL)  ? " ofpel"  : "");

	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(ldterm.t_dmodes.c_oflag&NLDLY)  == NL0  ? " nl0"  : "",
		(ldterm.t_dmodes.c_oflag&NLDLY)  == NL1  ? " nl1"  : "",
		(ldterm.t_dmodes.c_oflag&CRDLY)  == CR0  ? " cr0"  : "",
		(ldterm.t_dmodes.c_oflag&CRDLY)  == CR1  ? " cr1"  : "",
		(ldterm.t_dmodes.c_oflag&CRDLY)  == CR2  ? " cr2"  : "",
		(ldterm.t_dmodes.c_oflag&CRDLY)  == CR3  ? " cr3"  : "",
		(ldterm.t_dmodes.c_oflag&TABDLY) == TAB0 ? " tab0" : "",
		(ldterm.t_dmodes.c_oflag&TABDLY) == TAB1 ? " tab1" : "",
		(ldterm.t_dmodes.c_oflag&TABDLY) == TAB2 ? " tab2" : "",
		(ldterm.t_dmodes.c_oflag&TABDLY) == TAB3 ? " tab3" : "",
		(ldterm.t_dmodes.c_oflag&BSDLY)  == BS0  ? " bs0"  : "",
		(ldterm.t_dmodes.c_oflag&BSDLY)  == BS1  ? " bs1"  : "",
		(ldterm.t_dmodes.c_oflag&VTDLY)  == VT0  ? " vt0"  : "",
		(ldterm.t_dmodes.c_oflag&VTDLY)  == VT1  ? " vt1"  : "",
		(ldterm.t_dmodes.c_oflag&FFDLY)  == FF0  ? " ff0"  : "",
		(ldterm.t_dmodes.c_oflag&FFDLY)  == FF1  ? " ff1"  : "");

	fprintf(fp,"\tlflag: %s%s%s%s%s%s%s%s\n",
		ldterm.t_dmodes.c_lflag & ISIG   ? " isig"   : "",
		ldterm.t_dmodes.c_lflag & ICANON ? " icanon" : "",
		ldterm.t_dmodes.c_lflag & XCASE  ? " xcase"  : "",
		ldterm.t_dmodes.c_lflag & ECHO   ? " echo"   : "",
		ldterm.t_dmodes.c_lflag & ECHOE  ? " echoe"  : "",
		ldterm.t_dmodes.c_lflag & ECHOK  ? " echok"  : "",
		ldterm.t_dmodes.c_lflag & ECHONL ? " echonl" : "",
		ldterm.t_dmodes.c_lflag & NOFLSH ? " noflsh" : "");

	fprintf(fp,"\n");
	/*
	 * Fields related to multi-byte character handling
	 */
	fprintf( fp, "\tLine Discipline support for Multi-byte\n");
	fprintf( fp, "\tCode set indicator %o\tbytes left for current character %u\n", ldterm.t_codeset, ldterm.t_eucleft);
	fprintf( fp, "\tBytes left to ignore %u\tpadding %u\n", ldterm.t_eucign, ldterm.t_eucpad);
	fprintf( fp, "\tPointer to array cols. %x\tPointer to m_blk for col. array %x\n", ldterm.t_eucp, ldterm.t_eucp_mp);
	fprintf( fp, "\tMax. length count of an EUC. %u\tbad EUC counter %d\n", ldterm.t_maxeuc, ldterm.t_eucwarn);
	fprintf( fp, "\tstruct eucioc entries\n");
	fprintf( fp, "\teucw[0] %u\teucw[1] %u\teucw[2] %u\teucw[3] %u\n", ldterm.eucwioc.eucw[0], ldterm.eucwioc.eucw[1], ldterm.eucwioc.eucw[2], ldterm.eucwioc.eucw[3]);
	fprintf( fp, "\tscrw[0] %u\tscrw[1] %u\tscrw[2] %u\tscrw[3] %u\n", ldterm.eucwioc.scrw[0], ldterm.eucwioc.scrw[1], ldterm.eucwioc.scrw[2], ldterm.eucwioc.scrw[3]);

	return( 0);
}
