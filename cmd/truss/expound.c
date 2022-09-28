/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:expound.c	1.10.3.1"

#include <stdio.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"

#include <string.h>
#include <sys/param.h>
#include <sys/statfs.h>
#include <sys/times.h>
#include <sys/utssys.h>
#include <sys/utsname.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/dirent.h>
#include <ustat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/termios.h>
#include <sys/termiox.h>
#include <sys/termio.h>
#include <sys/ttold.h>
#include <sys/jioctl.h>
#include <sys/filio.h>
#include <stropts.h>
#include <poll.h>
#include <sys/uio.h>
#include <sys/resource.h>
#include <sys/statvfs.h>
#include <sys/time.h>

#include "ramdata.h"
#include "systable.h"
#include "proto.h"

/*
 * Function prototypes for static routines in this module.
 */
#if	defined(__STDC__)

static	void	show_utime( process_t * );
static	void	show_stime();
static	void	show_times( process_t * );
static	void	show_utssys( process_t * , int );
static	void	show_uname( process_t * , long );
static	void	show_nuname( process_t * , long );
static	void	show_ustat( process_t * , long );
static	void	show_fusers( process_t * , long , int );
static	void	show_ioctl( process_t * , int , long );
static	void	show_termio( process_t * , long );
static	void	show_termios( process_t * , long );
static	void	show_termiox( process_t * , long );
static	void	show_sgttyb( process_t * , long );
static	char *	show_char( char * , int );
static	void	show_termcb( process_t * , long );
static	void	show_strint( process_t * , int , long );
static	void	show_strioctl( process_t * , long );
static	void	show_strpeek( process_t * , long );
static	void	show_strfdinsert( process_t * , long );
static	CONST char *	strflags( long );
static	void	show_strrecvfd( process_t * , long );
#ifdef	I_LIST
static	void	show_strlist( process_t * , long );
#endif
static	void	show_jwinsize( process_t * , long );
static	void	show_winsize( process_t * , long );
static	void	show_statvfs( process_t * );
static	void	show_statfs( process_t * );
static	void	show_fcntl( process_t * );
static	void	show_flock( process_t * , long );
static	void	show_blocks( process_t * , CONST char * , long );
static	void	show_gp_msg( process_t * );
static	void	show_strbuf( process_t * , long , CONST char * );
static	void	print_strbuf( struct strbuf * , CONST char * );
static	void	show_poll( process_t * );
static	void	show_pollfd( process_t * , long );
static	CONST char *	pollevent( int );
static	void	show_perm( struct ipc_perm * );
static	void	show_msgsys( process_t * , int );
static	void	show_msgctl( process_t * , long );
static	void	show_msgbuf( process_t * , long , int );
static	void	show_semsys( process_t * );
static	void	show_semctl( process_t * , long );
static	void	show_semop( process_t * , long , int );
static	void	show_shmsys( process_t * );
static	void	show_shmctl( process_t * , long );
static	void	show_groups( process_t * , long , int );
static	void	show_sigset( process_t * , long , CONST char * );
static	char *	sigset_string( sigset_t * );
static	void	show_sigaltstack( process_t * , long , CONST char * );
static	void	show_sigaction( process_t * , long , CONST char * , int );
static	void	show_bool( process_t * , long , int );
static	void	show_iovec( process_t * , long , int , int , long );
static	void	show_dents( process_t * , long , int );
static	void	show_rlimit( process_t * , long );
static	void	show_adjtime( process_t * , long, long );

#else	/* defined(__STDC__) */

static	void	show_utime();
static	void	show_stime();
static	void	show_times();
static	void	show_utssys();
static	void	show_uname();
static	void	show_nuname();
static	void	show_ustat();
static	void	show_fusers();
static	void	show_ioctl();
static	void	show_termio();
static	void	show_termios();
static	void	show_termiox();
static	void	show_sgttyb();
static	char *	show_char();
static	void	show_termcb();
static	void	show_strint();
static	void	show_strioctl();
static	void	show_strpeek();
static	void	show_strfdinsert();
static	CONST char *	strflags();
static	void	show_strrecvfd();
#ifdef	I_LIST
static	void	show_strlist();
#endif
static	void	show_jwinsize();
static	void	show_winsize();
static	void	show_statvfs();
static	void	show_statfs();
static	void	show_fcntl();
static	void	show_flock();
static	void	show_blocks();
static	void	show_gp_msg();
static	void	show_strbuf();
static	void	print_strbuf();
static	void	show_poll();
static	void	show_pollfd();
static	CONST char *	pollevent();
static	void	show_perm();
static	void	show_msgsys();
static	void	show_msgctl();
static	void	show_msgbuf();
static	void	show_semsys();
static	void	show_semctl();
static	void	show_semop();
static	void	show_shmsys();
static	void	show_shmctl();
static	void	show_groups();
static	void	show_sigset();
static	char *	sigset_string();
static	void	show_sigaltstack();
static	void	show_sigaction();
static	void	show_bool();
static	void	show_iovec();
static	void	show_dents();
static	void	show_rlimit();
static	void	show_adjtime();

#endif	/* defined(__STDC__) */


