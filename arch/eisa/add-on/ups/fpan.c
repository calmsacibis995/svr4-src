/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (C) Ing. C. Olivetti & C. S.p.a., 1989.*/
/*	  All Rights Reserved  	*/

#ident	"@(#)eisa:add-on/ups/fpan.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "sys/xdebug.h"
#include "sys/cmn_err.h"

#include "fpan.h"


/* * * * * * * * I/O port address * * * * * * * * * * * * * * * * * * * */
#define	IO_BUF		0xCF8	/* receive/transmit buffer register 	*/
#define	INT_EN		0xCF9	/* interrupt enable register		*/
#define	LINE_CTR	0xCFB	/* line control register		*/
#define	LINE_STA	0xCFD	/* line status register			*/


/* * * * * * * * line status register & other masks * * * * * * * * * * */
#define TREG_EMPTY	0x20	/* transmit register empty mask		*/
#define	DATA_READY	0x01	/* data ready mask			*/
#define	ERR_MASK	0x0E	/* transmission error mask		*/
#define GET_NAK		0x80	/* tests NAK in rec. buffer 		*/
#define IS_DATA		0x80	/* set the 7 bit to say data type	*/


/* * * * * * * * front panel commands * * * * * * * * * * * * * * * * * */
#define P_DSPRST	0x45	/* reset the display after a user write	*/
#define P_STATUS	0x46	/* read system status register		*/
#define	P_PWOFF		0x4D	/* switch the power off			*/
#define P_BEGMESS	0x50	/* set begin message to display		*/
#define P_ENDMESS	0x51	/* set end message to display		*/
#define P_SHTPRG	0x52	/* set shutdown in progress bit		*/
#define P_SHTCMP	0x53	/* set shutdown complete		*/
#define P_UPSSUP	0x54	/* system support for UPS		*/


/* * * * * * * * initialization value * * * * * * * * * * * * * * * * * */
#define SET_DLAB	0x80	/* set Divisor Latch Access Bit		*/
#define	LDIV_REG	0x08	/* divisor latch register corresponding */
#define	HDIV_REG	0x00	/* to 14.4 Kbit/sec			*/
#define	TRPAR		0x1B	/* transmission param. & reset DLAB:	*/
				/* 1 stop bit, 8 data bits, even parity	*/

#define TLIMIT		10000	/* loop timeout				*/

#ifdef	OLD_STYLE
extern int (*offsys)();
#endif

static struct module_info minfo = { 
	1964, "fpan", 0, INFPSZ, 150, 50
};

int	fpanopen(), fpanclose(), fpanwput(), fpaninit();

static struct qinit rinit = { 
	NULL, NULL, fpanopen, fpanclose, NULL, &minfo, NULL 
}; /* read QUEUE */

static struct qinit winit = {
	fpanwput, NULL, NULL, NULL, NULL, &minfo, NULL
}; /* write QUEUE */

struct streamtab fpaninfo = { &rinit, &winit, NULL, NULL };

int	dispdim;	/* dim. in char of display */


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	send a command to the console
 * 
 */
static int send_cmd(cmd)
unsigned	char	cmd;
{
	unsigned char	status;
	int	i;

	#ifdef FPANDEBUG
		printf("BEGIN send_cmd(0x%x)\n", cmd);
	#endif

	/*
	 * wait until the console has received the previous command, if any.
	 */
	for (i = TLIMIT; i > 0; i--)
		if ((status = inb(LINE_STA)) & TREG_EMPTY)
			break;
	if (i == 0) 
	{
		cmn_err(CE_WARN, "front-panel: unexpected timeout in send cmd.\n");
		return(-1);
	}
	if (status & ERR_MASK) 
	{
		cmn_err(CE_WARN,
		     "front-panel: transmission error detected in send cmd.\n");

		#ifdef FPANDEBUG
			printf("END send_cmd(), i=%d, previous status read in LINE_STA=0x%X\n", TLIMIT - i, status);
		#endif

		return(-1);
	}

	/*
 	 * send the command to the console
	 */
	outb(IO_BUF, cmd );

	#ifdef FPANDEBUG
		printf("END send_cmd(), i=%d, previous status read in LINE_STA=0x%X\n", TLIMIT - i, status);
	#endif

	return(0);
} /* send_cmd */



/* 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	read data from I/O register 
 * 
 */
