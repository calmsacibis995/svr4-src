/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/logpacket.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)logpacket.c	3.14	LCC);	/* Modified: 15:27:27 11/17/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#ifndef NOLOG
#include	"pci_types.h"
#include	<fcntl.h>
#include	<errno.h>
#include	<memory.h>

#ifdef	RS232PCI
#include	<termio.h>
#endif	/* RS232PCI */

#ifdef	ETHLOCUS
#include	<sys/eth.h>		/* LOCUS Ethernet structs/constants */
#endif	/* ETHLOCUS */

#ifdef	ETH3BNET
#include	<sys/ni.h>		/* AT&T Ethernet structs/constants */
#endif	/* ETH3BNET */

#include	"flip.h"
#include	"const.h"

static char
	ethHdrFmt[] = "net: dst: %s; src: %s; type: %2x.%2x\n",
	udpHdrFmt[] = "net: dst: %s; src: %s;\n",
	rs232HdrFmt[] = "net: chks %u; f_cnt: %d\n",
	nhHdrFmt[] = "net: head: %s\n",
	preHdrFmt[] = "pre: select %d; reset %d\n",
	hdr1Fmt[] = "hdr: res %d; req %d; stat %d; seq %d; drvNum %d; mode: %d\n",
	hdr2Fmt[] = "...: t_cnt %d; b_cnt %d; inode %d; date %d; time %d; pid %d\n",
	hdr3Fmt[] = "...: fdsc %d; f_size %ld; offset %ld; pattern %#lx; attr %#x\n";

static char *reqname();
static char *resname();

extern char *nAddrFmt();		/* Format net address */

/*
   logPacket: log select portions of a packet contents to a file
*/

logPacket(pk, kind, how)
register struct input   *pk;
int                     kind;
long                    how;
{
    register int rq;

    /* Quit if packet logs not enabled or is a remote log packet */
    if ( (!dbgCheck(DBG_PLOG)) || (pk->hdr.req == LOG_MSG) || (logFile == NULL))
	return;

    rq = pk->hdr.req;

    if (kind == PLOG_RCV)
	fprintf(logFile,
	    "\nGot request=%d\t\t\t-----------------~%d~%s ----\n",
	    rq, rq, reqname(rq));

    /* only log more of read/write/seek packets when DBG_XPLOG is on */
    if ((rq != READ_SEQ && rq != WRITE_SEQ && rq != L_SEEK)
#ifdef SEE_RW
	|| dbgCheck(DBG_XPLOG)
#endif
	)
    {
	if (how & LOGNETHEADER)
	{
#ifdef  ETHNETPCI
#ifndef BERKELEY42
	    fprintf(logFile, ethHdrFmt, nAddrFmt(pk->net.dst),
		nAddrFmt(pk->net.src), pk->net.type[0], pk->net.type[1]);
#else   /* BERKELEY42 */
	    fprintf(logFile, udpHdrFmt, nAddrFmt(pk->net.dst),
		nAddrFmt(pk->net.src));
#endif  /* BERKELEY42 */
#endif  /* ETHNETPCI */
#ifdef  RS232PCI
	    fprintf(logFile, rs232HdrFmt, pk->rs232.chks, pk->rs232.f_cnt);
#endif  /* RS232PCI */
	}

#if     defined(ETH3BNET) || defined(ETHLOCUS)
	if (how & LOGNHHEAD)
	    fprintf(logFile, nhHdrFmt, nAddrFmt(pk->net.head));

#endif  /* ETHNETPCI || ETHLOCUS */
	if (how & LOGPREHEADER)
	    fprintf(logFile, preHdrFmt, pk->pre.select, pk->pre.reset);

	if (how & LOGHEADER)
	{
#define bhdr    pk->hdr
	    fprintf(logFile, hdr1Fmt, bhdr.res, bhdr.req, bhdr.stat,
		    bhdr.seq, bhdr.drvNum, bhdr.mode);
	    fprintf(logFile, hdr2Fmt, bhdr.t_cnt, bhdr.b_cnt, bhdr.inode,
		    bhdr.date, bhdr.time, bhdr.pid);
	    fprintf(logFile, hdr3Fmt, bhdr.fdsc, bhdr.f_size,
		    bhdr.offset, bhdr.pattern, bhdr.attr);
#undef  bhdr
	}

	/* Log the `text' of the packet */
#ifdef OLDTEXTLOG
	if (how & LOGASCII)
	{
	    fprintf(logFile, "txt(ASCII): ");
	    fwrite(pk->text, pk->hdr.t_cnt, 1, logFile);
	    fprintf(logFile, "\nEnd Packet Text\n");
	}
#else /* ~OLDTEXTLOG */
	if ((how & LOGASCII) && (pk->hdr.t_cnt > 0))
	{
	    unsigned short ii, jj;
	    char *ap;

/* KLO0168 - begin */
#ifdef	IX370
/* only log special packets with DBG_YPLOG, but never log packets
** with passwds
*/
#define	txtlogchk(x)	((dbgCheck(DBG_YPLOG) && (x != EST_BRIDGE))
#else	/* !IX370 */
/* only log special packets with DBG_YPLOG */
#define	txtlogchk(x)	(dbgCheck(DBG_YPLOG))
#endif	/* !IX370 */
/* KLO0168 - end */

	    /* do not log text of packets that have passwords in them */
	    /* do not log text of read/write packets */
	    /* unless txtlogchk says ok */	/* KLO0168 */
/*
 *	370 compiler seems a tad bogus so we have this bizarre illegal
 *	statement here for LOCUS and IX370 defined.
 */
#if defined(LOCUS) && defined(IX370)
	    if ( txtlogchk(rq) ) ||	/* KLO0168 */
		(rq != READ_SEQ && rq != WRITE_SEQ &&	/* KLO0168 */
		 rq != READ_RAN && rq != WRITE_RAN && rq != EST_BRIDGE))
#else
	    if ( (txtlogchk(rq) != 0) ||	
		(rq != READ_SEQ && rq != WRITE_SEQ &&	
		 rq != READ_RAN && rq != WRITE_RAN && rq != EST_BRIDGE))
#endif
	    {
		fputs("ascii: ", logFile);
		for (ii=0, jj=6, ap=pk->text; ii < pk->hdr.t_cnt; ii++,ap++)
		{
		    if (jj++ > 70)
		    {
			fputs("\n", logFile);
			jj=0;
		    }
		    if (*ap < ' ' || *ap >= 0x7f)
		    {
			fprintf(logFile, "<%02x>", *ap & 0x00ff);
			jj += 3;
		    }
		    else
		    {
			fputc(*ap, logFile);
		    }
		}
		fputs("\n", logFile);
	    }
	}
#endif /* ~OLDTEXTLOG */

#ifdef  notYet
	/* Alternate presentations of packet text */
	if (how & LOGDBYTES)
	{
	}

	if (how & LOGHBYTES)
	{
	}

	if (how & LOGDSHORTS)
	{
	}

	if (how & LOGHSHORTS)
	{
	}

	if (how & LOGDLONGS)
	{
	}

	if (how & LOGHLONGS)
	{
	}
#endif  /* notYet */
    }

    if (kind != PLOG_RCV)
	fprintf(logFile,
	    "\nSending >>\t\t\t>>>>%s Response %d %s <<<<\n",
	    reqname(rq), pk->hdr.res, resname(pk->hdr.res));
    fflush(logFile);
}

