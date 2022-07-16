/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sti:panel/private.h	1.1"

#ifndef PANELS_H
#define PANELS_H

#include <panel.h>

#define _panels_intersect(p1,p2)	(!((p1)->wstarty > (p2)->wendy || \
	    				   (p1)->wendy < (p2)->wstarty || \
					   (p1)->wstartx > (p2)->wendx || \
					   (p1)->wendx < (p2)->wstartx)) 

extern	PANEL	*_Bottom_panel;
extern	PANEL	*_Top_panel;

extern	_obscured_list	*_Free_list;

extern	int	_Free_list_cnt,
		_Panel_cnt;

extern void _intersect_panel ();
extern void _remove_overlap ();
extern int _alloc_overlap ();
extern void _free_overlap ();
extern _obscured_list *_unlink_obs ();

extern char *malloc ();

#endif