/*ARGSUSED*/
void
expound(Pr, r0, raw)	/* expound verbosely upon syscall arguments */
process_t *Pr;
int r0;
int raw;
{
	register int what = Pr->why.pr_what;
	register int err = Errno;	/* don't display output parameters */
					/* for a failed system call */
	switch (what) {
	case SYS_utime:
		show_utime(Pr);
		break;
	case SYS_stime:
		show_stime();
		break;
	case SYS_times:
		if (!err)
			show_times(Pr);
		break;
	case SYS_utssys:
		if (!err)
			show_utssys(Pr, r0);
		break;
	case SYS_ioctl:
		if (sys_nargs >= 3)	/* each case must decide for itself */
			show_ioctl(Pr, sys_args[1], (long)sys_args[2]);
		break;
	case SYS_stat:
	case SYS_fstat:
	case SYS_lstat:
		if (!err && sys_nargs >= 2)
			show_stat(Pr, (long)sys_args[1]);
		break;
	case SYS_xstat:
	case SYS_fxstat:
	case SYS_lxstat:
		if (!err && sys_nargs >= 3) {
			switch (sys_args[0]) {
			case 1:
				show_stat(Pr, (long)sys_args[2]);
				break;
			case 2:
				show_xstat(Pr, (long)sys_args[2]);
				break;
			}
		}
		break;
	case SYS_statvfs:
	case SYS_fstatvfs:
		if (!err)
			show_statvfs(Pr);
		break;
	case SYS_statfs:
	case SYS_fstatfs:
		if (!err)
			show_statfs(Pr);
		break;
	case SYS_fcntl:
		show_fcntl(Pr);
		break;
	case SYS_msgsys:
		show_msgsys(Pr, r0);	/* each case must decide for itself */
		break;
	case SYS_semsys:
		show_semsys(Pr);	/* each case must decide for itself */
		break;
	case SYS_shmsys:
		show_shmsys(Pr);	/* each case must decide for itself */
		break;
	case SYS_getdents:
		if (!err && sys_nargs > 1 && r0 > 0)
			show_dents(Pr, (long)sys_args[1], r0);
		break;
	case SYS_getmsg:
	case SYS_putmsg:
	case SYS_getpmsg:
	case SYS_putpmsg:
		show_gp_msg(Pr);
		break;
	case SYS_poll:
		show_poll(Pr);
		break;
	case SYS_setgroups:
		if (sys_nargs > 1 && (r0 = sys_args[0]) > 0)
			show_groups(Pr, (long)sys_args[1], r0);
		break;
	case SYS_getgroups:
		if (!err && sys_nargs > 1 && sys_args[0] > 0)
			show_groups(Pr, (long)sys_args[1], r0);
		break;
	case SYS_sigprocmask:
		if (sys_nargs > 1)
			show_sigset(Pr, (long)sys_args[1], " set");
		if (!err && sys_nargs > 2)
			show_sigset(Pr, (long)sys_args[2], "oset");
		break;
	case SYS_sigsuspend:
		if (sys_nargs > 0)
			show_sigset(Pr, (long)sys_args[0], "setmask");
		break;
	case SYS_sigaltstack:
		if (sys_nargs > 0)
			show_sigaltstack(Pr, (long)sys_args[0], "new");
		if (!err && sys_nargs > 1)
			show_sigaltstack(Pr, (long)sys_args[1], "old");
		break;
	case SYS_sigaction:
		if (sys_nargs > 1)
			show_sigaction(Pr, (long)sys_args[1], "new", NULL);
		if (!err && sys_nargs > 2)
			show_sigaction(Pr, (long)sys_args[2], "old", r0);
		break;
	case SYS_sigpending:
		if (!err && sys_nargs > 1)
			show_sigset(Pr, (long)sys_args[1], "sigmask");
		break;
	case SYS_waitsys:
		if (sys_nargs > 0)
			show_procset(Pr, (long)sys_args[0]);
		if (!err && sys_nargs > 2)
			show_statloc(Pr, (long)sys_args[2]);
		break;
	case SYS_sigsendsys:
		if (sys_nargs > 0)
			show_procset(Pr, (long)sys_args[0]);
		break;
	case SYS_priocntlsys:
		if (sys_nargs > 1)
			show_procset(Pr, (long)sys_args[1]);
		break;
	case SYS_mincore:
		if (!err && sys_nargs > 2)
			show_bool(Pr, (long)sys_args[2],
				(sys_args[1]+NBPC-1)/NBPC);
		break;
	case SYS_readv:
	case SYS_writev:
		if (sys_nargs > 2) {
			register int i = sys_args[0]+1;
			register int showbuf = FALSE;
			long nb = (what==SYS_readv)? r0 : 32*1024;

			if ((what==SYS_readv && !err && prismember(&readfd,i))
			  || (what==SYS_writev && prismember(&writefd,i)))
				showbuf = TRUE;
			show_iovec(Pr, (long)sys_args[1], sys_args[2],
				showbuf, nb);
		}
		break;
	case SYS_getrlimit:
		if (err)
			break;
		/*FALLTHROUGH*/
	case SYS_setrlimit:
		if (sys_nargs > 1)
			show_rlimit(Pr, (long)sys_args[1]);
		break;
	case SYS_uname:
		if (!err && sys_nargs > 0)
			show_nuname(Pr, (long)sys_args[0]);
		break;
	case SYS_adjtime:
		if (!err && sys_nargs > 1)
			show_adjtime(Pr, (long)sys_args[0], (long)sys_args[1]);
		break;
	}
}

static void
show_utime(Pr)
register process_t *Pr;
{
	register long offset;
	struct {
		time_t	atime;
		time_t	mtime;
	} utimbuf;

	if (sys_nargs >= 2 && (offset = sys_args[1]) != NULL
	 && Pread(Pr, offset, (char *)&utimbuf, sizeof(utimbuf)) ==
	    sizeof(utimbuf)) {
		/* print access and modification times */
		prtime("atime: ", utimbuf.atime);
		prtime("mtime: ", utimbuf.mtime);
	}
}

static void
show_stime()
{
	if (sys_nargs >= 1) {
		/* print new system time */
		prtime("systime = ", (time_t)sys_args[0]);
	}
}

static void
show_times(Pr)
register process_t *Pr;
{
	register long offset;
	struct tms tms;

	if (sys_nargs >= 1 && (offset = sys_args[0]) != NULL
	 && Pread(Pr, offset, (char *)&tms, sizeof(tms)) ==
	    sizeof(tms)) {
		(void) printf(
		"%s\tutim=%-6u stim=%-6u cutim=%-6u cstim=%-6u (HZ=%d)\n",
			pname,
			tms.tms_utime,
			tms.tms_stime,
			tms.tms_cutime,
			tms.tms_cstime,
			HZ);
	}
}

static void
show_utssys(Pr, r0)
register process_t *Pr;
int r0;
{
	if (sys_nargs >= 3) {
		switch (sys_args[2]) {
		case UTS_UNAME:
			show_uname(Pr, (long)sys_args[0]);
			break;
		case UTS_USTAT:
			show_ustat(Pr, (long)sys_args[0]);
			break;
		case UTS_FUSERS:
			show_fusers(Pr, (long)sys_args[3], r0);
			break;
		}
	}
}

static void
show_uname(Pr, offset)
process_t *Pr;
long offset;
{
	/*
	 * Old utsname buffer (no longer accessible in <sys/utsname.h>).
	 */
	struct {
		char	sysname[9];
		char	nodename[9];
		char	release[9];
		char	version[9];
		char	machine[9];
	} ubuf;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&ubuf, sizeof(ubuf)) ==
	    sizeof(ubuf)) {
		(void) printf(
		"%s\tsys=%-9.9snod=%-9.9srel=%-9.9sver=%-9.9smch=%.9s\n",
			pname,
			ubuf.sysname,
			ubuf.nodename,
			ubuf.release,
			ubuf.version,
			ubuf.machine);
	}
}

static void
show_ustat(Pr, offset)
register process_t *Pr;
long offset;
{
	struct ustat ubuf;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&ubuf, sizeof(ubuf)) ==
	    sizeof(ubuf)) {
		(void) printf(
		"%s\ttfree=%-6ld tinode=%-5d fname=%-6.6s fpack=%-.6s\n",
			pname,
			ubuf.f_tfree,
			ubuf.f_tinode,
			ubuf.f_fname,
			ubuf.f_fpack);
	}
}