/*****************************************************************************
 * reqname ()
 * return ascii name of Valid Event/Actions
 */	
static char *reqname(rq)
    int rq;
{
    char *rr;

    switch (rq)
    {
    case EXIT :         rr = "EXIT";            break;
    case PCI_CREATE :   rr = "CREATE";          break;
    case PCI_DELETE :   rr = "DELETE";          break;
    case OLD_OPEN :     rr = "OLD_OPEN";        break;
    case NEW_CLOSE :    rr = "NEW_CLOSE";       break;
    case READ_SEQ:      rr = "READ_SEQ";        break;
    case READ_RAN:      rr = "READ_RAN";        break;
    case WRITE_SEQ:     rr = "WRITE_SEQ";       break;
    case WRITE_RAN:     rr = "WRITE_RAN";       break;
    case CHDIR :        rr = "CHDIR";           break;
    case MKDIR :        rr = "MKDIR";           break;
    case RMDIR :        rr = "RMDIR";           break;
    case CHMOD :        rr = "CHMOD";           break;
    case RENAME :       rr = "RENAME_OLD";      break;
    case RENAME_NEW :   rr = "RENAME_NEW";      break;
    case SEARCH :       rr = "SEARCH";          break;
    case NEXT_SEARCH :  rr = "NEXT_SEARCH";     break;
    case TIME_DATE :    rr = "TIME_DATE";       break;
    case NEW_OPEN :     rr = "NEW_OPEN";        break;
    case FS_STATUS :    rr = "FS_STATUS";       break;
    case SET_STATUS :   rr = "SET_STATUS";      break;
    case FILE_SIZE :    rr = "FILE_SIZE";       break;
    case L_SEEK :       rr = "L_SEEK";          break;
    case FIND_FIRST :   rr = "FIND_FIRST";      break;
    case NEXT_FIND :    rr = "NEXT_FIND";       break;
    case RS232_ALIVE:   rr = "RS232_ALIVE";     break;
    case GETWD :        rr = "GETWD";           break;
    case PROBE:         rr = "PROBE";           break;
    case EST_BRIDGE:    rr = "EST_BRIDGE";      break;
    case EST_SHELL:     rr = "EST_SHELL";       break;
    case TERM_SHELL:    rr = "TERM_SHELL";      break;
    case SEND_MAP:      rr = "SEND_MAP";        break;
    case CONNECT:       rr = "CONNECT";         break;
    case DISCONNECT:    rr = "DISCONNECT";      break;
    case OLD_CLOSE :    rr = "OLD_CLOSE";       break;
    case LOCK_MODE :    rr = "LOCK_MODE";       break;
    case SECURITY_CHECK:rr = "SECURITY_CHECK";  break;
    case PC_CRASH:      rr = "PC_CRASH";        break;
    case CONSVR_HERE:   rr = "CONSVR_HERE";     break;
    case CONSVR_BYE:    rr = "CONSVR_BYE";      break;
    case GET_INIT_DATA: rr = "GET_INIT_DATA";   break;
    case DEVICE_INFO_C: rr = "DEVICE_INFO_C";   break;
    case LOCK:          rr = "LOCK";            break;
    case GET_EXT_ERROR: rr = "GET_EXT_ERROR";   break;
    case CONSOLE_READ:  rr = "CONSOLE_READ";    break;
    case LOG_CTRL:      rr = "LOG_CTRL";        break;
    case LOG_MSG:       rr = "LOG_MSG";         break;
    case UEXEC:         rr = "UEXEC";           break;
    case UWAIT:         rr = "UWAIT";           break;
    case UKILL:         rr = "UKILL";           break;
    case MAPNAME:	rr = "MAPNAME";		break;
    case SET_SDEBUG:	rr = "SET_SDEBUG";	break;
    case IPC:		rr = "IPC";		break;
    default: rr = "???"; break;
    }
return rr;
}

