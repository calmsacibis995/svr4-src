#ident	"@(#)qt.h	1.5	92/06/28	JPB"
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)qt:io/qt.h	1.3.1.2"

#include <sys/param.h>
#include <sys/pit.h>
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/file.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include <sys/tape.h>

#define ACTIONSIZE	32			/* Length of action queue */
#define SPACESIZE	1536			/* Static buffer space */
#define BLOCKSIZE	512			/* Length of a block */
#define TMOPERIOD	3600			/* Period of timeout routine invocation */
#define WAITLIMIT	500000			/* Maximum number of times through the wait loop */
#define DISSIGPRI	(PZERO - 5)		/* Sleep priority for disabling signals */
#define ENASIGPRI	((PZERO + 1) | PCATCH)	/* Sleep priority for enabling signals */
#define SUCCESS		1			/* Success return code */
#define FAIL		0			/* Failure return code */
#undef NULL
#define NULL		((char *) 0)
#define TRUE		1
#define FALSE		0
#define LOW16BITS	0177777	

/* Hardware QIC-02 commands */

#define C_REWIND	0041		/* Rewind */
#define C_ERASE		0042		/* Erase */
#define	C_RETENSION	0044		/* Retension */
#define C_WRDATA	0100		/* Write data */
#define C_WRFILEM	0140		/* Write file mark */
#define C_RDDATA	0200		/* Read data */
#define C_RDFILEM	0240		/* Read file mark */
#define C_STATUS	0300		/* Get device status */

#define isexception(s)		(!((s) & STS_NEXC))
#define isnotready(s)		((s) & STS_NRDY)
#define isinterrupt(s)		(!((s) & STS_NINT))
#define isdone(s)		((s) & STS_DONE)

/* Action queue codes */
#define A_RETENSION	'\001'
#define A_REWIND	'\002'
#define A_ERASE		'\003'
#define A_WRDATA	'\004'
#define A_RDDATA	'\005'
#define A_WRFILEM	'\006'
#define A_SBF		'\007'
#define A_FLUSH		'\010'
#define A_RDFILEM	'\011'

/* Timeout click maximums */
#define TM_1B		5060	/* Stream 17 times - 51 seconds */
#define TM_1T		8100	/* Position, i.e., traverse 1 track - 85 seconds */
#define TM_EXTRA	300	/* Extra time for timeouts */

/* Minor device bit meanings */
#define MD_RET		0010	/* Retension on open */
#define MD_REW		0004	/* Rewind on close */

typedef long	ALIGN;

#define SPACEWORDS	(SPACESIZE / sizeof(ALIGN))

/* structure shared by user process, interrupt process, and timeout process */
struct iqt {
	char	action[ACTIONSIZE];	/* Current action queue */
	char	*paction;		/* Pointer to current action in queue */
	long	flags;
	char	mask;			/* Control port mask */
	caddr_t sleep;			/* Sleep address of user process */
	int	sfarg;			/* Copy of ioctl skip forward arg */
	int	error;			/* Errno from interrupt routine */
	time_t start_time;		/* Operation start time */
	struct buf *b;			/* Physio buffer for RDDATA and WRDATA */
	caddr_t	vbuffer;		/* Buffer */
	paddr_t pbuffer;		/* Physical address of internal buffer */
	caddr_t	vbufferp;		/* Buffer pointer */
	caddr_t vmoved;			/* Buffer for indirect dma moves */
	paddr_t pmoved;			/* Physical address of moved_buffer */
	paddr_t pio_addr;		/* Data yet to be transferred for {RD|WR}DATA */
	caddr_t vio_addr;		/* Virtual address of user data space */
	unsigned int	io_count;	/* Count of bytes yet to be transferred for {RD|WR}DATA */
	char	uexit;			/* User process exit interrupt status */
	caddr_t moved_block;		/* User address for indirect dma transferred block */
};

