/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:print.c	1.9.3.1"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/nserve.h>
#include <sys/mman.h>
#include <sys/rf_sys.h>
#include <sys/resource.h>
#include <sys/ulimit.h>
#include <sys/utsname.h>
#include <stropts.h>

#include <sys/fault.h>
#include <sys/syscall.h>
#include <sys/systeminfo.h>
#include "pcontrol.h"
#include "print.h"
#include "ramdata.h"
#include "proto.h"

extern char * realloc();

/*
 * Function prototypes for static routines in this module.
 */
#if	defined(__STDC__)

static	void	prt_nov( int , int );
static	void	prt_dec( int , int );
static	void	prt_oct( int , int );
static	void	prt_hex( int , int );
static	void	prt_hhx( int , int );
static	void	prt_dex( int , int );
static	void	prt_stg( int , int );
static	void	prt_rst( int , int );
static	void	prt_rlk( int , int );
static	void	prt_ioc( int , int );
static	void	prt_ioa( int , int );
static	void	prt_fcn( int , int );

#ifdef i386
static	void	prt_si86( int , int );
#else
static	void	prt_s3b( int , int );
#endif

static	void	prt_uts( int , int );
static	void	prt_msc( int , int );
static	void	prt_msf( int , int );
static	void	prt_sec( int , int );
static	void	prt_sef( int , int );
static	void	prt_shc( int , int );
static	void	prt_shf( int , int );
static	void	prt_sfs( int , int );
static	void	prt_rfs( int , int );
static	void	prt_opn( int , int );
static	void	prt_sig( int , int );
static	void	prt_six( int , int );
static	void	prt_act( int , int );
static	void	prt_smf( int , int );
static	void	prt_rv1( int , int );
static	void	prt_rv2( int , int );
static	void	prt_rv3( int , int );
static	void	prt_plk( int , int );
static	void	prt_mtf( int , int );
static	void	prt_mft( int , int );
static	void	prt_iob( int , int );
static	void	prt_wop( int , int );
static	void	prt_spm( int , int );
static	void	prt_mpr( int , int );
static	void	prt_mty( int , int );
static	void	prt_mcf( int , int );
static	void	prt_mc4( int , int );
static	void	prt_mc5( int , int );
static	void	prt_mad( int , int );
static	void	prt_ulm( int , int );
static	void	prt_rlm( int , int );
static	void	prt_cnf( int , int );
static	void	prt_inf( int , int );
static	void	prt_ptc( int , int );
static	void	prt_fui( int , int );
static	void	grow( int );
static	CONST char *	mmap_protect( int );
static	CONST char *	mmap_type( int );
static	void	prt_cxen( int , int );

#else	/* defined(__STDC__) */

static	void	prt_nov();
static	void	prt_dec();
static	void	prt_oct();
static	void	prt_hex();
static	void	prt_hhx();
static	void	prt_dex();
static	void	prt_stg();
static	void	prt_rst();
static	void	prt_rlk();
static	void	prt_ioc();
static	void	prt_ioa();
static	void	prt_fcn();

#ifdef i386
static	void	prt_si86();
#else
static	void	prt_s3b();
#endif

static	void	prt_uts();
static	void	prt_msc();
static	void	prt_msf();
static	void	prt_sec();
static	void	prt_sef();
static	void	prt_shc();
static	void	prt_shf();
static	void	prt_sfs();
static	void	prt_rfs();
static	void	prt_opn();
static	void	prt_sig();
static	void	prt_six();
static	void	prt_act();
static	void	prt_smf();
static	void	prt_rv1();
static	void	prt_rv2();
static	void	prt_rv3();
static	void	prt_plk();
static	void	prt_mtf();
static	void	prt_mft();
static	void	prt_iob();
static	void	prt_wop();
static	void	prt_spm();
static	void	prt_mpr();
static	void	prt_mty();
static	void	prt_mcf();
static	void	prt_mc4();
static	void	prt_mc5();
static	void	prt_mad();
static	void	prt_ulm();
static	void	prt_rlm();
static	void	prt_cnf();
static	void	prt_inf();
static	void	prt_ptc();
static	void	prt_fui();
static	void	grow();
static	CONST char *	mmap_protect();
static	CONST char *	mmap_type();
static	void	prt_cxen();

