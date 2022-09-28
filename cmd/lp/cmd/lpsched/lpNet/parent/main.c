/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpsched/lpNet/parent/main.c	1.3.2.1"

/*=================================================================*/
/*
*/
#include	"lpNet.h"
#include	"boolean.h"
#include	"errorMgmt.h"
#include	"debug.h"

/*----------------------------------------------------------*/
/*
**	Global variables.
*/
	processInfo	ProcessInfo;

/*=================================================================*/

/*=================================================================*/
/*
*/
int
main (argc, argv)

int	argc;
char	*argv [];
{
	/*----------------------------------------------------------*/
	/*
	*/
		boolean	done;
	static	char	FnName []	= "main";

	/*----------------------------------------------------------*/
	/*
	*/
	lpNetInit (argc, argv);

	done = False;

	while (! done)
		switch (ProcessInfo.processType) {
		case	ParentProcess:
			lpNetParent ();
			break;

		case	ChildProcess:
			lpNetChild ();
			break;

		default:
			TrapError (Fatal, Internal, FnName,
			"Unknown processType.");
		}

	return	0;	/*  Never reached.	*/
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
Exit (exitCode)

int	exitCode;
{
	if (exitCode == 0)
		WriteLogMsg ("Normal process termination.");
	else
		WriteLogMsg ("Abnormal process termination.");

	exit (exitCode);
}
/*==================================================================*/
