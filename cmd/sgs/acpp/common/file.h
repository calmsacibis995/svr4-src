/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acpp:common/file.h	1.12"
/*	Definitions for file.c		Steve Adamski */

extern	void	fl_addincl(	/* char *	*/);
#ifdef VIMAL
extern int	fl_baseline(	/* void */ );
#endif
extern	char*	fl_basename( 	/* void */ );
extern	FILE*	fl_curfile(	/* void */ );
extern	char*	fl_curname(	/* void */ );
extern	int	fl_dotisource(	/* void */ );
extern	Token *	fl_error(	/* Token * */);
extern	Token*	fl_include(	/* Token * */);
extern	void	fl_init( 	/* void */   );
extern	int	fl_isoriginal(	/* void */ );
extern	Token*	fl_line(	/* Token * */);
extern	Token*	fl_lineinfo(	/* Token * */);
extern	void	fl_next( /* char*, FILE* */ );
extern	int	fl_numerrors(	/* void	*/);
extern	int	fl_numwarns(	/* void	*/);
extern	void	fl_prev(	/* void */ );
extern	void	fl_sayline(	/* void */ );
extern	int	fl_stdhdr(	/* void */);
extern	void	fl_stdir(	/* char * */);
extern	void	fl_fatal();
extern	void	fl_tkerror();
extern	void	fl_tkwarn();
#ifdef __STDC__
extern	void	fl_uerror(const char *, ...);
extern	void	fl_warn(const char *, ...);
#else
extern        void    fl_uerror( /* char *  */);
extern        void    fl_warn(   /* char *  */);
#endif