static void
show_fusers(Pr, offset, nproc)
process_t *Pr;
long offset;
int nproc;
{
	f_user_t fubuf;
	int serial = (nproc > 4);

	if (offset == NULL)
		return;

	/* enter region of lengthy output */
	if (serial)
		Eserialize();

	while (nproc > 0 && Pread(Pr, offset, (char *)&fubuf, sizeof(fubuf)) ==
	    sizeof(fubuf)) {
		(void) printf("%s\tpid=%-6duid=%-6dflags=%s\n",
		  pname, fubuf.fu_pid, fubuf.fu_uid, fuflags(fubuf.fu_flags));
		nproc--;
		offset += sizeof(fubuf);
	}

	/* exit region of lengthy output */
	if (serial)
		Xserialize();
}

static void
show_ioctl(Pr, code, offset)	/* print values in ioctl() argument */
process_t *Pr;
int code;
long offset;
{
	register int err = Errno;	/* don't display output parameters */
					/* for a failed system call */
	if (offset == NULL)
		return;

	switch (code) {
	case TCGETA:
		if (err)
			break;
		/*FALLTHROUGH*/
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		show_termio(Pr, offset);
		break;
	case TCGETS:
		if (err)
			break;
		/*FALLTHROUGH*/
	case TCSETS:
	case TCSETSW:
	case TCSETSF:
		show_termios(Pr, offset);
		break;
	case TCGETX:
		if (err)
			break;
		/*FALLTHROUGH*/
	case TCSETX:
	case TCSETXW:
	case TCSETXF:
		show_termiox(Pr, offset);
		break;
	case TIOCGETP:
		if (err)
			break;
		/*FALLTHROUGH*/
	case TIOCSETP:
		show_sgttyb(Pr, offset);
		break;
	case LDGETT:
		if (err)
			break;
		/*FALLTHROUGH*/
	case LDSETT:
		show_termcb(Pr, offset);
		break;
	/* streams ioctl()s */
#if 0
		/* these are displayed as strings in the arg list */
		/* by prt_ioa().  don't display them again here */
	case I_PUSH:
	case I_LOOK:
	case I_FIND:
		/* these are displayed as decimal in the arg list */
		/* by prt_ioa().  don't display them again here */
	case I_LINK:
	case I_UNLINK:
	case I_SENDFD:
		/* these are displayed symbolically in the arg list */
		/* by prt_ioa().  don't display them again here */
	case I_SRDOPT:
	case I_SETSIG:
	case I_FLUSH:
		break;
		/* this one just ignores the argument */
	case I_POP:
		break;
#endif
		/* these return something in an int pointed to by arg */
	case I_NREAD:
	case I_GRDOPT:
	case I_GETSIG:
	case TIOCGSID:
	case TIOCGPGRP:
	case FIONREAD:
		if (err)
			break;
		/*FALLTHROUGH*/
	case TIOCSPGRP:
		show_strint(Pr, code, offset);
		break;
		/* these all point to structures */
	case I_STR:
		show_strioctl(Pr, offset);
		break;
	case I_PEEK:
		show_strpeek(Pr, offset);
		break;
	case I_FDINSERT:
		show_strfdinsert(Pr, offset);
		break;
	case I_RECVFD:
		if (!err)
			show_strrecvfd(Pr, offset);
		break;
#ifdef I_LIST			/* new in SVR4.0 */
	case I_LIST:
		if (!err)
			show_strlist(Pr, offset);
		break;
#endif /* I_LIST */
	case JWINSIZE:
		if (!err)
			show_jwinsize(Pr, offset);
		break;
	case TIOCGWINSZ:
		if (err)
			break;
		/*FALLTHROUGH*/
	case TIOCSWINSZ:
		show_winsize(Pr, offset);
		break;
	}
}

static void
show_termio(Pr, offset)
process_t *Pr;
long offset;
{
	struct termio termio;
	char cbuf[8];
	register int i;

	if (Pread(Pr, offset, (char *)&termio, sizeof(termio)) ==
	    sizeof(termio)) {
		(void) printf(
		"%s\tiflag=0%.6o oflag=0%.6o cflag=0%.6o lflag=0%.6o line=%d\n",
			pname,
			termio.c_iflag,
			termio.c_oflag,
			termio.c_cflag,
			termio.c_lflag,
			termio.c_line);
		(void) printf("%s\t    cc: ", pname);
		for (i = 0; i < NCC; i++)
			(void) printf(" %s",
				show_char(cbuf, (int)termio.c_cc[i]));
		(void) fputc('\n', stdout);
	}
}

static void
show_termios(Pr, offset)
process_t *Pr;
long offset;
{
	struct termios termios;
	char cbuf[8];
	register int i;

	if (Pread(Pr, offset, (char *)&termios, sizeof(termios)) ==
	    sizeof(termios)) {
		(void) printf(
		"%s\tiflag=0%.6lo oflag=0%.6lo cflag=0%.6lo lflag=0%.6lo\n",
			pname,
			termios.c_iflag,
			termios.c_oflag,
			termios.c_cflag,
			termios.c_lflag);
		(void) printf("%s\t    cc: ", pname);
		for (i = 0; i < NCCS; i++) {
			if (i == NCC)	/* show new chars on new line */
				(void) printf("\n%s\t\t", pname);
			(void) printf(" %s",
				show_char(cbuf, (int)termios.c_cc[i]));
		}
		(void) fputc('\n', stdout);
	}
}

static void
show_termiox(Pr, offset)
process_t *Pr;
long offset;
{
	struct termiox termiox;
	register int i;

	if (Pread(Pr, offset, (char *)&termiox, sizeof(termiox)) ==
	    sizeof(termiox)) {
		(void) printf("%s\thflag=0%.3o cflag=0%.3o rflag=0%.3o",
			pname,
			termiox.x_hflag,
			termiox.x_cflag,
			termiox.x_rflag[0]);
		for (i = 1; i < NFF; i++)
			(void) printf(",0%.3o", termiox.x_rflag[i]);
		(void) printf(" sflag=0%.3o\n",
			termiox.x_sflag);
	}
}

static void
show_sgttyb(Pr, offset)
process_t *Pr;
long offset;
{
	struct sgttyb sgttyb;

	if (Pread(Pr, offset, (char *)&sgttyb, sizeof(sgttyb)) ==
	    sizeof(sgttyb)) {
		char erase[8];
		char kill[8];

		(void) printf(
		"%s\tispeed=%-2d ospeed=%-2d erase=%s kill=%s flags=0%.6o\n",
			pname,
			sgttyb.sg_ispeed&0xff,
			sgttyb.sg_ospeed&0xff,
			show_char(erase, sgttyb.sg_erase),
			show_char(kill, sgttyb.sg_kill),
			sgttyb.sg_flags);
	}
}

