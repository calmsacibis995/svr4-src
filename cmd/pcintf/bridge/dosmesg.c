/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/dosmesg.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)dosmesg.c	3.5	LCC);	/* Modified: 16:03:40 7/13/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#ifndef NOLOG

#define	NIL	((char *)0)

#define		NCALLS		4		/* Max `simultaneous' calls */
#define		MAXMSG		32		/* Max message length */

static char *dosCallNames[] = {
		"Terminate Program",
		"Read Keyboard And Echo",
		"Display Character",
		"Auxiliary Input",
		"Auxiliary Output",
		"Print Character",
		"Direct Console I/O",
		"Direct Console Input",
		"Read Keyboard",
		"Display String",
		"Buffered Keyboard Input",
		"Check Keyboad Status",
		"Flush Buffer, Read Keyboard",
		"Reset Disk",
		"Select Disk",
		"Open File",
		"Close File",
		"Search For First Entry",
		"Search For Next Entry",
		"Delete File",
		"Sequential Read",
		"Sequential Write",
		"Create File",
		"Rename File",
		NIL,
		"Get Current Disk",
		"Set Disk Transfer Address",
		"Get Default Drive Data",
		"Get Drive Data",
		NIL,
		NIL,
		NIL,
		NIL,
		"Random Read",
		"Random Write",
		"Get File Size",
		"Set Relative Record",
		"Set Interrupt Vector",
		"Create New PSP",
		"Random Block Read",
		"Random Block Write",
		"Parse File Name",
		"Get Date",
		"Set Date",
		"Get Time",
		"Set Time",
		"Set/Reset Verify Flag",
		"Get Disk Transfer Address",
		"Get MS-DOS Version Number",
		"Keep Process",
		NIL,
		"Control-C Check",
		NIL,
		"Get Interrupt Vector",
		"Get Disk Free Space",
		NIL,
 		"Get/Set Country Data",
		"Create Directory",
		"Remove Directory",
		"Change Current Directory",
		"Create Handle",
		"Open Handle",
		"Close Handle",
		"Read Handle",
		"Write Handle",
		"Delete Directory Entry",
		"Move File Pointer",
		"Get/Set File Attributes",
		"IOCTL Information Related",
		"Duplicate File Handle",
		"Force Duplicate File Handle",
		"Get Current Directory",
		"Allocate Memory",
		"Free Allocated Memory",
		"Set Block",
		"Load And Execute or Overlay",
		"End Process",
		"Get Return Code Child Process",
		"Find First File",
		"Find Next File",
		NIL,
		NIL,
		NIL,
		NIL,
		"Get Verify State",
		NIL,
		"Change Directory Entry",
		"Get/Set Date/Time of File",
		"Get/Set Allocation Strategy",
		"Get Extended Error",
		"Create Temporary File",
		"Create New File",
		"Lock/Unlock",
		NIL,
		"Get Machine Name/Printer Setup",
		"Get/Make/Cancel Assign List Entry",
		NIL,
		NIL,
		"Get PSP",
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL
	};

#define	MAXDOSCALL	(sizeof dosCallNames / sizeof dosCallNames[0])

static char     cNameBuf[NCALLS][MAXMSG];   /* Call name buffers */

static int      cNameSlot;              /* Next slot of callName to use */

char *dosCName(callCode)
unsigned int callCode;
{
register char   *callName;              /* Call name string */
register char   *nameBuf;               /* cNameBuf buffer used this time */

	/* Use least recently used cNameBuf slot to hold return value */
	nameBuf = cNameBuf[cNameSlot];
	cNameSlot = ++cNameSlot % NCALLS;

	/* Validate callCode */
	if (callCode > MAXDOSCALL) {
		sprintf(nameBuf, "Call %d/%#x", callCode, callCode);
		return nameBuf;
	} else {
		callName = dosCallNames[callCode];

		/* Empty table slots are unused DOS call codes */
		if (callName == NIL)
			strcpy(nameBuf, "Unused");
		else
			strcpy(nameBuf, callName);
	}

	return nameBuf;
}

/*
   string descriptions of DOS 3.1 error codes
*/

static char *dosErrMsgs[] = {
		"Error 0",
		"Invalid function code.",
		"File not found.",
		"Path not found.",
		"Too many open files (no available handles).",
		"Access denied.",
		"Invalid handle.",
		"Memory control blocks destroyed.",
		"Insufficient memory.",
		"Invalid memory block address.",
		"Invalid enviroment.",
		"Invalid format.",
		"Invalid access code.",
		"Invalid data.",
		NIL,
		"Invalid drive.",
		"Attempt to remove the current directory.",
		"Not same device.",
		"No more files.",
		"Disk is write-protected.",
		"Bad disk unit.",
		"Drive not ready.",
		"Invalid disk command.",
		"CRC error.",
		"Invalid length (disk operation).",
		"Seek error.",
		"Not an MS-DOS disk.",
		"Sector not found.",
		"Out of paper.",
		"Write fault.",
		"Read fault." ,
		"General failure.",
		"Sharing violation.",
		"Lock violation.",
		"Wrong disk.",
		"FCB unavailable.",
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		"Network request not supported.",
		"Remote computer not listening.",
		"Duplicate name on network.",
		"Network name not found.",
		"Network busy.",
		"Network device no longer exists.",
		"Net BIOS command limit exceeded.",
		"Network adapter hardware error.",
		"Incorrect response from network.",
		"Unexpected network error.",
		"Incompatible network adapt.",
		"Print queue full.",
		"Queue not full.",
		"Not enough space for print file.",
		"Network name was deleted.",
		"Access denied.",
		"Network device type incorrect.",
		"Network name not found.",
		"Network name limit was exceeded.",
		"Net BIOS session limit exceeded.",
		"Temporarily paused.",
		"Network request not excepted.",
		"Print or disk redirection is paused.",
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		NIL,
		"File exists.",
		NIL,
		"Cannot make.",
		"Interrupt 24h failure.", 
		"Out of structures.",
		"Already assigned.",
		"Invalid password.",
		"Invalid parameter.",
		"Net write fault."
	};

#define	MAXDOSERR	(sizeof dosErrMsgs / sizeof dosErrMsgs[0])

static char     dErrBuf[NCALLS][MAXMSG];    /* DOS error message buffers */

static int      dErrSlot;               /* Next slot of callName to use */

char *dosEName(errCode)
unsigned int errCode;
{
register char   *errMesg;               /* Error message string */
register char   *errBuf;                /* dErrBuf buffer used this time */

	/* Use least recently used dErrBuf slot to hold return value */
	errBuf = dErrBuf[dErrSlot];
	dErrSlot = ++dErrSlot % NCALLS;

	/* Validate errCode */
	if (errCode > MAXDOSERR) {
		sprintf(errBuf, "Error %d/%#x", errCode, errCode);
		return errBuf;
	} else {
		errMesg = dosErrMsgs[errCode];

		/* Empty table slots are unused DOS error codes */
		if (errMesg == NIL)
			strcpy(errBuf, "Unused");
		else
			strcpy(errBuf, errMesg);
	}

	return errBuf;
}
#endif /* ~NOLOG */
