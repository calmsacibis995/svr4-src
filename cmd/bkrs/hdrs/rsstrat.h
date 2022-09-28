/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/rsstrat.h	1.2.2.1"

/*
	This file contains information about the restore Turing Machine mechanism
	and the restore strategy file
*/

/* Field Names */
#define	RSS_TYPE	(unsigned char *)"type"
#define	RSS_STATE	(unsigned char *)"state"
#define	RSS_STIMULUS	(unsigned char *)"stimulus"
#define	RSS_NEXTSTATE	(unsigned char *)"nextstate"
#define	RSS_STOPAT	(unsigned char *)"stopat"

/* Valid stimulae */
#define	RSS_BEGIN	(unsigned char *)"begin"
#define	RSS_END	(unsigned char *)"end"
#define	RSS_ARCHIVEF	(unsigned char *)"archive.f"
#define	RSS_ARCHIVEP	(unsigned char *)"archive.p"
#define	RSS_RSDATE	(unsigned char *)"rsdate"
#define	RSS_OPT_ARCHIVEF	(unsigned char *)"archive.f?"
#define	RSS_OPT_ARCHIVEP	(unsigned char *)"archive.p?"

/* Valid stopats */
#define	RSS_ONE	(unsigned char *)"one"
#define	RSS_ALL	(unsigned char *)"all"

/* Entry Format */
#define RSS_ENTRY_F (unsigned char *)"type:state:stimulus:nextstate:stopat"