static char *
show_char(buf, c)	/* represent character as itself ('c') or octal (012) */
char * buf;
register int c;
{
	register CONST char * fmt;

	if (c >= ' ' && c < 0177)
		fmt = "'%c'";
	else
		fmt = "%.3o";

	(void) sprintf(buf, fmt, c&0xff);
	return buf;
}

static void
show_termcb(Pr, offset)
process_t *Pr;
long offset;
{
	struct termcb termcb;

	if (Pread(Pr, offset, (char *)&termcb, sizeof(termcb)) ==
	    sizeof(termcb)) {
		(void) printf(
		"%s\tflgs=0%.2o termt=%d crow=%d ccol=%d vrow=%d lrow=%d\n",
			pname,
			termcb.st_flgs&0xff,
			termcb.st_termt&0xff,
			termcb.st_crow&0xff,
			termcb.st_ccol&0xff,
			termcb.st_vrow&0xff,
			termcb.st_lrow&0xff);
	}
}

static void
show_strint(Pr, code, offset)	/* integer value pointed to by ioctl() arg */
process_t *Pr;
int code;
long offset;
{
	int val;

	if (Pread(Pr, offset, (char *)&val, sizeof(val)) == sizeof(val)) {
		CONST char * s = NULL;

		switch (code) {		/* interpret these symbolically */
		case I_GRDOPT:
			s = strrdopt(val);
			break;
		case I_GETSIG:
			s = strevents(val);
			break;
		}

		if (s == NULL)
			(void) printf("%s\t0x%.8X: %d\n", pname, offset, val);
		else
			(void) printf("%s\t0x%.8X: %s\n", pname, offset, s);
	}
}

static void
show_strioctl(Pr, offset)
process_t *Pr;
long offset;
{
	struct strioctl strioctl;

	if (Pread(Pr, offset, (char *)&strioctl, sizeof(strioctl)) ==
	    sizeof(strioctl)) {
		(void) printf(
			"%s\tcmd=%s timout=%d len=%d dp=0x%.8X\n",
			pname,
			ioctlname(strioctl.ic_cmd),
			strioctl.ic_timout,
			strioctl.ic_len,
			strioctl.ic_dp);

		if (!recur++)	/* avoid indefinite recursion */
			show_ioctl(Pr, strioctl.ic_cmd, (long)strioctl.ic_dp);
		--recur;
	}
}

static void
show_strpeek(Pr, offset)
process_t *Pr;
long offset;
{
	struct strpeek strpeek;

	if (Pread(Pr, offset, (char *)&strpeek, sizeof(strpeek)) ==
	    sizeof(strpeek)) {

		print_strbuf(&strpeek.ctlbuf, "ctl");
		print_strbuf(&strpeek.databuf, "dat");

		(void) printf("%s\tflags=%s\n",
			pname,
			strflags(strpeek.flags));
	}
}

static void
show_strfdinsert(Pr, offset)
process_t *Pr;
long offset;
{
	struct strfdinsert strfdinsert;

	if (Pread(Pr, offset, (char *)&strfdinsert, sizeof(strfdinsert)) ==
	    sizeof(strfdinsert)) {

		print_strbuf(&strfdinsert.ctlbuf, "ctl");
		print_strbuf(&strfdinsert.databuf, "dat");

		(void) printf("%s\tflags=%s fildes=%d offset=%d\n",
			pname,
			strflags(strfdinsert.flags),
			strfdinsert.fildes,
			strfdinsert.offset);
	}
}

static CONST char *
strflags(flags)		/* strpeek and strfdinsert flags word */
long flags;
{
	register CONST char * s;

	switch (flags) {
	case 0:
		s = "0";
		break;
	case RS_HIPRI:
		s = "RS_HIPRI";
		break;
	default:
		(void) sprintf(code_buf, "0x%.4X", flags);
		s = code_buf;
	}

	return s;
}

static void
show_strrecvfd(Pr, offset)
process_t *Pr;
long offset;
{
	struct strrecvfd strrecvfd;

	if (Pread(Pr, offset, (char *)&strrecvfd, sizeof(strrecvfd)) ==
	    sizeof(strrecvfd)) {
		(void) printf(
			"%s\tfd=%-5d uid=%-5d gid=%d\n",
			pname,
			strrecvfd.fd,
			strrecvfd.uid,
			strrecvfd.gid);
	}
}

#ifdef I_LIST
static void
show_strlist(Pr, offset)
register process_t *Pr;
register long offset;
{
	struct str_list strlist;
	struct str_mlist list;
	register int count;

	if (Pread(Pr, offset, (char *)&strlist, sizeof(strlist)) ==
	    sizeof(strlist)) {
		(void) printf("%s\tnmods=%d  modlist=0x%.8lX\n",
			pname,
			strlist.sl_nmods,
			(long)strlist.sl_modlist);

		count = strlist.sl_nmods;
		offset = (long)strlist.sl_modlist;
		while (!interrupt && --count >= 0) {
			if (Pread(Pr, offset, (char *)&list, sizeof(list)) !=
			    sizeof(list))
				break;
			(void) printf("%s\t\t\"%.*s\"\n",
				pname,
				sizeof(list.l_name),
				list.l_name);
			offset += sizeof(struct str_mlist);
		}
	}
}
#endif

static void
show_jwinsize(Pr, offset)
register process_t *Pr;
register long offset;
{
	struct jwinsize jwinsize;

	if (Pread(Pr, offset, (char *)&jwinsize, sizeof(jwinsize)) ==
	    sizeof(jwinsize)) {
		(void) printf(
			"%s\tbytesx=%-3u bytesy=%-3u bitsx=%-3u bitsy=%-3u\n",
			pname,
			(unsigned)jwinsize.bytesx,
			(unsigned)jwinsize.bytesy,
			(unsigned)jwinsize.bitsx,
			(unsigned)jwinsize.bitsy);
	}
}

static void
show_winsize(Pr, offset)
register process_t *Pr;
register long offset;
{
	struct winsize winsize;

	if (Pread(Pr, offset, (char *)&winsize, sizeof(winsize)) ==
	    sizeof(winsize)) {
		(void) printf(
			"%s\trow=%-3d col=%-3d xpixel=%-3d ypixel=%-3d\n",
			pname,
			winsize.ws_row,
			winsize.ws_col,
			winsize.ws_xpixel,
			winsize.ws_ypixel);
	}
}

