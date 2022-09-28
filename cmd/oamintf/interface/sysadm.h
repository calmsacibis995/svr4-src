/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:interface/sysadm.h	1.2.2.2"

#define SYSADM	"sysadm"
#define POWER	"powerdown"
#define MOUNT	"mountfsys"
#define UMOUNT	"umountfsys"
#define SETUP	"setup"
#define CHECK	"checkfsys"
#define MAKE	"makefsys"

#define NOOPTS		0
#define PWR_CMD		1
#define MNT_CMD		2
#define UMNT_CMD	3

#define FMLI		"/usr/bin/fmli"
#define CMD_OBJ		"oam.cmd"
#define INIT_OBJ	"oam.init"
#define FORM_OBJ	"Form.sysadm"
#define TEXT_OBJ	"Text.sysadm"
#define MENU_OBJ	"Menu.sysadm"
