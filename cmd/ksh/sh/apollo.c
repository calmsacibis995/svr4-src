/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/apollo.c	1.2.3.1"

#include        "defs.h"
#include	<invoke.h>
#include	<errno.h>


#define PATHLEN	256


static int conlist[] = {0,1,2,3,-1};

int exec_here(com)
register char **com;
{
	register char *prog = com[1];
	char **arge;
	register char *path;
	char iname[PATHLEN];
	VOID (*oldsig)();
	int sig;
	int xitval;
	path = prog;
	/* see if program name contains a / */
	if(strchr(prog,'/')==0)
	{
		if((path = path_absolute(prog))==NULL)
			sh_failed(prog,e_found);
		stak_end(path+strlen(path));
	}
	arge = env_gen();
	oldsig = signal(SIGQUIT,SIG_DFL);
	io_sync();
	errno = 0;
	xitval = invokeve(path,INV_WAIT,conlist,com+1,arge);
	if(errno==ENOEXEC)
	{
		char *savcom = com[0];
		if(get_shell(path,iname)<0)
			sh_fail(e_exec);
		com[0] = iname;
		xitval = invokeve(iname,INV_WAIT,conlist,com,arge);
		com[0] = savcom;
	}
	signal(SIGQUIT,oldsig);
	if(xitval>=0)
	{
		if(sig=(xitval&0177))
		{
			if(sig==2)
				sh_fault(sig);
			if(*sh.sigmsg[sig])
			{
				if(output!=ERRIO)
					p_setout(ERRIO);
				if((st.states&PROMPT)==0)
					p_prp(utos((long)getpid(),10),SP);
				p_str(sh.sigmsg[sig],NL);
				p_setout(output);
				xitval = sig|SIGFLG;
			}
		}
		else
			xitval >>= 8;
	}
	else if(xitval == -1)
		sh_failed(prog,e_exec);
	return(xitval);
}

