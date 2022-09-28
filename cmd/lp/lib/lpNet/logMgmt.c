/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/lpNet/logMgmt.c	1.3.2.1"

/*==================================================================*/
/*
*/
#include	<unistd.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<time.h>
#include	"logMgmt.h"
#include	"errorMgmt.h"
#include	"debug.h"

static	char	*(*LogMsgTagFn_p)()	= NULL;
static	FILE	*LogFile_p		= NULL;
/*==================================================================*/

/*==================================================================*/
/*
**	This function tries to open the requested logfile
**	but if this fails then it opens an alternate logfile
**	in '/tmp'  this is for those applications that do not
**	have 'stderr' open and are relying on a logfile.
**
**	We protect against recursive calls that may occur via
**	'TrapError/WriteLogMsg/OpenLogFile'
**
*/
boolean
OpenLogFile (logFilePath_p)

char	*logFilePath_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		char	alternateFile [32];
	static	char	FnName []		= "OpenLogFile";


	/*----------------------------------------------------------*/
	/*
	*/
	if (LogFile_p != NULL) {
		(void)	fclose (LogFile_p);
		LogFile_p = NULL;
	}
	if (logFilePath_p == NULL)
		goto	openAlternateFile;


	/*----------------------------------------------------------*/
	/*
	*/
	LogFile_p = fopen (logFilePath_p, "a+");

	if (LogFile_p == NULL) {
		TrapError (NonFatal, Unix, FnName, "fopen");

		return	False;
	}


	return	True;


	/*----------------------------------------------------------*/
	/*
	**	Do not TrapError() if we cannot open the alternate
	**	file because the error handler may be using us
	**	to get SOME information to the outside world and
	**	this would only result in recursive calls.
	*/
openAlternateFile:

	(void)	sprintf (alternateFile, "/tmp/%dLog", getpid ());

	LogFile_p = fopen (alternateFile, "a+");

	if (LogFile_p == NULL)
		return	False;	/*  This is all we can do. */


	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
WriteLogMsg (msg_p)

char	*msg_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		time_t	timeValue;
		char	timeStamp [15];
		char	*msgTag_p;

		struct
		tm		*tm_p;


	/*----------------------------------------------------------*/
	/*
	**	If a log file is not open then open an alternate.
	*/
	if (LogFile_p == NULL && !OpenLogFile (NULL))
		return;


	/*----------------------------------------------------------*/
	/*
	**	Get and format the time stamp.
	*/
	timeValue = time ((long *) 0);
	tm_p = localtime (&timeValue);

	(void)	sprintf (timeStamp, "%02d/%02d/%02d %02d:%02d",
		tm_p->tm_mon+1, tm_p->tm_mday, tm_p->tm_year, 
		tm_p->tm_hour, tm_p->tm_min);


	/*----------------------------------------------------------*/
	/*
	**	Get the application defined string to be included
	**	in the log message.
	*/
	if (LogMsgTagFn_p != NULL)
		msgTag_p = (*LogMsgTagFn_p) ();


	/*----------------------------------------------------------*/
	/*
	**	Format and write the log message.
	*/
	(void)	fprintf (LogFile_p, "%s %s %s\n",
		timeStamp, msgTag_p == NULL ? "" : msgTag_p, msg_p);
	(void)	fflush (LogFile_p);


	return;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
SetLogMsgTagFn (logMsgTagFnp)

char	*(*logMsgTagFnp) ();
{
	LogMsgTagFn_p = logMsgTagFnp;
}
/*==================================================================*/
