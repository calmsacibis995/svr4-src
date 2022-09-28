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
#ident	"@(#)fmli:wish/getstring.c	1.15.1.1"

#include	<stdio.h>
#include	<ctype.h>
#include        <curses.h>
#include	"wish.h"
#include	"vtdefs.h"
#include	"token.h"
#include	"actrec.h"
#include	"slk.h"
#include	"winp.h"
#include	"moremacros.h"

struct prompt {
    token	(*myfunc)();
    struct actrec	*rec;
    char	*prompt;
    char	*init;
    bool	helpismenu;
    char	*helpfile;
    char	*helptitle;
    int	flags;
};

static struct prompt	p = { NULL, NULL, NULL, NULL, 0};
static struct actrec	*prompt_actrec;

extern	int Vflag;

static int
prompt_noncur(a, all)
struct actrec *a;
bool all;
    
{
#ifdef _DEBUG
    _debug(stderr, "clearing PROMPT MODE\n");
#endif
    wgo(0, 0);
    wclrwin(VT_UNDEFINED);	/* abs W2 */
    return SUCCESS;
}

static int
prompt_ctl(rec, cmd, a1, a2, a3, a4, a5, a6)
struct actrec	*rec;
int	cmd;
int	a1, a2, a3, a4, a5, a6;
{
#ifdef _DEBUG
    _debug(stderr, "prompt_ctl(0x%x, %d, %d, %d)\n", rec, cmd, a1, a2);
#endif
    return vt_ctl(rec->id, cmd, a1, a2, a3, a4, a5, a6);
}

static token
prompt_stream(rec, t)
struct actrec	*rec;
register token	t;
{
    register char	*s;
    char *fldval, *getfield();
    
    switch (t = field_stream(t)) {
    case TOK_SAVE:
	fldval = getfield(NULL, NULL);
	s = strsave(fldval);
	break;
    case TOK_CANCEL:
	s = NULL;
	break;
    case TOK_NOP:
    case TOK_HELP:
	return t;
    default:
	return t | TOK_ERROR;
    }
    ar_backup();
    /*
     * MUST clear the message line before doing the token operation
     * because (temporary) messages may be generated via ar_backup()
     * which calls ar_current() (yuk!!);
     */ 
    mess_temp("");
    t = (*p.rec->fcntbl[AR_ODSH])(p.rec, (*p.myfunc)(s, t));
    if (s)
	free(s);
    return t;
}

static int
prompt_current(rec)
register struct actrec	*rec;
{
    ifield *fld, *deffield();
    
    p.rec = rec->backup;
    vt_current(rec->id);
    wgo(0, 0);
    wclrwin(VT_UNDEFINED);	/* abs W2 */
    if (p.prompt)
	wputs(p.prompt, NULL);
    fld = deffield();
    setfieldflags(fld, p.flags);
    gotofield(fld, 0, 0);
    if (p.init) {
	putfield(fld, p.init);
	gotofield(fld, 0, min(fld->cols - 1, strlen(p.init)));
    }
    return SUCCESS;
}

static int
prompt_help()
{
    if (Vflag) {
	if (p.helpismenu)
	    return objop("OPEN", "MENU", p.helpfile, p.helptitle, NULL);
	else
	    return objop("OPEN", "TEXT", "$VMSYS/OBJECTS/Text.help", p.helpfile, p.helptitle, NULL);
    }
    else
	return(SUCCESS);
}

void
get_string(func, s1, s2, flags, helpismenu, helpfile, helptitle)
token	(*func)();
char	*s1;
char	*s2;
int	flags;
int	helpismenu;
char	*helpfile;
char	*helptitle;
{
    p.myfunc = func;
    p.prompt = s1;
    if (s2)
	p.init = s2;
    else
	p.init = strnsave("", 0); /* abs */
    p.flags = flags | I_SCROLL;   /* abs. added srcoll flag */
    p.helpismenu = helpismenu;
    p.helpfile = helpfile;
    p.helptitle = helptitle;
    if (prompt_actrec == NULL) {
	struct actrec	a;
	struct actrec	*ar_create();
	extern struct slk	Echslk[];
	
	a.id = CMD_WIN;
	a.flags = AR_SKIP;
	a.odptr = a.path = NULL;
	a.fcntbl[AR_CLOSE]  = AR_NOP;
	a.fcntbl[AR_REINIT] = AR_NOP;
	a.fcntbl[AR_REREAD] = AR_NOP; /* abs k18 */
	a.fcntbl[AR_HELP] = prompt_help;
	a.fcntbl[AR_NONCUR] = prompt_noncur;
	a.fcntbl[AR_CURRENT] = prompt_current;
	a.fcntbl[AR_TEMP_CUR] = prompt_current;
	a.fcntbl[AR_CTL] = prompt_ctl;
	a.fcntbl[AR_ODSH] = (int (*)())prompt_stream; /* added cast. abs */
	a.lifetime = AR_PERMANENT;
	a.slks = Echslk;
	prompt_actrec = ar_create(&a);
    }
    ar_current(prompt_actrec, FALSE); /* abs k15 */
}
