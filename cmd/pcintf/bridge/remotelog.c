/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/remotelog.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)remotelog.c	3.6	LCC);	/* Modified: 16:55:14 7/13/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#ifndef NOLOG
#include	<ctype.h>
#include	<pci_types.h>

/*
   remotelog.c:  Log messages from workstation
*/


#define	MAXPRFARGS	32		/* Max # of printf arguments */

/*
   logControl:  Turn remote logging on and off
*/

extern char
	*dosCName(),
	*dosEName();

logControl(rmtLogFlag)
int
	rmtLogFlag;			/* TRUE ==> logs on; FALSE ==> off */
{
	if (rmtLogFlag) {
		dbgOn(DBG_RLOG);
		if (logFile == NULL)
			logOpen(DOSSVR_LOG, getpid());
	}
	else
		dbgOff(DBG_RLOG);
}


/*
   logMessage:  Log a message from the workstation
*/

void
logMessage(msgBuf, nBytes)
unsigned char
	*msgBuf;			/* Encoded printf message buffer */
int
	nBytes;				/* Number of bytes of text in message */
{
unsigned int
	prfArgs[MAXPRFARGS];		/* Printf argument vector */
int
	nArgs,				/* Number of printf arguments */
	longFlag;			/* Numeric argument is long */
char
	*dosMesg;			/* DOS error message/call name */
unsigned char
	*bufEnd;			/* First address not in msgBuf */
register unsigned char
	*argScan,			/* Scan arguments in msgBuf */
	*fmtScan;			/* Scan format string */
register unsigned int
	*argFill = &prfArgs[0];		/* Fill the argument vector */
unsigned long
	buildArg;			/* Construct arguments here */

	/* If remote logs aren't enabled, return immediately */
	if (!dbgCheck(DBG_RLOG) || logFile == NULL)
		return;

	/* Compute first argument address and message buffer end */
	bufEnd = &msgBuf[nBytes];
	argScan = &msgBuf[strlen(msgBuf) + 1];
	/* All arguments are aligned on even addresses within message text */
	if ((int)argScan & 1)
		argScan++;

	/*
	   Scan format string and convert it to a real printf format
	   string and accumulate arguments from message buffer into
	   argument vector
	*/
	for (fmtScan = msgBuf; fmtScan < bufEnd && *fmtScan != '\0'; fmtScan++)
	{
		/* We only care about format specs */
		if (*fmtScan != '%')
			continue;

		/* Skip format introducer (`%') */
		fmtScan++;

		/* If base indicator flag is present, skip it */
		if (*fmtScan == '#')
			fmtScan++;

		while (isdigit(*fmtScan) || *fmtScan == '.')
			fmtScan++;

		/* Long integer? */
		if (*fmtScan == 'l') {
			longFlag = 1;
			fmtScan++;
		} else
			longFlag = 0;

		switch (*fmtScan) {
		case '\0':			/* End of format string */
			break;

		case 'd':			/* Decimal number */
		case 'u':			/* Unsigned decimal */
		case 'x':			/* Hex */
		case 'o':			/* Octal */
			/*
			   Construct argument for printf and save it in
			   prfArgs.  iAPx processor byte order and 16 bit
			   ints are assumed in log request message.
			*/
			buildArg = *argScan++;
			buildArg |= (*argScan++ << 8);
			if (longFlag) {
				buildArg |= (unsigned long)*argScan++ << 16;
				buildArg |= (unsigned long)*argScan++ << 24;
			}

			/* Put integer argument into argument vector. */
			if (longFlag && sizeof (int) != sizeof (long)) {
#if	LSW == 0
				*argFill++ = (int)buildArg;
				*argFill++ = (int)(buildArg >> 16);
#else
				*argFill++ = (int)(buildArg >> 16);
				*argFill++ = (int)buildArg;
#endif	/* LSW != 0 */
			} else
				*argFill++ = (int)buildArg;
			break;

		case 's':			/* String */
			if (sizeof (char *) == sizeof (int))
				*argFill++ = (int)argScan;
			else {
#if	LSW == 0
				*argFill++ = (int)argScan;
				*argFill++ = (int)((long)argScan >> 16);
#else
				*argFill++ = (int)((long)argScan >> 16);
				*argFill++ = (int)argScan;
#endif	/* LSW != 0 */
			}

			/* Move argScan up to next argument */
			argScan += strlen(argScan) + 1;
			/* Arguments are word aligned */
			if ((int)argScan & 1)
				argScan++;
			break;

		case 'e':			/* DOS error code */
		case 'y':			/* DOS call number */
			buildArg = *argScan++;
			buildArg |= (*argScan++ << 8);
			if (*fmtScan == 'e')
				dosMesg = dosEName(buildArg);
			else
				dosMesg = dosCName((buildArg >> 8) & 0xff);

			/* Change the format specifier for the real printf */
			*fmtScan = 's';

			/* Put address of message buffer in argument vector */
			if (sizeof (char *) == sizeof (int))
				*argFill++ = (int)dosMesg;
			else {
#if	LSW == 0
				*argFill++ = (int)dosMesg;
				*argFill++ = (int)((long)dosMesg >> 16);
#else
				*argFill++ = (int)((long)dosMesg >> 16);
				*argFill++ = (int)dosMesg;
#endif	/* LSW != 0 */
			}

		case 'c':			/* Character */
			*argFill++ = *argScan++;
			/* Arguments are all on even boundaries */
			argScan++;
			break;
		}

		/* Quit at end of format string or when arguments are used up */
		if (*fmtScan == '\0' || argScan > bufEnd)
			break;
	}

	/* Log the message */
	logv(msgBuf, prfArgs);
}
#endif /* ~NOLOG */