static void
show_statvfs(Pr)
process_t *Pr;
{
	register long offset;
	register char *cp, *cend, *bp;
	struct statvfs statvfs;

	if (sys_nargs > 1 && (offset = sys_args[1]) != NULL
	 && Pread(Pr, offset, (char *)&statvfs, sizeof(statvfs)) ==
	    sizeof(statvfs)) {
		(void) printf(
		"%s\tbsize=%-10lu frsize=%-9lu blocks=%-8lu bfree=%-9lu\n",
			pname,
			statvfs.f_bsize,
			statvfs.f_frsize,
			statvfs.f_blocks,
			statvfs.f_bfree);
		(void) printf(
		"%s\tbavail=%-9lu files=%-10lu ffree=%-9lu favail=%-9lu\n",
			pname,
			statvfs.f_bavail,
			statvfs.f_files,
			statvfs.f_ffree,
			statvfs.f_favail);
		(void) printf(
		"%s\tfsid=0x%-9.4X basetype=%-7.16s namemax=%u\n",
			pname,
			statvfs.f_fsid,
			statvfs.f_basetype,
			statvfs.f_namemax);
		(void) printf(
		"%s\tflag=%s\n",
			pname,
			svfsflags(statvfs.f_flag));
		cp = statvfs.f_fstr;
		cend = statvfs.f_fstr + sizeof(statvfs.f_fstr);
		bp = cend;
		while (cp < cend) {
			if (*cp == '\0')
				*cp = ' ';
			else
				bp = cp;
			cp++;
		}
		if (bp < cend-1)
			bp[1] = '\0';
		(void) printf( "%s\tfstr=\"%.*s\"\n",
			pname,
			sizeof(statvfs.f_fstr),
			statvfs.f_fstr);
	}
}

static void
show_statfs(Pr)
process_t *Pr;
{
	register long offset;
	struct statfs statfs;

	if (sys_nargs >= 2 && (offset = sys_args[1]) != NULL
	 && Pread(Pr, offset, (char *)&statfs, sizeof(statfs)) ==
	    sizeof(statfs)) {
		(void) printf(
		"%s\tfty=%d bsz=%d fsz=%d blk=%d bfr=%d fil=%d ffr=%d\n",
			pname,
			statfs.f_fstyp,
			statfs.f_bsize,
			statfs.f_frsize,
			statfs.f_blocks,
			statfs.f_bfree,
			statfs.f_files,
			statfs.f_ffree);
		(void) printf("%s\t    fname=%.6s fpack=%.6s\n",
			pname,
			statfs.f_fname,
			statfs.f_fpack);
	}
}

static void
show_fcntl(Pr)		/* print values in fcntl() pointed-to structure */
process_t *Pr;
{
	register long offset;

	if (sys_nargs < 3 || (offset = sys_args[2]) == NULL)
		return;

	switch (sys_args[1]) {
	case F_GETLK:
	case F_SETLK:
	case F_SETLKW:
	case F_FREESP:
	case F_ALLOCSP:
		show_flock(Pr, offset);
		break;
	}
}

static void
show_flock(Pr, offset)
process_t *Pr;
long offset;
{
	struct flock flock;

	if (Pread(Pr, offset, (char *)&flock, sizeof(flock)) ==
	    sizeof(flock)) {
		register CONST char * typ = NULL;

		(void) printf("%s\ttyp=", pname);

		switch (flock.l_type) {
		case F_RDLCK:
			typ = "F_RDLCK";
			break;
		case F_WRLCK:
			typ = "F_WRLCK";
			break;
		case F_UNLCK:
			typ = "F_UNLCK";
			break;
		}
		if (typ != NULL)
			(void) printf("%s", typ);
		else
			(void) printf("%-7d", flock.l_type);

		(void) printf(
			"  whence=%-2u start=%-5d len=%-5d sys=%-2u pid=%u\n",
			flock.l_whence,
			flock.l_start,
			flock.l_len,
			flock.l_sysid&0xffff,
			flock.l_pid&0xffff);
	}
}

static void
show_gp_msg(Pr)
process_t *Pr;
{
	register long offset;

	if (sys_nargs >= 2 && (offset = sys_args[1]) != NULL)
		show_strbuf(Pr, offset, "ctl");

	if (sys_nargs >= 3 && (offset = sys_args[2]) != NULL)
		show_strbuf(Pr, offset, "dat");
}

static void
show_strbuf(Pr, offset, name)
process_t *Pr;
long offset;
CONST char *name;
{
	struct strbuf strbuf;

	if (Pread(Pr, offset, (char *)&strbuf, sizeof(strbuf)) ==
	    sizeof(strbuf))
		print_strbuf(&strbuf, name);
}

static void
print_strbuf(sp, name)
struct strbuf *sp;
CONST char *name;
{
	(void) printf(
		"%s\t%s:  maxlen=%-4d len=%-4d buf=0x%.8X\n",
		pname,
		name,
		sp->maxlen,
		sp->len,
		sp->buf);
	/*
	 * Should we show the buffer contents?
	 * Keyed to the '-r fds' and '-w fds' options?
	 */
}

static void
show_poll(Pr)
register process_t *Pr;
{
	register long offset;
	register int nfds, skip = 0;

	if (sys_nargs >= 2
	 && (offset = sys_args[0]) != NULL
	 && (nfds = sys_args[1]) > 0) {
		if (nfds > 32) {	/* let's not be ridiculous */
			skip = nfds - 32;
			nfds = 32;
		}
		for ( ; nfds && !interrupt;
		    nfds--, offset += sizeof(struct pollfd))
			show_pollfd(Pr, offset);
		if (skip && !interrupt)
			(void) printf(
				"%s\t...skipping %d file descriptors...\n",
				pname,
				skip);
	}
}

static void
show_pollfd(Pr, offset)
process_t *Pr;
long offset;
{
	struct pollfd pollfd;

	if (Pread(Pr, offset, (char *)&pollfd, sizeof(pollfd)) ==
	    sizeof(pollfd)) {
		(void) printf(
			"%s\tfd=%-2d ev=%s rev=%s\n",
			pname,
			pollfd.fd,
			pollevent(pollfd.events),
			pollevent(pollfd.revents));
	}
}

static CONST char *
pollevent(arg)
register int arg;
{
	register char * str = code_buf;

	if (arg == 0)
		return "0";
	if (arg & ~(POLLIN|POLLPRI|POLLOUT|POLLERR|POLLHUP|POLLNVAL)) {
		(void) sprintf(str, "0%-6o", arg);
		return (CONST char *)str;
	}

	*str = '\0';
	if (arg & POLLIN)
		(void) strcat(str, "|POLLIN");
	if (arg & POLLPRI)
		(void) strcat(str, "|POLLPRI");
	if (arg & POLLOUT)
		(void) strcat(str, "|POLLOUT");
	if (arg & POLLERR)
		(void) strcat(str, "|POLLERR");
	if (arg & POLLHUP)
		(void) strcat(str, "|POLLHUP");
	if (arg & POLLNVAL)
		(void) strcat(str, "|POLLNVAL");

	return (CONST char *)(str+1);
}

