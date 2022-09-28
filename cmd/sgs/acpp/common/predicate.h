/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident   "@(#)acpp:common/predicate.h	1.4"

extern Token *	pd_assert( /* Token * */ );
extern void	pd_init( /* void */ );
extern void	pd_option( /* char *  */ );
extern void	pd_preassert( /* void  */ );
extern Token *	pd_replace( /* Token * */ );
extern Token *	pd_unassert( /* Token * */ );

