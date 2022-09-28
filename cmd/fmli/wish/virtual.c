/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:wish/virtual.c	1.14"

#include	<stdio.h>
#include	<ctype.h>
#include	"wish.h"
#include	"token.h"
#include	"vtdefs.h"
#include	"actrec.h"
#include	"slk.h"
#include	"moremacros.h"
#include 	"message.h"

/*
 * Caution: MAX_ARGS is defined in several files and should ultimately reside
 * in wish.h 
 */
#define MAX_ARGS	25
extern char	*Args[MAX_ARGS];
extern int	Arg_count;
extern bool	Nobang;

static bool	Command_mode = FALSE;		/* abs k17 */

token
virtual_stream(t)
register token	t;
{
    char	*s;
    register token	c;
    char	*tok_to_cmd();

    if ( t > 037 && t < 0177 )
	return t;

    Arg_count = 0;
    if (s = tok_to_cmd(t))
	t = cmd_to_tok(s);
    if (t == TOK_COMMAND)
    {
	/* single equals sign is correct, here */
	if (Command_mode = !Command_mode)
	{
	    token	done_cmd();
	    char	*cur_cmd();

	    get_string(done_cmd, "--> ", cur_cmd(), 0, TRUE,
		       "$VMSYS/OBJECTS/Menu.h6.list", NULL);
	    t = TOK_NOP;
	}
	else
	    t = TOK_CANCEL;
    }
    else
    {
	if (t >= TOK_SLK1 && t <= TOK_SLK8)
	    t = slk_token(t);
    }
    return t;
}

static token
done_cmd(s, t)
char	*s;
token	t;
{
    char *strchr();

    if (t == TOK_CANCEL)
	t = TOK_NOP;
    else
    {
	int i;

	/* Remove all blanks in the beginning of the command line */

        while(*s && isspace(*s))
	    s++;
	if (s[0] == '!')	/* execute shell cmd from cmd line */
	    if (Nobang)		/* feature disabled by application developer */
	    {
		mess_temp("Command ignored: the ! prefix is disabled in this application");
		mess_lock();
		t = TOK_NOP;
	    }
	    else
	    {
		char	*tok_to_cmd();

		t = TOK_OPEN;
		for (i=0; i < 5; i++)
		    if (Args[i])
			free(Args[i]); /* les */

		Args[0] = strsave("OPEN");
		Args[1] = strsave("EXECUTABLE");
		Args[2] = strsave("${SHELL:-/bin/sh}");
		Args[3] = strsave("-c");
		Args[4] = strsave(&s[1]);
		Arg_count = 5;
	    }
	else
	{
	    set_Args(s);

	    /* changed if's to switch and added security clauses. abs k17 */

	    t = cmd_to_tok(Args[0]);
	    switch(t)
	    {
	        case TOK_NOP:
	        {
		    /* change to unknown_command which becomes a goto or
		    ** open (see global_stream() ) unless command was
		    ** entered from command line while Nobang is set;
		    ** in this case only change to unknown_command if
		    ** it will turn into a goto.  abs k17
		    */
		    if (!Nobang || (i = atoi(Args[0])) && wdw_to_ar(i) &&
			strspn(Args[0], "0123456789") == strlen(Args[0]))
			t = TOK_UNK_CMD;
		    else
		    {
			mess_temp("Command ignored: open is disabled in this application");
			mess_lock();
		    }
		    break;
		}
	        case TOK_NUNIQUE:
	        {
		    char msg[MESSIZ];

		    sprintf(msg, "Command '%s' not unique.  Type more of its name.", Args[0]);
		    mess_temp(msg);
		    t = TOK_NOP;
		    break;
		}
	        case TOK_RUN:	/* added clause.  abs k17 */
	        {
		    if (Nobang)
		    {
			mess_temp("Command ignored: run is disabled in this application");
			mess_lock();
			t = TOK_NOP;
		    }
		    break;
		}
	        case TOK_OPEN:	/* added clause.  abs k17 */
	        {
		    if (Nobang)
		    {
			mess_temp("Command ignored: open is disabled in this application");
			mess_lock();
			t = TOK_NOP;
		    }
		    break;
		}
	        default:
		{
		    if (t < 0)
			t = do_app_cmd(); /* Application defined command */
		    break;
		}
	    }
	}
    }
    Command_mode = FALSE; 
    return t;
}

set_Args(s)
char *s;
{
	for (Arg_count = 0; Arg_count < (MAX_ARGS - 1); Arg_count++) {
		while (*s && isspace(*s))
			s++;
		if (*s == '\0')
			break;

		if (Args[Arg_count])
			free(Args[Arg_count]); /* les */

		Args[Arg_count] = s;

		while (*s && !isspace(*s))
			s++;
		if (*s != '\0')
			*s++ = '\0';
		Args[Arg_count] = strsave(Args[Arg_count]);
#ifdef _DEBUG
		_debug(stderr, "Args[%d] = '%s'\n", Arg_count, Args[Arg_count]);
#endif
	}

	if (Args[Arg_count])
		free(Args[Arg_count]); /* les */

	Args[Arg_count] = NULL;
}