static void
show_perm(ip)
register struct ipc_perm * ip;
{
	(void) printf(
		"%s\tu=%-5u g=%-5u cu=%-5u cg=%-5u m=0%.6o seq=%u key=%d\n",
		pname,
		ip->uid,
		ip->gid,
		ip->cuid,
		ip->cgid,
		ip->mode,
		ip->seq,
		ip->key);
}

static void
show_msgsys(Pr, msgsz)
process_t *Pr;
int msgsz;
{
	switch (sys_args[0]) {
	case 0:			/* msgget() */
		break;
	case 1:			/* msgctl() */
		if (sys_nargs > 3) {
			switch (sys_args[2]) {
			case IPC_STAT:
				if (Errno)
					break;
				/*FALLTHROUGH*/
			case IPC_SET:
				show_msgctl(Pr, (long)sys_args[3]);
				break;
			}
		}
		break;
	case 2:			/* msgrcv() */
		if (!Errno && sys_nargs > 2)
			show_msgbuf(Pr, (long)sys_args[2], msgsz);
		break;
	case 3:			/* msgsnd() */
		if (sys_nargs > 3)
			show_msgbuf(Pr, (long)sys_args[2], sys_args[3]);
		break;
	default:		/* unexpected subcode */
		break;
	}
}

static void
show_msgctl(Pr, offset)
register process_t *Pr;
register long offset;
{
	struct msqid_ds msgq;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&msgq, sizeof(msgq)) ==
	    sizeof(msgq)) {
		show_perm(&msgq.msg_perm);

		(void) printf(
		"%s\tbytes=%-5u msgs=%-5u maxby=%-5u lspid=%-5u lrpid=%-5u\n",
			pname,
			msgq.msg_cbytes,
			msgq.msg_qnum,
			msgq.msg_qbytes,
			msgq.msg_lspid,
			msgq.msg_lrpid);

		prtime("    st = ", msgq.msg_stime);
		prtime("    rt = ", msgq.msg_rtime);
		prtime("    ct = ", msgq.msg_ctime);
	}
}

static void
show_msgbuf(Pr, offset, msgsz)
register process_t *Pr;
register long offset;
int msgsz;
{
	struct msgbuf msgb;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&msgb, sizeof(long)) ==
	    sizeof(long)) {
		/* enter region of lengthy output */
		if (msgsz > BUFSIZ/4)
			Eserialize();

		(void) printf("%s\tmtype=%u  mtext[]=\n",
			pname,
			msgb.mtype);
		showbuffer(Pr, (long)(offset+sizeof(long)), msgsz);

		/* exit region of lengthy output */
		if (msgsz > BUFSIZ/4)
			Xserialize();
	}
}

static void
show_semsys(Pr)
process_t *Pr;
{
	switch (sys_args[0]) {
	case 0:			/* semctl() */
		if (sys_nargs > 4) {
			switch (sys_args[3]) {
			case IPC_STAT:
				if (Errno)
					break;
				/*FALLTHROUGH*/
			case IPC_SET:
				show_semctl(Pr, (long)sys_args[4]);
				break;
			}
		}
		break;
	case 1:			/* semget() */
		break;
	case 2:			/* semop() */
		if (sys_nargs > 3)
			show_semop(Pr, (long)sys_args[2], sys_args[3]);
		break;
	default:		/* unexpected subcode */
		break;
	}
}

static void
show_semctl(Pr, offset)
register process_t *Pr;
register long offset;
{
	struct semid_ds semds;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&semds, sizeof(semds)) ==
	    sizeof(semds)) {
		show_perm(&semds.sem_perm);

		(void) printf("%s\tnsems=%u\n",
			pname,
			semds.sem_nsems);

		prtime("    ot = ", semds.sem_otime);
		prtime("    ct = ", semds.sem_ctime);
	}
}

static void
show_semop(Pr, offset, nsops)
register process_t *Pr;
register long offset;
register int nsops;
{
	struct sembuf sembuf;
	register CONST char * str;

	if (offset == NULL)
		return;

	if (nsops > 40)		/* let's not be ridiculous */
		nsops = 40;

	for ( ; nsops > 0 && !interrupt; --nsops, offset += sizeof(sembuf)) {
		if (Pread(Pr, offset, (char *)&sembuf, sizeof(sembuf)) !=
		    sizeof(sembuf))
			break;

		(void) printf("%s\tsemnum=%-5u semop=%-5d semflg=",
			pname,
			sembuf.sem_num,
			sembuf.sem_op);

		if (sembuf.sem_flg == 0)
			(void) printf("0\n");
		else if ((str = semflags(sembuf.sem_flg)) != NULL)
			(void) printf("%s\n", str);
		else
			(void) printf("0%.6o\n", sembuf.sem_flg);
	}
}

static void
show_shmsys(Pr)
process_t *Pr;
{
	switch (sys_args[0]) {
	case 0:			/* shmat() */
		break;
	case 1:			/* shmctl() */
		if (sys_nargs > 3) {
			switch (sys_args[2]) {
			case IPC_STAT:
				if (Errno)
					break;
				/*FALLTHROUGH*/
			case IPC_SET:
				show_shmctl(Pr, (long)sys_args[3]);
				break;
			}
		}
		break;
	case 2:			/* shmdt() */
		break;
	case 3:			/* shmget() */
		break;
	default:		/* unexpected subcode */
		break;
	}
}

static void
show_shmctl(Pr, offset)
register process_t *Pr;
register long offset;
{
	struct shmid_ds shmds;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&shmds, sizeof(shmds)) ==
	    sizeof(shmds)) {
		show_perm(&shmds.shm_perm);

		(void) printf(
			"%s\tsize=%-6u lpid=%-5u cpid=%-5u na=%-5u cna=%u\n",
			pname,
			shmds.shm_segsz,
			shmds.shm_lpid,
			shmds.shm_cpid,
			shmds.shm_nattch,
			shmds.shm_cnattch);

		prtime("    at = ", shmds.shm_atime);
		prtime("    dt = ", shmds.shm_dtime);
		prtime("    ct = ", shmds.shm_ctime);
	}
}

