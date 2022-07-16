/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs.h:bkrs.h	1.4"

/* What type of method is being performed */
#define	BACKUP_T	1
#define	RESTORE_T	2

/* State values for brstate */
#define	BR_PROCEED	1
#define	BR_SUSPEND	2
#define	BR_CANCEL	3

/* Flag values for brhistory() */
#define	BR_ARCHIVE_TOC	0x1	/* an online TOC should be archived */
#define	BR_IS_OLD_ENTRY	0x2	/* this is a new history entry */
#define	BR_IS_TMNAMES	0x4	/* the volumes are TOC volumes */

/* Flag values for rsgethdr() */
#define BR_PROMPT_ALLWD	0x1	/* rsgethdr prompt allowed */
#define BR_LABEL_CHECK	0x2	/* be sure requested vol is what is up */

/* error values for methods */
#define	BRSUCCESS	0

#define	BRERRBASE	-500
#define	BRBADARGS	(BRERRBASE - 1)
#define	BRNOTALLOWED	(BRERRBASE - 2)
#define	BRFAULT	(BRERRBASE - 3)
#define	BRTOOBIG	(BRERRBASE - 4)
#define	BRFATAL	(BRERRBASE - 5)
#define	BRNOTINITIALIZED	(BRERRBASE - 6)
#define	BRCANCELED	(BRERRBASE - 7)
#define	BRFAILED	(BRERRBASE - 8)
#define	BRSUSPENDED	(BRERRBASE - 9)
#define BRBADOPTS	(BRERRBASE - 10)
#define BRBADCMDS	(BRERRBASE - 11)
#define BRBADFSCK	(BRERRBASE - 12)
#define BRBADEXCEPT	(BRERRBASE - 13)
#define BRBADFIND	(BRERRBASE - 14)
#define BRBADTOC	(BRERRBASE - 15)
#define BRBADCPIO	(BRERRBASE - 16)

#define BRUNSUCCESS	(BRERRBASE - 17)
