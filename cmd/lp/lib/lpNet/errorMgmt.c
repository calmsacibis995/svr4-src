/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/lpNet/errorMgmt.c	1.3.2.1"

/*===================================================================*/
/*
*/
#include	<unistd.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<tiuser.h>
#include	"errorMgmt.h"
#include	"debug.h"

extern	int	errno;			/*  UNIX errno		*/
extern	int	t_errno;		/*  TLI library errno	*/
extern	char	*sys_errlist [];	/*  UNIX errlist	*/
extern	char	*t_errlist [];		/*  TLI errlist		*/
/*===================================================================*/

/*===================================================================*/
/*
**
*/
void
TrapError (class, type, fnName1_p, msg_p)

errorClass	class;
errorType	type;
char		*fnName1_p;
char		*msg_p;
{
	/*---------------------------------------------------------*/
	/*
	*/
		char	msgBuffer [512];
		char	trace [128];
		char	errnoMsg [32];
		char	*class_p	= NULL;
		char	*type_p		= NULL;
		char	*fnName2_p	= NULL;
		char	*systemMsg_p	= NULL;
	static	char	FnName []	= "TrapError";

	ENTRYP
	TRACE (class)
	TRACE (type)
	TRACEs (fnName1_p)
	TRACEs (msg_p)
	/*---------------------------------------------------------*/
	/*
	**	Determine the class of the error.
	*/
	switch (class) {
	case	Fatal:
		class_p = "Fatal";
		break;

	case	NonFatal:
		class_p = "NonFatal";
		break;

	default:
		class_p = "<Default: Fatal>";
		break;
	}


	/*---------------------------------------------------------*/
	/*
	**	Determine the type of the error.
	*/
	if (type == TLI && t_errno == TSYSERR)
		type = Unix;

	switch (type) {
	case	Unix:
		TRACE (errno)
		type_p = "Unix";
		systemMsg_p = sys_errlist [errno];
		if (systemMsg_p == NULL)
		{
			(void) sprintf (errnoMsg, "errno= %d", errno);
			systemMsg_p = errnoMsg;
		}
		if (msg_p == NULL)
			fnName2_p = NULL;
		else
			fnName2_p = msg_p;
		break;

	case	TLI:
		TRACE (t_errno)
		type_p = "TLI";
		systemMsg_p = t_errlist [t_errno];
		if (systemMsg_p == NULL)
		{
			(void) sprintf (errnoMsg, "t_errno= %d", t_errno);
			systemMsg_p = errnoMsg;
		}
		if (msg_p == NULL)
			fnName2_p = NULL;
		else
			fnName2_p = msg_p;
		break;

	case	XdrEncode:
	case	XdrDecode:
		type_p = "XDR";
		systemMsg_p = "Failed to encode/decode structure.";
		if (msg_p == NULL)
			fnName2_p = NULL;
		else
			fnName2_p = msg_p;
		break;
		
	case	Internal:
		type_p = "Internal";
		if (msg_p == NULL)
			systemMsg_p = "<Unspecified Message>";
		else
			systemMsg_p = msg_p;
		fnName2_p = NULL;
		break;

	default:
		type_p = "<Default: Internal>";
		if (msg_p == NULL)
			systemMsg_p = "<Unspecified Message>";
		else
			systemMsg_p = msg_p;
		fnName2_p = NULL;
		break;
	}


	/*---------------------------------------------------------*/
	/*
	*/
	if (fnName1_p == NULL)
		fnName1_p = "?";

	(void)
	sprintf (trace, "(%s%s%s)", fnName1_p,
		(fnName2_p == NULL ? "" : "/"),
		(fnName2_p == NULL ? "" : fnName2_p));

	TRACEs (class_p)
	TRACEs (type_p)
	TRACEs (trace)
	TRACEs (systemMsg_p)
	(void) sprintf (msgBuffer,
	       "ERROR:  class=%s, type=%s, trace=%s, %s",
	       class_p, type_p, trace, systemMsg_p);

	WriteLogMsg (msgBuffer);

	if (class == NonFatal)
		return;


	Exit (1);
}
/*===================================================================*/