static void
show_groups(Pr, offset, count)
	process_t *Pr;
	long offset;
	register int count;
{
	int groups[100];

	if (count > 100)
		count = 100;

	if (count > 0 && offset != NULL
	 && Pread(Pr, offset, (char *)&groups[0], (int)(count*sizeof(int))) ==
	    count*sizeof(int)) {
		register int n;

		(void) printf("%s\t", pname);
		for (n = 0; !interrupt && n < count; n++) {
			if (n != 0 && n%10 == 0)
				(void) printf("\n%s\t", pname);
			(void) printf("%6d", groups[n]);
		}
		(void) fputc('\n', stdout);
	}
}

static void
show_sigset(Pr, offset, name)
	process_t *Pr;
	long offset;
	CONST char * name;
{
	sigset_t sigset;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&sigset, sizeof(sigset)) ==
	    sizeof(sigset)) {
		(void) printf("%s\t%s =%s\n",
			pname, name, sigset_string(&sigset));
	}
}

/*
 * This assumes that a sigset_t is simply an array of longs.
 */
static char *
sigset_string(sp)
	sigset_t * sp;
{
	register char * s = code_buf;
	register int n = sizeof(*sp)/sizeof(long);
	register long * lp = (long *)sp;

	while (--n >= 0) {
		register long val = *lp++;

		if (val == 0)
			s += sprintf(s, " 0");
		else
			s += sprintf(s, " 0x%.8lX", val);
	}

	return code_buf;
}

static void
show_sigaltstack(Pr, offset, name)
	process_t *Pr;
	long offset;
	CONST char * name;
{
	struct sigaltstack altstack;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&altstack, sizeof(altstack)) ==
	    sizeof(altstack)) {
		(void) printf("%s\t%s: sp=0x%.8X size=%ld flags=0x%.4X\n",
			pname,
			name,
			altstack.ss_sp,
			altstack.ss_size,
			altstack.ss_flags);
	}
}

static void
show_sigaction(Pr, offset, name, odisp)
	process_t *Pr;
	long offset;
	CONST char * name;
	int odisp;
{
	struct sigaction sigaction;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&sigaction, sizeof(sigaction)) ==
	    sizeof(sigaction)) {
		/* This is stupid, we shouldn't have to do this */
		if (odisp != NULL)
			sigaction.sa_handler = (void (*)())odisp;
		(void) printf("%s    %s: hand = 0x%.8X mask =%s flags = 0x%.4X\n",
			pname,
			name,
			sigaction.sa_handler,
			sigset_string(&sigaction.sa_mask),
			sigaction.sa_flags);
	}
}

void
print_siginfo(sip)
register siginfo_t *sip;
{
	register CONST char * code = NULL;

	(void) printf("%s      siginfo: %s", pname, signame(sip->si_signo));

	if (sip->si_signo != 0 && SI_FROMUSER(sip) && sip->si_pid != 0) {
		(void) printf(" pid=%ld uid=%ld", sip->si_pid, sip->si_uid);
		if (sip->si_code != 0)
			(void) printf(" code=%d", sip->si_code);
		(void) fputc('\n', stdout);
		return;
	}

	switch (sip->si_signo) {
	default:
		(void) fputc('\n', stdout);
		return;
	case SIGILL:
	case SIGTRAP:
	case SIGFPE:
	case SIGSEGV:
	case SIGBUS:
	case SIGEMT:
	case SIGCLD:
	case SIGPOLL:
	case SIGXFSZ:
		break;
	}

	switch (sip->si_signo) {
	case SIGILL:
		switch (sip->si_code) {
		case ILL_ILLOPC:	code = "ILL_ILLOPC";	break;
		case ILL_ILLOPN:	code = "ILL_ILLOPN";	break;
		case ILL_ILLADR:	code = "ILL_ILLADR";	break;
		case ILL_ILLTRP:	code = "ILL_ILLTRP";	break;
		case ILL_PRVOPC:	code = "ILL_PRVOPC";	break;
		case ILL_PRVREG:	code = "ILL_PRVREG";	break;
		case ILL_COPROC:	code = "ILL_COPROC";	break;
		case ILL_BADSTK:	code = "ILL_BADSTK";	break;
		}
		break;
	case SIGTRAP:
		switch (sip->si_code) {
		case TRAP_BRKPT:	code = "TRAP_BRKPT";	break;
		case TRAP_TRACE:	code = "TRAP_TRACE";	break;
		}
		break;
	case SIGFPE:
		switch (sip->si_code) {
		case FPE_INTDIV:	code = "FPE_INTDIV";	break;
		case FPE_INTOVF:	code = "FPE_INTOVF";	break;
		case FPE_FLTDIV:	code = "FPE_FLTDIV";	break;
		case FPE_FLTOVF:	code = "FPE_FLTOVF";	break;
		case FPE_FLTUND:	code = "FPE_FLTUND";	break;
		case FPE_FLTRES:	code = "FPE_FLTRES";	break;
		case FPE_FLTINV:	code = "FPE_FLTINV";	break;
		case FPE_FLTSUB:	code = "FPE_FLTSUB";	break;
		}
		break;
	case SIGSEGV:
		switch (sip->si_code) {
		case SEGV_MAPERR:	code = "SEGV_MAPERR";	break;
		case SEGV_ACCERR:	code = "SEGV_ACCERR";	break;
		}
		break;
	case SIGBUS:
	case SIGEMT:
		switch (sip->si_code) {
		case BUS_ADRALN:	code = "BUS_ADRALN";	break;
		case BUS_ADRERR:	code = "BUS_ADRERR";	break;
		case BUS_OBJERR:	code = "BUS_OBJERR";	break;
		}
		break;
	case SIGCLD:
		switch (sip->si_code) {
		case CLD_EXITED:	code = "CLD_EXITED";	break;
		case CLD_KILLED:	code = "CLD_KILLED";	break;
		case CLD_DUMPED:	code = "CLD_DUMPED";	break;
		case CLD_TRAPPED:	code = "CLD_TRAPPED";	break;
		case CLD_STOPPED:	code = "CLD_STOPPED";	break;
		case CLD_CONTINUED:	code = "CLD_CONTINUED";	break;
		}
		break;
	case SIGPOLL:
		switch (sip->si_code) {
		case POLL_IN:		code = "POLL_IN";	break;
		case POLL_OUT:		code = "POLL_OUT";	break;
		case POLL_MSG:		code = "POLL_MSG";	break;
		case POLL_ERR:		code = "POLL_ERR";	break;
		case POLL_PRI:		code = "POLL_PRI";	break;
		case POLL_HUP:		code = "POLL_HUP";	break;
		}
		break;
	}

	if (code == NULL) {
		(void) sprintf(code_buf, "code=%d", sip->si_code);
		code = (CONST char *)code_buf;
	}

	switch (sip->si_signo) {
	case SIGILL:
	case SIGTRAP:
	case SIGFPE:
	case SIGSEGV:
	case SIGBUS:
	case SIGEMT:
		(void) printf(" %s addr=0x%.8X",
			code,
			sip->si_addr);
		break;
	case SIGCLD:
		(void) printf(" %s pid=%ld uid=%ld status=0x%.4X",
			code,
			sip->si_pid,
			sip->si_uid,
			sip->si_status);
		break;
	case SIGPOLL:
	case SIGXFSZ:
		(void) printf(" %s fd=%d band=%ld",
			code,
			sip->si_fd,
			sip->si_band);
		break;
	}

	if (sip->si_errno != 0) {
		register CONST char * ename = errname(sip->si_errno);

		(void) printf(" errno=%d", sip->si_errno);
		if (ename != NULL)
			(void) printf("(%s)", ename);
	}

	(void) fputc('\n', stdout);
}