static int get_data(data)
unsigned char	*data;
{
	int		i;
	unsigned char	lsr_status;

	#ifdef FPANDEBUG
		printf("BEGIN get_data()\n");
	#endif

	for (i = TLIMIT; i > 0; i--)	
		if ((lsr_status = inb(LINE_STA)) & DATA_READY)
			break;
	if (i == 0) 
	{
		cmn_err(CE_WARN, "front-panel: unexpected timeout in get data.\n");
		return(-1);
	}
	if (lsr_status & ERR_MASK) 
	{
		cmn_err(CE_WARN, "front-panel: transmission error detected in get data.\n");

		#ifdef FPANDEBUG
			printf("END get_data(): i=%d, status read in LINE_STA=0x%X\n", TLIMIT - i, lsr_status);
		#endif

		return(-1);
	}

	#ifdef FPANDEBUG
		printf("get_data(): i=%d, status read in LINE_STA=0x%X\n",
			 TLIMIT - i, lsr_status);
	#endif

	if ((*data = inb(IO_BUF)) & GET_NAK)
	{
		cmn_err(CE_WARN, "front-panel: received NAK.\n");

		#ifdef FPANDEBUG
			printf("END get_data(): i=%d, data read in IO_BUF=0x%X\n", TLIMIT - i, *data);
		#endif

		return(-1);
	}

	#ifdef FPANDEBUG
		printf("END get_data(), data read in IO_BUF=0x%X, i=%d\n",
			 *data, TLIMIT - i);
	#endif

	return(0);	/* OK */
} /* get_data */



/* 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	driver init 
 * 
 */
fpaninit()
{
	unsigned char	upsstatus;
#ifdef	OLD_STYLE
	int		fpanoff();

	/*
 	 * set the pointer to function, initialized to NULL, to point
	 * to fpanoff() (switch off the machine).
	 */
	offsys = fpanoff;
#endif

	#ifdef FPANDEBUG
		printf("BEGIN fpaninit()\n");
	#endif

	/* 
	 * initialization sequence: 
	 * set divisor latch bit, set baud rate, others line parameters
	 * and reset latch bit
	 * WARNING: this initialization is automatically made by last
	 * versions of rom.
	 */
	outb(LINE_CTR, SET_DLAB);
	outb(IO_BUF, LDIV_REG);	
	outb(INT_EN, HDIV_REG);
	outb(LINE_CTR, TRPAR);

	outb(IO_BUF, P_STATUS);

	if(get_data(&upsstatus) >= 0) 
	{
		dispdim = ((upsstatus & DISPLAY_TYPE) ? 24 : 16);
#ifdef	OLD_STYLE
		printcfg("fpanel", 0x0CF8, 0x07, -1, -1, "chars=%d ups=%c",
			 dispdim,
			 (upsstatus & UPS_PRESENT) ? 'y' : 'n'   );
#endif
	}

	#ifdef FPANDEBUG
		printf("END fpaninit()\n");
	#endif

} /* fpaninit */



/* 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	driver open 
 * 
 */
fpanopen(q, dev, flag, sflag)
queue_t *q;	/* read queue */
int dev, flag, sflag;
{
	#ifdef FPANDEBUG
		printf("BEGIN fpanopen(0x%x, 0x%x, 0x%x, 0x%x)\n",
			q, dev, flag, sflag);
	#endif

	dev = minor(dev);
/*	if (!suser() || sflag || dev)	/* must be root, normal open and */
	if (sflag || dev)	/* must be root, normal open and */
		return(OPENFAIL);	/* minor number = 0 */

	#ifdef FPANDEBUG
		printf("END fpanopen()\n");
	#endif

	return(dev);
} /* fpanopen */



/* 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	driver write on front panel 
 * 
 */
