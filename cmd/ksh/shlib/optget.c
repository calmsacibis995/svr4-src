/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/optget.c	1.3.3.1"
/*
 * commmand line option parse assist
 *
 *	-- or ++ terminates option list
 *
 *	return:
 *		0	no more options
 *		'?'	unknown option opt_option
 *		':'	option opt_option requires an argument
 *		'#'	option opt_option requires a numeric argument
 *
 *	conditional compilation:
 *
 *		KSHELL	<opt># disabled
 */

#ifdef KSHELL
#include "sh_config.h"
#endif

int		opt_index;		/* argv index			*/
int		opt_char;		/* char pos in argv[opt_index]	*/
#ifndef KSHELL
long		opt_num;		/* # numeric argument		*/
#endif
char*		opt_arg;		/* {:,#} string argument	*/
char		opt_option[3];		/* current flag {-,+} + option	*/

extern char*	strchr();

#ifndef KSHELL
extern long	strtol();
#endif

int
optget(argv, opts)
register char**	argv;
char*		opts;
{
	register int	c;
	register char*	s;

	for (;;)
	{
		if (!opt_char)
		{
			if (!opt_index) opt_index++;
			if (!(s = argv[opt_index]) || (opt_option[0] = *s++) != '-' && opt_option[0] != '+' || !*s) return(0);
			if (*s++ == opt_option[0] && !*s)
			{
				opt_index++;
				return(0);
			}
			opt_char++;
		}
		if (opt_option[1] = c = argv[opt_index][opt_char++]) break;
		opt_char = 0;
		opt_index++;
	}
	opt_arg = 0;
#ifndef KSHELL
	opt_num = 0;
#endif
	if (c == ':' || c == '#' || c == '?' || !(s = strchr(opts, c))) c = '?';
	else if (*++s == ':' || *s == '#')
	{
		if (argv[opt_index][opt_char]) opt_arg = &argv[opt_index++][opt_char];
		else if (!(opt_arg = argv[++opt_index])) c = ':';
		else opt_index++;
#ifndef KSHELL
		if (*s == '#' && opt_arg)
		{
			char*	e;

			opt_num = strtol(opt_arg, &e, 0);
			if (*e) c = '#';
		}
#endif
		opt_char = 0;
	}
	return(c);
}