static void
show_bool(Pr, offset, count)
	process_t *Pr;
	register long offset;
	register int count;
{
	int serial = (count > BUFSIZ/4);

	/* enter region of lengthy output */
	if (serial)
		Eserialize();

	while (count > 0) {
		char buf[32];
		register int nb = (count < 32)? count : 32;
		register int i;

		if (Pread(Pr, offset, buf, nb) != nb)
			break;

		(void) printf("%s   ", pname);
		for (i = 0; i < nb; i++)
			(void) printf(" %d", buf[i]);
		(void) fputc('\n', stdout);

		count -= nb;
		offset += nb;
	}

	/* exit region of lengthy output */
	if (serial)
		Xserialize();
}

static void
show_iovec(Pr, offset, niov, showbuf, count)
	process_t *Pr;
	long offset;
	register int niov;
	register int showbuf;
	register long count;
{
	iovec_t iovec[16];
	register iovec_t * ip;
	register int nb;
	int serial = (count > BUFSIZ/4 && showbuf);

	if (niov > 16)		/* is this the real limit? */
		niov = 16;

	if (offset != NULL && niov > 0
	 && Pread(Pr, offset, (char *)&iovec[0], (int)(niov*sizeof(iovec_t))) ==
	    niov*sizeof(iovec_t)) {
		/* enter region of lengthy output */
		if (serial)
			Eserialize();

		for (ip = &iovec[0]; niov-- && !interrupt; ip++) {
			(void) printf("%s\tiov_base = 0x%.8X  iov_len = %d\n",
				pname,
				ip->iov_base,
				ip->iov_len);
			if ((nb = count) > 0) {
				if (nb > ip->iov_len)
					nb = ip->iov_len;
				if (nb > 0)
					count -= nb;
			}
			if (showbuf && nb > 0)
				showbuffer(Pr, (long)ip->iov_base, nb);
		}

		/* exit region of lengthy output */
		if (serial)
			Xserialize();
	}
}

static void
show_dents(Pr, offset, count)
	register process_t *Pr;
	register long offset;
	register int count;
{
	long buf[BUFSIZ/sizeof(long)];
	register struct dirent *dp;
	int serial = (count > 100);

	if (offset == NULL)
		return;

	/* enter region of lengthy output */
	if (serial)
		Eserialize();

	while (count > 0 && !interrupt) {
		register int nb = count < BUFSIZ? count : BUFSIZ;

		if ((nb = Pread(Pr, offset, (char *)&buf[0], nb)) <= 0)
			break;

		dp = (struct dirent *)&buf[0];
		if (nb < (int)(dp->d_name - (char *)dp))
			break;
		if ((unsigned)nb < dp->d_reclen) {
			/* getdents() error? */
			(void) printf(
			"%s    ino=%-5d off=%-4d rlen=%-3d\n",
				pname,
				dp->d_ino,
				dp->d_off,
				dp->d_reclen);
			break;
		}

		while (!interrupt
		  && nb >= (int)(dp->d_name - (char *)dp)
		  && (unsigned)nb >= dp->d_reclen) {
			(void) printf(
			"%s    ino=%-5d off=%-4d rlen=%-3d \"%.*s\"\n",
				pname,
				dp->d_ino,
				dp->d_off,
				dp->d_reclen,
				dp->d_reclen - (int)(dp->d_name - (char *)dp),
				dp->d_name);
			nb -= dp->d_reclen;
			count -= dp->d_reclen;
			offset += dp->d_reclen;
			dp = (struct dirent *)((char *)dp + dp->d_reclen);
		}
	}

	/* exit region of lengthy output */
	if (serial)
		Xserialize();
}

static void
show_rlimit(Pr, offset)
	process_t *Pr;
	long offset;
{
	struct rlimit rlimit;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&rlimit, sizeof(rlimit)) ==
	    sizeof(rlimit)) {
		(void) printf("%s\t", pname);
		if (rlimit.rlim_cur == (ulong)RLIM_INFINITY)
			(void) fputs("cur = RLIM_INFINITY", stdout);
		else
			(void) printf("cur = %lu", rlimit.rlim_cur);
		if (rlimit.rlim_max == (ulong)RLIM_INFINITY)
			(void) fputs("  max = RLIM_INFINITY\n", stdout);
		else
			(void) printf("  max = %lu\n", rlimit.rlim_max);
	}
}

static void
show_nuname(Pr, offset)
process_t *Pr;
long offset;
{
	struct utsname ubuf;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&ubuf, sizeof(ubuf)) ==
	    sizeof(ubuf)) {
		(void) printf(
		"%s\tsys=%s nod=%s rel=%s ver=%s mch=%s\n",
			pname,
			ubuf.sysname,
			ubuf.nodename,
			ubuf.release,
			ubuf.version,
			ubuf.machine);
	}
}

static void
show_adjtime(Pr, off1, off2)
register process_t *Pr;
long off1, off2;
{
	register long offset;
	struct timeval delta;

	if (Pread(Pr, off1, (char *)&delta, sizeof(delta)) == sizeof(delta))
		(void) printf("delta = %ld sec %ld usec\n",
			delta.tv_sec, delta.tv_usec);
	if (Pread(Pr, off2, (char *)&delta, sizeof(delta)) == sizeof(delta))
		(void) printf("olddelta = %ld sec %ld usec\n",
			delta.tv_sec, delta.tv_usec);
}

void
prtime(name, value)
CONST char * name;
time_t value;
{
	struct tm *tp = localtime(&value);
	char * timp = asctime(tp);
	char * tzp = tzname[tp->tm_isdst? 1 : 0];

	*(timp+19) = '\0';
	*(timp+24) = '\0';
	(void) printf("%s\t%s%s %s %s  [ %lu ]\n",
		pname, name, timp+4, tzp, timp+20, value);
}

void
prtimestruc(name, value)
CONST char * name;
timestruc_t value;
{
	prtime(name, value.tv_sec);
}