#endif	/* defined(__STDC__) */

#define GROW(nb) if (sys_leng+(nb) >= sys_ssize) grow(nb)

/*ARGSUSED*/
static void
prt_nov(val, raw)	/* print nothing */
int val;
int raw;
{
}

/*ARGSUSED*/
static void
prt_dec(val, raw)	/* print as decimal */
int val;
int raw;
{
	GROW(12);
	sys_leng += sprintf(sys_string+sys_leng, "%d", val);
}

/*ARGSUSED*/
static void
prt_oct(val, raw)	/* print as octal */
int val;
int raw;
{
	GROW(12);
	sys_leng += sprintf(sys_string+sys_leng, "%#o", val);
}

/*ARGSUSED*/
static void
prt_hex(val, raw)	/* print as hexadecimal */
int val;
int raw;
{
	GROW(10);
	sys_leng += sprintf(sys_string+sys_leng, "0x%.8X", val);
}

/*ARGSUSED*/
static void
prt_hhx(val, raw)	/* print as hexadecimal (half size) */
int val;
int raw;
{
	GROW(10);
	sys_leng += sprintf(sys_string+sys_leng, "0x%.4X", val);
}

/*ARGSUSED*/
static void
prt_dex(val, raw)	/* print as decimal if small, else hexadecimal */
int val;
int raw;
{
	GROW(12);
	if (val & 0xff000000)
		prt_hex(val, 0);
	else
		prt_dec(val, 0);
}

static void
prt_stg(val, raw)	/* print as string */
int val;
int raw;
{
	register char * s = raw? NULL : fetchstring((long)val, 400);

	if (s == NULL)
		prt_hex(val, 0);
	else {
		GROW((int)strlen(s)+2);
		sys_leng += sprintf(sys_string+sys_leng, "\"%s\"", s);
	}
}

static void
prt_rst(val, raw)	/* print as string returned from syscall */
int val;
int raw;
{
	register char * s = (raw || Errno)? NULL : fetchstring((long)val, 400);

	if (s == NULL)
		prt_hex(val, 0);
	else {
		GROW((int)strlen(s)+2);
		sys_leng += sprintf(sys_string+sys_leng, "\"%s\"", s);
	}
}

static void
prt_rlk(val, raw)	/* print contents of readlink() buffer */
int val;		/* address of buffer */
int raw;
{
	register char * s = (raw || Errno || Rval1 <= 0)? NULL :
				fetchstring((long)val, (Rval1>400)?400:Rval1);

	if (s == NULL)
		prt_hex(val, 0);
	else {
		GROW((int)strlen(s)+2);
		sys_leng += sprintf(sys_string+sys_leng, "\"%s\"", s);
	}
}

static void
prt_ioc(val, raw)	/* print ioctl code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : ioctlname(val);

	if (s == NULL)
		prt_hex(val, 0);
	else
		outstring(s);
}

static void
prt_ioa(val, raw)	/* print ioctl argument */
int val;
int raw;
{
	register CONST char * s;

	switch(sys_args[1]) {	/* cheating -- look at the ioctl() code */

	/* streams ioctl()s */
	case I_LOOK:
		prt_rst(val, raw);
		break;
	case I_PUSH:
	case I_FIND:
		prt_stg(val, raw);
		break;
	case I_LINK:
	case I_UNLINK:
	case I_SENDFD:
		prt_dec(val, 0);
		break;
	case I_SRDOPT:
		if (raw || (s = strrdopt(val)) == NULL)
			prt_dec(val, 0);
		else
			outstring(s);
		break;
	case I_SETSIG:
		if (raw || (s = strevents(val)) == NULL)
			prt_hex(val, 0);
		else
			outstring(s);
		break;
	case I_FLUSH:
		if (raw || (s = strflush(val)) == NULL)
			prt_dec(val, 0);
		else
			outstring(s);
		break;

	/* tty ioctl()s */
	case TCSBRK:
	case TCXONC:
	case TCFLSH:
	case TCDSET:
		prt_dec(val, 0);
		break;

	default:
		prt_hex(val, 0);
		break;
	}
}

