/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/pci_ipc.h	1.1"
/* SCCSID(@(#)pci_ipc.h	3.2	LCC);	/* Modified: 16:32:19 7/13/87 */

/****************************************************************************

	Copyright (c) 1985 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

****************************************************************************/

#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

/*
 * Extended Protocol Header for Locus Computing Corporation PC-Interface
 * Bridge. The information for doing IPC messages is contained in the
 * hdr.text field of the incoming packet. Overlay the following
 * structures to get at the information.
 */

struct msgget_type {
	char 	code;
	key_t	key;
	int	flag;
};

struct msgsnd_type {
	char 	code;
	int	id;
	int	size;
	int	flag;
	struct	msgbuf message;
};

struct msgrcv_type {
	char	code;
	int 	id;
	int	size;
	int	flag;
	long	type;
	struct	msgbuf message;
};

struct msgctl_type {
	char	code;
	int	id;
	int	command;
	struct msqid_ds	buffer;
}; 
