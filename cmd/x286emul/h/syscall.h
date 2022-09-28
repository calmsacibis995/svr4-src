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

#ident	"@(#)x286emul:h/syscall.h	1.1"

/* @(#)syscall.h	1.2 */

/* pre-2.3 trap numbers that differ from 2.3 */

#define	OTFTIME		35
#define	OTCLOCAL		62
#define	OTCXENIX		63

/* 3.0 trap numbers */

#define	TEXIT		1
#define	TFORK		2
#define	TREAD		3
#define	TWRITE		4
#define	TOPEN		5
#define	TCLOSE		6
#define	TWAIT		7
#define	TCREAT		8
#define	TLINK		9
#define	TUNLINK		10
#define	TEXEC		11
#define	TCHDIR		12
#define	TTIME		13
#define	TMKNOD		14
#define	TCHMOD		15
#define	TCHOWN		16
#define	TBREAK		17
#define	TSTAT		18
#define	TLSEEK		19
#define	TGETPID		20
#define	TMOUNT		21
#define	TUMOUNT		22
#define	TSETUID		23
#define	TGETUID		24
#define	TSTIME		25
#define	TPTRACE		26
#define	TALARM		27
#define	TFSTAT		28
#define	TPAUSE		29
#define	TUTIME		30
#define	TSTTY		31
#define	TGTTY		32
#define	TACCESS		33
#define	TNICE		34
/*		35*/
#define	TSYNC		36
#define	TKILL		37

#define	TCLOCAL		38

#define	TSETPGRP	39

#define	TCXENIX		40
#define		TSHUTDN		0
#define		TLOCKING	1
#define		TCREATSEM	2
#define		TOPENSEM	3
#define		TSIGSEM		4
#define		TWAITSEM	5
#define		TNBWAITSEM	6
#define		TRDCHK		7
#define		TXTRACE		9
#define		TCHSIZE		10
#define		TFTIME		11
#define		TNAP		12
#define		TSDGET		13
#define		TSDFREE		14
#define		TSDENTER	15
#define		TSDLEAVE	16
#define		TSDGETV		17
#define		TSDWAITV	18
#define		TBRKCTL		19
#define		TMSGCTL		22	/* M000 */
#define		TMSGGET		23	/* M000 */
#define		TMSGSND		24	/* M000 */
#define		TMSGRCV		25	/* M000 */
#define		TSEMCTL		26	/* M001 */
#define		TSEMGET		27	/* M001 */
#define		TSEMOP		28	/* M001 */
#define		TSHMCTL		29	/* M002 */
#define		TSHMGET		30	/* M002 */
#define		TSHMAT		31	/* M002 */
#define		TPROCTL		32	/* M003 */

#define	TDUP		41
#define	TPIPE		42
#define	TTIMES		43
#define	TPROFIL		44
#define	TLOCK		45		/* sys5 */
#define	TSETGID		46
#define	TGETGID		47
#define	TSIGS		48
/*		49*/
/*		50*/
#define	TACCT		51
#define	TPHYS		52
/*		53*/
#define	TIOCTL		54
/*		55*/
#define	TMPX		56

#define	TPWBSYS		57
#define	TUNAME		0
#define	TUSTAT		2

/*		58*/
#define	TEXECE		59
#define	TUMASK		60
#define	TCHROOT		61
#define	TFCNTL		62
#define	TULIMIT		63