fpanwput(q, mp)
queue_t		*q;
register mblk_t	*mp;
{
	struct iocblk	*iocp;
	unsigned char	upsstatus, data;
	mblk_t		*nextp;
	unsigned char	testcmd;
	char		*buff;
	int		rc, i, len;

	#ifdef FPANDEBUG
		printf("BEGIN fpanwput( 0x%x, 0x%x)\n", q, mp);
	#endif

	rc = 0;

	switch(mp->b_datap->db_type)
	{
	    case M_DATA:
		
		buff = (char *)mp->b_rptr;
		len = (((mp->b_wptr - mp->b_rptr) < dispdim) ? (mp->b_wptr - mp->b_rptr) : dispdim);
		buff[len] = '\0';
		mp->b_rptr = mp->b_wptr;

		#ifdef FPANDEBUG
			printf("fpanwput: M_DATA received buff %s\n", buff);
		#endif

		if(send_cmd(P_BEGMESS) < 0)
			goto notgood;
		if((rc = get_data(&data)) < 0)
		{

			#ifdef FPANDEBUG
				printf("fpanwput: M_DATA get_data(): rc = %d\n",
					 rc);
			#endif

			goto notgood;
		}

		#ifdef FPANDEBUG
			printf("fpanwput: M_DATA get_data(): rc = %d\n", rc);
		#endif

		for(i = 0; i < len; i++)
		{
			if(send_cmd(buff[i] | IS_DATA) < 0)
			{

			#ifdef FPANDEBUG
				printf("fpanwput: M_DATA received NAK on %d loop\n", i);
			#endif

				goto notgood;
			}
		}

		#ifdef FPANDEBUG
			printf("fpanwput: M_DATA sent %d char\n", i);
		#endif

		if(send_cmd(P_ENDMESS) < 0)
			goto notgood;
		if((rc = get_data(&data)) < 0)
		{

			#ifdef FPANDEBUG
				printf("fpanwput: M_DATA get_data(): rc = %d\n", rc);
			#endif

			goto notgood;
		}

		#ifdef FPANDEBUG
			printf("fpanwput: M_DATA get_data(): rc = %d\n", rc);
		#endif

		freemsg(mp);
		break;

	    case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			flushq(q, 0);
		if (*mp->b_rptr & FLUSHR)
		{
			flushq(RD(q), 0);
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
		}
		else
			freemsg(mp);
		break;

	    case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr; /* first byte */
		switch(iocp->ioc_cmd)
		{
		    case FPAN_STATUS:
			if(send_cmd(P_STATUS) < 0)
				goto iocnak;
			if((rc = get_data(&upsstatus)) < 0)
				goto iocnak;

			#ifdef FPANDEBUG
				printf("\tfpanwput(): status = 0x%x, rc = %d\n",
					 upsstatus, rc);
			#endif

			/*
			 * write the result in the next message block
			 */
			ASSERT(mp->b_cont);	/* test if not null	*/
			nextp = mp->b_cont;

			/* reset the read and write data pointers */
			nextp->b_rptr = nextp->b_wptr = nextp->b_datap->db_base;
			*nextp->b_wptr++ = upsstatus;
			iocp->ioc_count = 1;	/* # of byte to user	*/

			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			break;

		    case FPAN_DSPRST:
			/* reset the display after a user write; cmd #45 */

			if(send_cmd(P_DSPRST) < 0)
				goto iocnak;
			if(get_data(&data) < 0)
				goto iocnak;

			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			break;

		    case FPAN_TESTo:
			/* sends the received command to the console */

			/*
			 * read the command in the next message block
			 */
			ASSERT(mp->b_cont);	/* test if not null	*/
			testcmd = *mp->b_cont->b_rptr++;

			printf("fpanwput: TESTo received command 0x%X\n", testcmd);

			if(send_cmd(testcmd) < 0)
				goto iocnak;
			if((rc = get_data(&data)) < 0)
			{
				printf("fpanwput: TESTo get_data(): rc = %d\n", rc);
				goto iocnak;
			}
			printf("fpanwput: TESTo get_data(): rc = %d\n", rc);

			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			break;

		    case FPAN_SHTCMP:
			/* shutdown complete, cmd #53 */

			if(send_cmd(P_SHTCMP) < 0)
				goto iocnak;
			if(get_data(&data) < 0)
				goto iocnak;

			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			break;

		    case FPAN_SHTPRG:
			/* shutdown in progress, cmd #52 */

			if(send_cmd(P_SHTPRG) < 0)
				goto iocnak;
			if(get_data(&data) < 0)
				goto iocnak;

			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			break;

		    case FPAN_UPSSUP:
			/* system support for UPS, cmd #54 */

			if(send_cmd(P_UPSSUP) < 0)
				goto iocnak;
			if(get_data(&data) < 0)
				goto iocnak;

			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			break;

		    case FPAN_PWOFF:
			if(send_cmd(P_PWOFF) < 0)
				goto iocnak;
			if(get_data(&data) < 0)
				goto iocnak;
			freemsg(mp);
			goto iocnak;
			break;
		
		    default:
		    iocnak:
			#ifdef FPANDEBUG
				printf("fpanwput: IOCNAK case\n");
			#endif

			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			break;
		} 
		break;	/* M_IOCTL case */

	    default:
	    notgood:
		freemsg(mp);
		break;
	}

	#ifdef FPANDEBUG
		printf("END fpanwput()\n");
	#endif
} /* fpanwput */



/* 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	driver close 
 * 
 */
fpanclose()
{
}



#ifdef	OLD_STYLE
/* 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	switch off the machine. 
 * 	called at the end of haltsys -d procedure.
 *
 */
int fpanoff(type)
int	type;
{
	unsigned char	upsstatus;
	int		rc;

	sleep(2);

	if ( rc = send_cmd(P_STATUS) < 0)
	{
		cmn_err(CE_WARN, "front-panel: error in send command\n");
		return(-1);
	}
	if(get_data(&upsstatus) < 0) 
	{
		cmn_err(CE_WARN, "front-panel: error in get data\n");
		return(-1);
	}
	if ((upsstatus & UPS_PRESENT) && (type == 1))
	{
		/* called from powerfail */
		send_cmd(P_UPSSUP);
		send_cmd(P_SHTCMP);
	}
	else
		send_cmd(P_PWOFF);

	return(-1);
}
#endif