/*****************************************************************************
 * resname ()
 * return ascii name of Valid Event/Actions
 */	
static char *resname(rq)
    int rq;
{
    char *rr;

    switch (rq)
    {
    case SUCCESS:               rr = "SUCCESS";                 break;
    case INVALID_FUNCTION:      rr = "INVALID_FUNCTION";        break;
    case FILE_NOT_FOUND:        rr = "FILE_NOT_FOUND";          break;
    case PATH_NOT_FOUND:        rr = "PATH_NOT_FOUND";          break;
    case TOO_MANY_FILES:        rr = "TOO_MANY_FILES";          break;
    case ACCESS_DENIED:         rr = "ACCESS_DENIED";           break;
    case FILDES_INVALID:        rr = "FILDES_INVALID";          break;
    case MEMORY_BLOCK_DESTROYED:rr = "MEMORY_BLOCK_DESTROYED";  break;
    case INSUFFICIENT_MEMORY:   rr = "INSUFFICIENT_MEMORY";     break;
    case MEM_BLOCK_INVALID:     rr = "MEM_BLOCK_INVALID";       break;
    case ENVIRONMENT_INVALID:   rr = "ENVIRONMENT_INVALID";     break;
    case FORMAT_INVALID:        rr = "FORMAT_INVALID";          break;
    case ACCESS_CODE_INVALID:   rr = "ACCESS_CODE_INVALID";     break;
    case DATA_INVALID:          rr = "DATA_INVALID";            break;
    case DRIVE_INVALID:         rr = "DRIVE_INVALID";           break;
    case ATTEMPT_TO_REMOVE_DIR: rr = "ATTEMPT_TO_REMOVE_DIR";   break;
    case NOT_SAME_DEVICE:       rr = "NOT_SAME_DEVICE";         break;
    case NO_MORE_FILES:         rr = "NO_MORE_FILES";           break;
    case LOGIN_FAILED:          rr = "LOGIN_FAILED";            break;
    case INTERNAL_ERRORS:       rr = "INTERNAL_ERRORS";         break;
    case SYSTEM_GOING_DOWN:     rr = "SYSTEM_GOING_DOWN";       break;
    case NO_SESSION:            rr = "NO_SESSION";              break;
    case NO_PORTS:              rr = "NO_PORTS";                break;
    case OTHER:                 rr = "OTHER";                   break;
    case DUPLICATE_CONNECTION:  rr = "DUPLICATE_CONNECTION";    break;
    case INV_SHELL:             rr = "INV_SHELL";               break;
    case LOCK_VIOLATION:     	rr = "LOCK_VIOLATION";		break;
    case SHARE_VIOLATION:     	rr = "SHARE_VIOLATION";		break;
    default: rr = "???"; break;
    }
return rr;
}
#endif /* ~NOLOG */