static void
prt_fcn(val, raw)	/* print fcntl code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : fcntlname(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void

#ifdef i386
prt_si86(val, raw)	/* print sysi86 code */
#else
prt_s3b(val, raw)	/* print sys3b code */
#endif

int val;
int raw;
{

#ifdef i386
	register CONST char * s = raw? NULL : si86name(val);
#else
	register CONST char * s = raw? NULL : s3bname(val);
#endif

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

#ifdef i386 /* XENIX Support */
extern char *cxenixname();

static void
prt_cxen(val, raw)	/* print cxenix code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : cxenixname(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}
#else
static void
prt_cxen(val, raw)	/* Stub for non 386 boxes */
int val;
int raw;
{
		prt_dec(val, 0);
}
#endif /* i386 */

static void
prt_uts(val, raw)	/* print utssys code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : utscode(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_msc(val, raw)	/* print msgsys command */
int val;
int raw;
{
	register CONST char * s = raw? NULL : msgcmd(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_msf(val, raw)	/* print msgsys flags */
int val;
int raw;
{
	register CONST char * s = raw? NULL : msgflags(val);

	if (s == NULL)
		prt_oct(val, 0);
	else
		outstring(s);
}

static void
prt_sec(val, raw)	/* print semsys command */
int val;
int raw;
{
	register CONST char * s = raw? NULL : semcmd(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_sef(val, raw)	/* print semsys flags */
int val;
int raw;
{
	register CONST char * s = raw? NULL : semflags(val);

	if (s == NULL)
		prt_oct(val, 0);
	else
		outstring(s);
}

static void
prt_shc(val, raw)	/* print shmsys command */
int val;
int raw;
{
	register CONST char * s = raw? NULL : shmcmd(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_shf(val, raw)	/* print shmsys flags */
int val;
int raw;
{
	register CONST char * s = raw? NULL : shmflags(val);

	if (s == NULL)
		prt_oct(val, 0);
	else
		outstring(s);
}

static void
prt_sfs(val, raw)	/* print sysfs code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : sfsname(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_rfs(val, raw)	/* print rfsys code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : rfsysname(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_opn(val, raw)	/* print open code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : openarg(val);

	if (s == NULL)
		prt_oct(val, 0);
	else
		outstring(s);
}

static void
prt_sig(val, raw)	/* print signal name plus flags */
int val;
int raw;
{
	register CONST char * s = raw? NULL : sigarg(val);

	if (s == NULL)
		prt_hex(val, 0);
	else
		outstring(s);
}

static void
prt_six(val, raw)	/* print signal name, masked with SIGNO_MASK */
int val;
int raw;
{
	register CONST char * s = raw? NULL : sigarg(val & SIGNO_MASK);

	if (s == NULL)
		prt_hex(val, 0);
	else
		outstring(s);
}

static void
prt_act(val, raw)	/* print signal action value */
int val;
int raw;
{
	register CONST char * s;

	if (raw)
		s = NULL;
	else if (val == (int)SIG_DFL)
		s = "SIG_DFL";
	else if (val == (int)SIG_IGN)
		s = "SIG_IGN";
	else if (val == (int)SIG_HOLD)
		s = "SIG_HOLD";
	else
		s = NULL;

	if (s == NULL)
		prt_hex(val, 0);
	else
		outstring(s);
}

static void
prt_smf(val, raw)	/* print streams message flags */
int val;
int raw;
{
	switch (val) {
	case 0:
		prt_dec(val, 0);
		break;
	case RS_HIPRI:
		if (raw)
			prt_hhx(val, 0);
		else
			outstring("RS_HIPRI");
		break;
	default:
		prt_hhx(val, 0);
		break;
	}
}

static void
prt_rv1(val, raw)	/* print RFS verification argument */
int val;
int raw;
{
	register CONST char * s = NULL;

	switch (val) {
	case V_SET:
		s = "V_SET";
		break;
	case V_CLEAR:
		s = "V_CLEAR";
		break;
	case V_GET:
		s = "V_GET";
		break;
	}

	if (raw || s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_rv2(val, raw)	/* print RFS version argument */
int val;
int raw;
{
	register CONST char * s = NULL;

	switch (val) {
	case VER_CHECK:
		s = "VER_CHECK";
		break;
	case VER_GET:
		s = "VER_GET";
		break;
	}

	if (raw || s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

/*ARGSUSED*/
static void
prt_rv3(val, raw)	/* print RFS tuneable argument */
int val;
int raw;
{
	register CONST char * s = NULL;

#ifndef SVR3
	switch (val) {
	case T_NSRMOUNT:
		s = "T_NSRMOUNT";
		break;
	case T_NADVERTISE:
		s = "T_NADVERTISE";
		break;
	}
#endif

	if (raw || s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_plk(val, raw)	/* print plock code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : plockname(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_mtf(val, raw)	/* print mount flags */
int val;
int raw;
{
	register CONST char * s = raw? NULL : mountflags(val);

	if (s == NULL)
		prt_hex(val, 0);
	else
		outstring(s);
}

static void
prt_mft(val, raw)	/* print mount file system type */
int val;
int raw;
{
	if (val >= 0 && val < 256)
		prt_dec(val, 0);
	else if (raw)
		prt_hex(val, 0);
	else
		prt_stg(val, raw);
}

static void
prt_iob(val, raw)	/* print contents of read() or write() I/O buffer */
register int val;	/* address of I/O buffer (sys_args[1]) */
int raw;
{
	register process_t *Pr = &Proc;
	register int fdp1 = sys_args[0]+1;
	register int nbyte = Pr->why.pr_what==SYS_write? sys_args[2] :
				(Errno? 0 : Rval1);
	register int elsewhere = FALSE;		/* TRUE iff dumped elsewhere */
	char buffer[IOBSIZE];

	iob_buf[0] = '\0';

	if (Pr->why.pr_why == PR_SYSEXIT && nbyte > IOBSIZE) {
		switch (Pr->why.pr_what) {
		case SYS_read:
			elsewhere = prismember(&readfd, fdp1);
			break;
		case SYS_write:
			elsewhere = prismember(&writefd, fdp1);
			break;
		}
	}

	if (nbyte <= 0 || elsewhere)
		prt_hex(val, 0);
	else {
		register int nb = nbyte>IOBSIZE? IOBSIZE : nbyte;

		if (Pread(Pr, (long)val, buffer, nb) != nb)
			prt_hex(val, 0);
		else {
			iob_buf[0] = '"';
			showbytes(buffer, nb, iob_buf+1);
			(void) strcat(iob_buf,
				(nb == nbyte)?
				    (CONST char *)"\"" : (CONST char *)"\"..");
			if (raw)
				prt_hex(val, 0);
			else
				outstring(iob_buf);
		}
	}
}

static void
prt_wop(val, raw)	/* print waitsys() options */
int val;
int raw;
{
	register CONST char * s = raw? NULL : woptions(val);

	if (s == NULL)
		prt_oct(val, 0);
	else
		outstring(s);
}

/*ARGSUSED*/
static void
prt_spm(val, raw)	/* print sigprocmask argument */
int val;
int raw;
{
	register CONST char * s = NULL;

	if (!raw) {
		switch (val) {
		case SIG_BLOCK:		s = "SIG_BLOCK";	break;
		case SIG_UNBLOCK:	s = "SIG_UNBLOCK";	break;
		case SIG_SETMASK:	s = "SIG_SETMASK";	break;
		}
	}

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static CONST char *
mmap_protect(arg)
register int arg;
{
	register char * str = code_buf;

	if (arg & ~(PROT_READ|PROT_WRITE|PROT_EXEC))
		return (char *)NULL;
	
	if (arg == PROT_NONE)
		return "PROT_NONE";

	*str = '\0';
	if (arg & PROT_READ)
		(void) strcat(str, "|PROT_READ");
	if (arg & PROT_WRITE)
		(void) strcat(str, "|PROT_WRITE");
	if (arg & PROT_EXEC)
		(void) strcat(str, "|PROT_EXEC");
	return (CONST char *)(str+1);
}

static CONST char *
mmap_type(arg)
register int arg;
{
	register char * str = code_buf;

	switch (arg&MAP_TYPE) {
	case MAP_SHARED:
		(void) strcpy(str, "MAP_SHARED");
		break;
	case MAP_PRIVATE:
		(void) strcpy(str, "MAP_PRIVATE");
		break;
	default:
		(void) sprintf(str, "%d", arg&MAP_TYPE);
		break;
	}

	arg &= ~(_MAP_NEW|MAP_TYPE);

	if (arg & ~(MAP_FIXED|MAP_RENAME|MAP_NORESERVE))
		(void) sprintf(str+strlen(str), "|0x%X", arg);
	else {
		if (arg & MAP_FIXED)
			(void) strcat(str, "|MAP_FIXED");
		if (arg & MAP_RENAME)
			(void) strcat(str, "|MAP_RENAME");
		if (arg & MAP_NORESERVE)
			(void) strcat(str, "|MAP_NORESERVE");
	}

	return (CONST char *)str;
}

static void
prt_mpr(val, raw)	/* print mmap()/mprotect() flags */
int val;
int raw;
{
	register CONST char * s = raw? NULL : mmap_protect(val);

	if (s == NULL)
		prt_hhx(val, 0);
	else
		outstring(s);
}

static void
prt_mty(val, raw)	/* print mmap() mapping type flags */
int val;
int raw;
{
	register CONST char * s = raw? NULL : mmap_type(val);

	if (s == NULL)
		prt_hhx(val, 0);
	else
		outstring(s);
}

/*ARGSUSED*/
static void
prt_mcf(val, raw)	/* print memcntl() function */
int val;
int raw;
{
	register CONST char * s = NULL;

	if (!raw) {
		switch (val) {
		case MC_SYNC:		s = "MC_SYNC";		break;
		case MC_LOCK:		s = "MC_LOCK";		break;
		case MC_UNLOCK:		s = "MC_UNLOCK";	break;
		case MC_ADVISE:		s = "MC_ADVISE";	break;
		case MC_LOCKAS:		s = "MC_LOCKAS";	break;
		case MC_UNLOCKAS:	s = "MC_UNLOCKAS";	break;
		}
	}

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_mc4(val, raw)	/* print memcntl() (fourth) argument */
int val;
int raw;
{
	if (val == 0)
		prt_dec(val, 0);
	else if (raw)
		prt_hhx(val, 0);
	else {
		register char * s = NULL;

		switch (sys_args[2]) { /* cheating -- look at memcntl func */
		case MC_SYNC:
			if ((val & ~(MS_ASYNC|MS_INVALIDATE)) == 0) {
				*(s = code_buf) = '\0';
				if (val & MS_ASYNC)
					(void) strcat(s, "|MS_ASYNC");
				if (val & MS_INVALIDATE)
					(void) strcat(s, "|MS_INVALIDATE");
			}
			break;

		case MC_LOCKAS:
		case MC_UNLOCKAS:
			if ((val & ~(MCL_CURRENT|MCL_FUTURE)) == 0) {
				*(s = code_buf) = '\0';
				if (val & MCL_CURRENT)
					(void) strcat(s, "|MCL_CURRENT");
				if (val & MCL_FUTURE)
					(void) strcat(s, "|MCL_FUTURE");
			}
			break;
		}

		if (s == NULL)
			prt_hhx(val, 0);
		else
			outstring(++s);
	}
}

static void
prt_mc5(val, raw)	/* print memcntl() (fifth) argument */
int val;
int raw;
{
	register char * s;

	if (val == 0)
		prt_dec(val, 0);
	else if (raw || (val & ~VALID_ATTR))
		prt_hhx(val, 0);
	else {
		s = code_buf;
		*s = '\0';
		if (val & SHARED)
			strcat(s, "|SHARED");
		if (val & PRIVATE)
			strcat(s, "|PRIVATE");
		if (val & PROT_READ)
			(void) strcat(s, "|PROT_READ");
		if (val & PROT_WRITE)
			(void) strcat(s, "|PROT_WRITE");
		if (val & PROT_EXEC)
			(void) strcat(s, "|PROT_EXEC");
		if (*s == '\0')
			prt_hhx(val, 0);
		else
			outstring(++s);
	}
}

static void
prt_mad(val, raw)	/* print madvise() argument */
int val;
int raw;
{
	register CONST char * s = NULL;

	if (!raw) {
		switch (val) {
		case MADV_NORMAL:	s = "MADV_NORMAL";	break;
		case MADV_RANDOM:	s = "MADV_RANDOM";	break;
		case MADV_SEQUENTIAL:	s = "MADV_SEQUENTIAL";	break;
		case MADV_WILLNEED:	s = "MADV_WILLNEED";	break;
		case MADV_DONTNEED:	s = "MADV_DONTNEED";	break;
		}
	}

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_ulm(val, raw)	/* print ulimit() argument */
int val;
int raw;
{
	register CONST char * s = NULL;

	if (!raw) {
		switch (val) {
		case UL_GFILLIM:	s = "UL_GFILLIM";	break;
		case UL_SFILLIM:	s = "UL_SFILLIM";	break;
		case UL_GMEMLIM:	s = "UL_GMEMLIM";	break;
		case UL_GDESLIM:	s = "UL_GDESLIM";	break;
		}
	}

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_rlm(val, raw)	/* print get/setrlimit() argument */
int val;
int raw;
{
	register CONST char * s = NULL;

	if (!raw) {
		switch (val) {
		case RLIMIT_CPU:	s = "RLIMIT_CPU";	break;
		case RLIMIT_FSIZE:	s = "RLIMIT_FSIZE";	break;
		case RLIMIT_DATA:	s = "RLIMIT_DATA";	break;
		case RLIMIT_STACK:	s = "RLIMIT_STACK";	break;
		case RLIMIT_CORE:	s = "RLIMIT_CORE";	break;
		case RLIMIT_NOFILE:	s = "RLIMIT_NOFILE";	break;
		case RLIMIT_VMEM:	s = "RLIMIT_VMEM";	break;
		}
	}

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_cnf(val, raw)	/* print sysconfig code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : sconfname(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_inf(val, raw)	/* print systeminfo code */
int val;
int raw;
{
	register CONST char * s = NULL;

	if (!raw) {
		switch (val) {
		case SI_SYSNAME:	s = "SI_SYSNAME";	break;
		case SI_HOSTNAME:	s = "SI_HOSTNAME";	break;
		case SI_RELEASE:	s = "SI_RELEASE";	break;
		case SI_VERSION:	s = "SI_VERSION";	break;
		case SI_MACHINE:	s = "SI_MACHINE";	break;
		case SI_ARCHITECTURE:	s = "SI_ARCHITECTURE";	break;
		case SI_HW_SERIAL:	s = "SI_HW_SERIAL";	break;
		case SI_HW_PROVIDER:	s = "SI_HW_PROVIDER";	break;
		case SI_SRPC_DOMAIN:	s = "SI_SRPC_DOMAIN";	break;
		case SI_SET_HOSTNAME:	s = "SI_SET_HOSTNAME";	break;
		case SI_SET_SRPC_DOMAIN: s = "SI_SET_SRPC_DOMAIN"; break;
		}
	}

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_ptc(val, raw)	/* print pathconf code */
int val;
int raw;
{
	register CONST char * s = raw? NULL : pathconfname(val);

	if (s == NULL)
		prt_dec(val, 0);
	else
		outstring(s);
}

static void
prt_fui(val, raw)	/* print fusers() input argument */
int val;
int raw;
{
	register CONST char * s = raw? NULL : fuiname(val);

	if (s == NULL)
		prt_hhx(val, 0);
	else
		outstring(s);
}

void
outstring(s)
register CONST char * s;
{
	register int len = strlen(s);

	GROW(len);
	(void) strcpy(sys_string+sys_leng, s);
	sys_leng += len;
}

static void
grow(nbyte)	/* reallocate format buffer if necessary */
register int nbyte;
{
	while (sys_leng+nbyte >= sys_ssize) {
		sys_string = realloc(sys_string, sys_ssize *= 2);
		if (sys_string == NULL)
			abend("cannot reallocate format buffer", 0);
	}
}


/* array of pointers to print functions, one for each format */

void (* CONST Print[])() = {
	prt_nov,	/* NOV -- no value */
	prt_dec,	/* DEC -- print value in decimal */
	prt_oct,	/* OCT -- print value in octal */
	prt_hex,	/* HEX -- print value in hexadecimal */
	prt_dex,	/* DEX -- print value in hexadecimal if big enough */
	prt_stg,	/* STG -- print value as string */
	prt_ioc,	/* IOC -- print ioctl code */
	prt_fcn,	/* FCN -- print fcntl code */

#ifdef i386
	prt_si86,	/* SI86 -- print sysi86 code */
#else
	prt_s3b,	/* S3B -- print sys3b code */
#endif

	prt_uts,	/* UTS -- print utssys code */
	prt_opn,	/* OPN -- print open code */
	prt_sig,	/* SIG -- print signal name plus flags */
	prt_act,	/* ACT -- print signal action value */
	prt_rfs,	/* RFS -- print rfsys code */
	prt_rv1,	/* RV1 -- print RFS verification argument */
	prt_rv2,	/* RV2 -- print RFS version argument */
	prt_rv3,	/* RV3 -- print RFS tuneable argument */
	prt_msc,	/* MSC -- print msgsys command */
	prt_msf,	/* MSF -- print msgsys flags */
	prt_sec,	/* SEC -- print semsys command */
	prt_sef,	/* SEF -- print semsys flags */
	prt_shc,	/* SHC -- print shmsys command */
	prt_shf,	/* SHF -- print shmsys flags */
	prt_plk,	/* PLK -- print plock code */
	prt_sfs,	/* SFS -- print sysfs code */
	prt_rst,	/* RST -- print string returned by syscall */
	prt_smf,	/* SMF -- print streams message flags */
	prt_ioa,	/* IOA -- print ioctl argument */
	prt_six,	/* SIX -- print signal, masked with SIGNO_MASK */
	prt_mtf,	/* MTF -- print mount flags */
	prt_mft,	/* MFT -- print mount file system type */
	prt_iob,	/* IOB -- print contents of I/O buffer */
	prt_hhx,	/* HHX -- print value in hexadecimal (half size) */
	prt_wop,	/* WOP -- print waitsys() options */
	prt_spm,	/* SPM -- print sigprocmask argument */
	prt_rlk,	/* RLK -- print readlink buffer */
	prt_mpr,	/* MPR -- print mmap()/mprotect() flags */
	prt_mty,	/* MTY -- print mmap() mapping type flags */
	prt_mcf,	/* MCF -- print memcntl() function */
	prt_mc4,	/* MC4 -- print memcntl() (fourth) argument */
	prt_mc5,	/* MC5 -- print memcntl() (fifth) argument */
	prt_mad,	/* MAD -- print madvise() argument */
	prt_ulm,	/* ULM -- print ulimit() argument */
	prt_rlm,	/* RLM -- print get/setrlimit() argument */
	prt_cnf,	/* CNF -- print sysconfig() argument */
	prt_inf,	/* INF -- print systeminfo() argument */
	prt_ptc,	/* PTC -- print pathconf/fpathconf() argument */
	prt_fui,	/* FUI -- print fusers() input argument */
	prt_dec,	/* HID -- hidden argument, not normally called */
	prt_cxen,	/* CXEN -- print cxenix code */
};
