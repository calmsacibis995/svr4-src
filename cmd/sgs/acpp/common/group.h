/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acpp:common/group.h	1.3"
/* group.h - conditional inclusion directives */

#ifdef INLINE_FUNC
#	define	GR_TRUEPART()	gr_current
	extern int	gr_current;
#else
#	define	GR_TRUEPART()	gr_truepart()
#endif

extern	void	gr_check(	/* void		*/ );
extern	Token * gr_elif(	/* Token *	*/ );
extern	Token * gr_else(	/* Token *	*/ );
extern	Token * gr_endif(	/* Token *	*/ );
extern	Token * gr_if(		/* Token *	*/ );
extern	Token * gr_ifdef(	/* Token *	*/ );
extern	Token * gr_ifndef(	/* Token *	*/ );
extern	void	gr_init(	/* void		*/ );
extern	int	gr_truepart(	/* void		*/ );
