/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:oh/evstr.c	1.12"

#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<ctype.h>
#include	<signal.h>
#include	"wish.h"
#include	"token.h"
#include	"fm_mn_par.h"
#include	"var_arrays.h"
#include	"terror.h"
#include	"eval.h"
#include	"moremacros.h"
#include	"interrupt.h"

char *pline();
char **parselist();

/* Global variables used in "eval" (see sys/eval.c) */
extern int EV_retcode;
extern int EV_backquotes;
extern int Lasttok;

/*
 * EVAL_STRING sets up calls to the evaluator (eval) given the
 * expected descriptor type
 */ 
extern void  intr_handler();

char *
eval_string(defstring, flags)
char *defstring;
int *flags;
{
	char *retval;
	int savecode;
	int savequotes;
	int savetok;
	int tmpflags;

	/*
	 * Save globals used in "eval" (in case eval_string is
	 * called while "eval" function is still active ... this 
	 * save/restore should ultimately be done in eval.c) 
	 */
	savecode = EV_retcode;
	savequotes = EV_backquotes;
	savetok = Lasttok;

	

/* 	If interrupts are suppose to be enabled, 
 *	set up the interrupt handler.
*/
	if (Cur_intr.interrupt)
	    sigset(SIGINT, intr_handler);

	switch(*flags & RETS) {
	case RET_BOOL:
		retval = pline(defstring, flags);
		if (retval && (strCcmp(retval, "false") == 0)) {
			free(retval);
			retval = NULL;
		}
		break;
	case RET_INT:
	case RET_STR:
		if (!(retval = pline(defstring, flags)))
			retval = strsave(nil);
		break;
	case RET_ARGS:
	case RET_LIST:
		retval = (char *) parselist(defstring, flags, '\0', '\0');
		break;
	}
	sigignore(SIGINT);	/* ignore interrupts */
	if (Cur_intr.skip_eval == TRUE)	/* if (eval was interrupted) */
	{
	    Cur_intr.skip_eval = FALSE;
	    Cur_intr.interrupt = FALSE;
	    tmpflags = RET_ARGS;
	    retval = eval_string(Cur_intr.oninterrupt,  &tmpflags);
	}


	/*
	 * Restore globals used in "eval"
	 */
	EV_retcode = savecode;
	EV_backquotes = savequotes;
	Lasttok = savetok;
	return(retval);
}

/*
** Calls eval() repeatedly to generate one string.
*/
char *
pline(line, pflags)
char *line;
int *pflags;
{
    register char *str;
    char *ret;
    IOSTRUCT *in, *out;
    int firstime;
    int more;

    if (!line)
	return(NULL);
    /* could be useful in future */
    /* if (strcspn(line, "'\"`$\\") == strlen(line)) */
    /* return(strsave(line)); */
    ret = nil;
    firstime = TRUE;
    in = io_open(EV_USE_STRING|EV_READONLY, line);
    out = io_open(EV_USE_STRING, NULL);
    if (strcspn(line, "`$") == strlen(line))
	io_size(out, strlen(line) + 1);
    while (eval(in, out, EV_TOKEN)) 
    {
        /* for booleans, return code has precidence over the string generated: */
	if (EV_retcode && ((*pflags & RETS) == RET_BOOL))   /* abs k16 */
	{			
	    ret = NULL;
	    break;
	}

	if (firstime)
	{
	    char *str;

	    str = io_string(out);
	    if (strncmp(str, "vary", 4) == 0)
	    {
/*		if (*pflags & EVAL_ONCE)
**		    *pflags = (*pflags & (RETS | FREEIT)) | EVAL_SOMETIMES;
** abs k15 */
		*pflags = (*pflags & (RETS | FREEIT)) | EVAL_ALWAYS;
		io_seek(out, 0);
	    }
	    else if (strncmp(str, "const", 5) == 0)
	    {
/**		if (*pflags & EVAL_SOMETIMES)    abs k15 */
		*pflags = (*pflags & (RETS | FREEIT)) | EVAL_ONCE;
		io_seek(out, 0);
	    }
	    else
		firstime = FALSE;
	    free(str);
	}
	if (!firstime)
	    putac(' ', out);
    }
    /* remove trailing space */
    if (!firstime)
	unputac(out);
    if (ret)
	ret = io_string(out);
    io_close(out);
    io_close(in);
    return(ret);
}

/*
** Repeatedly calls "eval" to get a list of tokens.
*/
char **
parselist(list, pflags, beg, end)
char *list;
register int *pflags;
char beg;
char end;
{
    IOSTRUCT *in, *out;
    register char **hold;
    char *str;
    int more;
    int squig_flag;

    hold = (char **) array_create(sizeof(char *), 5);
    if (!(list && *list))
	return(hold);

    /* flag to tell eval whether `{' and `}' have special meaning */
    squig_flag = *pflags & EV_SQUIG;
	
    in = io_open(EV_USE_STRING|EV_READONLY, list);
    out = io_open(EV_USE_STRING, NULL);
    if (eval(in, out, EV_TOKEN | squig_flag)) {
	str = io_ret_string(out);
	if (str && *str == '\0') {
	    io_seek(out, 0);
	    if (eval(in, out, EV_TOKEN | squig_flag))
		str = io_ret_string(out);
	}
	if (strncmp(str, "vary", 4) == 0) {
/*	    if (*pflags & EVAL_ONCE)
**		*pflags = (*pflags & (RETS | FREEIT)) | EVAL_SOMETIMES;
abs k15 */
	    *pflags = (*pflags & (RETS | FREEIT)) | EVAL_ALWAYS;
	    io_seek(out, 0);
	    if (eval(in, out, EV_TOKEN | squig_flag))
		str = io_ret_string(out);
	}
	else if (strncmp(str, "const", 5) == 0) {
/**	    if (*pflags & EVAL_SOMETIMES)  abs k15 */
	    *pflags = (*pflags & (RETS | FREEIT)) | EVAL_ONCE;
	    io_seek(out, 0);
	    if (eval(in, out, EV_TOKEN | squig_flag))
		str = io_ret_string(out);
	}
	if (beg && (str[0] != beg)) {
	    FILE *fp;
	    char buf[BUFSIZ];

	    if ((fp = fopen(str, "r")) == NULL) {
		warn(NOPEN, nil);
		io_close(in);
		io_close(out);
		return(NULL);
	    }
	    while (fgets(buf, BUFSIZ, fp)) {
		char *p;

		buf[strlen(buf) -1] = '\0';
		p = strsave(buf);
		hold = (char **) array_append(hold, &p);
	    }
	    io_close(in);
	    io_close(out);
	    return(hold);
	}
	if (!beg) {
	    char *p, *q;

	    str = strsave(str);
	    if (EV_backquotes && (p = strtok(str, " \t\n"))) {
		for (; p; p = strtok(NULL, " \t\n")) {
		    q = strsave(p);
		    hold = (char **) array_append(hold, &q);
		}
		free(str);
	    }
	    else
		hold = (char **) array_append(hold, &str);
	}
	io_seek(out, 0);
	while (eval(in, out, EV_TOKEN | squig_flag)) {
	    if (EV_retcode != 0) {
		array_trunc(hold);
		break;
	    }
	    str = io_string(out);
	    io_seek(out, 0);
	    if (!end || (*str != end))
		hold = (char **) array_append(hold, &str);
	    else if (str)
		free(str);
	}
    }
    io_close(out);
    io_close(in);
    return(hold);
}
