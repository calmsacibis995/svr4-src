/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:fmtmsg.h	1.3"

/*
 * fmtmsg.h
 *
 *	The <fmtmsg.h> header file contains the definitions needed
 *	to use the fmtmsg() function.  This function writes messages
 *	in a standard format to the standard error stream (stderr)
 *	and to the system console.
 */


/*
 * Define the value "NULL" if it hasn't been defined already.
 */

#ifndef	NULL
#define	NULL	0
#endif


/*
 * Constraint definitions:
 *	MM_MXLABELLN	Maximum size of a "label" in a message
 *	MM_MXTAGLN	Maximum size of a "tag" in a message
 *	MM_MXTXTLN	Maximum size of a text string
 *	MM_MXACTLN	Maximum size of an action string
 */

#define	MM_MXLABELLN	25
#define	MM_MXTAGLN	32
#define MM_MXTXTLN	512
#define MM_MXACTLN	512


/*
 * Environment variable names used by fmtmsg():
 *	MSGVERB		Tells fmtmsg() which components it is to write
 *			to the standard error stream
 */

#define	MSGVERB		"MSGVERB"

/*
 * Classification information
 *      - Definition of classifications
 *	- Definition of recoverability
 *	- Definition of source classifications
 */

/*
 * Definition of the "null" classification
 *	MM_NULL		Indicates that the classification has been omitted
 */

#define	MM_NULL		0L


/*
 * Definitions of type classifications:
 *	MM_HARD		Hardware
 *	MM_SOFT		Software
 *	MM_FIRM		Firmware
 */

#define	MM_HARD		0x00000001L
#define	MM_SOFT		0x00000002L
#define	MM_FIRM		0x00000004L


/*
 * Definitions of recoverability subclassification
 *	MM_RECOVER	Recoverable
 *	MM_NRECOV	Non-recoverable
 */

#define	MM_RECOVER	0x00000100L
#define	MM_NRECOV	0x00000200L


/*
 * Definitions of source subclassification
 *	MM_APPL		Application
 *	MM_UTIL		Utility
 *	MM_OPSYS	Kernel
 */

#define	MM_APPL		0x00000008L
#define MM_UTIL		0x00000010L
#define	MM_OPSYS	0x00000020L


/*
 * Definitions for the action to take with the message:
 *	MM_PRINT	Write to the standard error stream
 *	MM_CONSOLE	Treat the message as a console message
 */

#define	MM_PRINT	0x00000040L
#define	MM_CONSOLE	0x00000080L

/*
 * Constants for severity values
 *
 *	SEV_LEVEL	Names the env variable that defines severities
 *
 *	MM_NOSEV	Message has no severity
 *	MM_HALT		Message describes a severe error condition
 *	MM_ERROR	Message describes an error condition
 *	MM_WARNING	Message tells of probable error condition
 *	MM_INFO		Message informs, not in error
 */

#define	SEV_LEVEL	"SEV_LEVEL"
#define MM_NOSEV	0
#define	MM_HALT		1
#define	MM_ERROR	2
#define	MM_WARNING	3
#define	MM_INFO		4

/*
 * Null values for message components
 *	MM_NULLLBL		Null value for the label-component
 *	MM_NULLSEV		Null value for the severity-component
 *	MM_NULLMC		Null value for the classification-component
 *	MM_NULLTXT		Null value for the text-component
 *	MM_NULLACT		Null value for the action-component
 *	MM_NULLTAG		Null value for the tag-component
 */

#define	MM_NULLLBL	((char *) NULL)
#define	MM_NULLSEV	MM_NOSEV
#define	MM_NULLMC	MM_NULL
#define	MM_NULLTXT	((char *) NULL)
#define	MM_NULLACT	((char *) NULL)
#define	MM_NULLTAG	((char *) NULL)

/*
 * Values returned by fmtmsg()
 *
 *	MM_NOTOK	None of the requested messages were generated
 *	MM_NOMSG	No message was written to stderr
 *	MM_NOCON	No console message was generated
 */

#define	MM_NOTOK	-1
#define	MM_OK		0x00
#define	MM_NOMSG	0x01
#define MM_NOCON	0x04


/* Function definition */

#if defined(__STDC__)
int	fmtmsg(long, const char *, int, const char *, const char *, const char *);
int	addseverity(int, const char *);

#else
int	fmtmsg();
int	addseverity();
#endif
