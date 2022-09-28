/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/qreason.c	1.2.3.1"
char *
qreason(retcode)
{
	char	*status;

	switch(retcode) {
	  case  0:
	  case 10:
	  case 20:
		status = "was successful";
		break;

	  case  1:
	  case 11:
	  case 21:
		status = "failed";
		break;

	  case  2:
	  case 12:
	  case 22:
		status = "partially failed";
		break;

	  case  3:
	  case 13:
	  case 23:
		status = "was terminated due to user request";
		break;

	  case  4:
	  case 14:
	  case 24:
		status = "was suspended (administration)";
		break;

	  case  5:
	  case 15:
	  case 25:
		status = "was suspended (interaction required)";
		break;

	  case 99:
		status = "failed (internal error)";
		break;

	  default:
		status = "failed with an unrecognized error code.";
		break;
	}

	return(status);
}

