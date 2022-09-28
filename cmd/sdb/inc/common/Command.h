/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Command.h	1.1"

// debugger command language -- top-level interfaces

#ifndef	Command_h
#define	Command_h

// The standard output and error output environment
// must be set up before calling either of these interfaces.

extern	int	CommandFile(FILE *);		// read commands from open file
extern	int	CommandString(char *);		// read commands from string
extern	int	CommandScreen();		// read commands in screeen mode


// This sets up the environment and calls CommandFile(stdin)
extern	int	LineMode();			// enter line mode interface


// This executes debugger actions when a process stops at a breakpoint
class	Process;
extern	int	BreakAction( Process * procp );
// This one executes debugger actions when a process stops on a signal
extern	int	SignalAction( Process * procp );


// This displays the contents of a collected command
struct command;
extern	void	ShowCommand(command *);

#endif	/* Command_h */