/* iqt flag meanings */
#define F_OPENED	00000001	/* Device is opened */
#define F_TMOON		00000002	/* Timeout routine is in operation */
#define F_EXCLUSIVE	00000004	/* Exclusive access enforcement flag */
#define F_PROGRESS	00000010	/* Hardware has operation in progress */
#define F_ACTIVE	00000020	/* Data buffer is active */
#define F_WRMODE	00000040	/* Data buffer is in write, else read mode */
#define F_USLEEP	00000100	/* User process is asleep */
#define F_INTERRUPT	00000200	/* User process interrupted operation */
#define F_ERROCC	00000400	/* Error has occurred in operation */
#define F_ISSRDWR	00001000	/* Command to read/write is to be issued */
#define F_DMAWAIT	00002000	/* Waiting for device buffer to fill on read */
#define F_FFMR		00004000	/* First file mark is read */
#define F_EOFR		00010000	/* End of file mark read during last read */
#define F_LOPW		00020000	/* Last operation was a write */
#define F_FFMW		00040000	/* First file mark is written */
#define F_SFMW		00100000	/* Second file mark is written */
#define F_LCMDW		00200000	/* Last command was a write */
#define F_LCMDR		00400000	/* Last command was a read */
#define F_EOM		01000000	/* End of media reached */
#define F_READ		02000000	/* Data was read from this file */

#define isset(x)	(iqt->flags & (x))
#define isclr(x)	(!(iqt->flags & (x)))
#define setflags(x)	(iqt->flags |= (x))
#define clrflags(x)	(iqt->flags &= ~(x))

/* uexit and iexit codes */
#define I_SNDCMD	1
#define I_ENACINT	2
#define I_ENADMA	3

#define isboundary(a)	(((kvtophys ((unsigned long) (a)) & LOW16BITS) + (BLOCKSIZE - 1)) & ~LOW16BITS)
#define first3rd()	(space)
#define second3rd()	(&space[SPACEWORDS / 3])
#define third3rd()	(&space[2 * SPACEWORDS / 3])
#define	splon()		spl = spl6 ()
#define sploff()	splx (spl)
#define w1sec()		(timeout (wakeup, (caddr_t) qt_reset, HZ), sleep ((caddr_t) qt_reset, DISSIGPRI))
#define topq()		(*iqt->paction)
#define zeroq()		(*(iqt->paction = iqt->action) = '\0')
#define dq()		(++iqt->paction)
#define firstq()	(*iqt->action)

#define assert(x)	outb (CTLPORT, iqt->mask |= (x))
#define deassert(x)	outb (CTLPORT, iqt->mask &= ~(x))

#define	TDELTA25	20
#define	TDELTA20	15

#define waitloop(m)					\
{							\
	int i;						\
	for (i=0; i < m; i++ )				\
		tenmicrosec();				\
}

#define kill_check(t)					\
if (elapsed > (t))					\
	kill_it = TRUE

#define	excl_check() 					\
{							\
	splon();					\
	if (isset(F_EXCLUSIVE)) {			\
		sploff();				\
		u.u_error = EBUSY;			\
		return;					\
	}						\
	setflags(F_EXCLUSIVE);				\
	sploff();					\
}

#define seq_check()					\
if (isset(F_LOPW)) {					\
	u.u_error = EINVAL;				\
	return;						\
}

#define flush_check() 					\
if (isset(F_ACTIVE) && isset(F_WRMODE))			\
	nq(A_FLUSH)					\
else							\
	clrflags(F_ACTIVE)

#define ffmw_check()					\
if (isclr(F_FFMW))					\
	nq(A_WRFILEM)

#define sfmw_check()					\
if (isclr(F_SFMW))					\
	nq(A_WRFILEM)

#define ffmr_check()					\
if (isclr(F_FFMR | F_EOFR)) {				\
	nq(A_RDFILEM)					\
	iqt->sfarg = 1;					\
}

#define execq()					\
while (isclr(F_PROGRESS) && topq())		\
	if (qt_exec ()) {			\
		if (isclr(F_PROGRESS))		\
			dq();			\
	} else {				\
		error_proc (EIO);		\
		break;				\
	}

#define nq(c)						\
{							\
	register char	*p;				\
							\
	for (p = iqt->action; *p; p++)			\
		;					\
	if (p <= iqt->action + ACTIONSIZE - 2) {	\
		*p++ = c;				\
		*p = '\0';				\
	}						\
}

#define iseom(s)	(s[0] & 010)
#define isreadfm(s)	((s[0] & 0357) == 0201 && !s[1])
/* #define ispor(s)	(!(s[0] & 0140) && s[1] & 01) */
#define ispor(s)	(!(s[0] & 0040) && s[1] & 01)

extern char	fubyte (), inb ();
extern void	wakeup (caddr_t);
extern int	qt_chan, qt_0_vect, qt_0_sioa;
extern time_t lbolt;
extern struct user u;
