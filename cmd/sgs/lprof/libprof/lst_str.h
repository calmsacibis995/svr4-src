/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libprof:lst_str.h	1.1"
/******STRUCTURE FOR MAINTAINING USED FUNCTION LISTS******/


/*******USED FUNCTION LIST*******/

struct caFUNCLIST
   {	char	*name;			 /*   Function Entry Name  */
	struct	caFUNCLIST   *next_func; /* Pointer to next element*/
    };
