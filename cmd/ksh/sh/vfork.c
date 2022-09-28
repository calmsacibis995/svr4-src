/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/vfork.c	1.3.3.1"

#include	"defs.h"
#include	"jobs.h"
#include	"sym.h"
#include	"builtins.h"

/*
 * This module is provided to allow the Shell to work with vfork instead
 * of fork.  With vfork the data area is shared by parent and child.
 * Save state variables at fork and make Shell variables copy on write.
 * Restore everything to previous state when fork_exit is called and
 * terminate process.
 */


/*
 * Get the interpreter name given a script file 
 * The first line must be of the form #!  <iname>.
 * Returns 1 if <iname> is found, 0 otherwise
 */
int     get_shell(name,iname)
char *name;
char *iname;
{
	register int c;
	register int state = 0;
	register int fd;
	int n;
	char *cp;
	int	rval = 0;
	char buffer[256];
	cp = nam_strval(SHELLNOD);
	/* don't use csh */
	if(strcmp(path_basename(cp),"csh")==0)
		cp = 0;
	strcpy(iname,cp?cp:"/bin/sh");
	if((fd=open(name,0))<0)
		return(-1);
	n = read(fd,buffer,sizeof(buffer));
	cp = buffer;
	while(n-- > 0)
	{
		c = *cp++;
		switch(state)
		{
			case 0:
				if(c!='#')
					goto out;
				break;

			case 1:
				if(c!='!')
					goto out;
				break;

			case 2:
				if(c==' ' || c =='\t')
					continue;
			default:
				if(c=='\n')
				{
					*iname = 0;
					rval = 1;
					goto out;
				}
				*iname++ = c;
		}
		state++;
	}
out:
	close(fd);
	return(rval);
}

#ifdef VFORK
/* The following structure contains the variables that must be saved */
struct f_save
{
	struct	f_save	*f_save_fork;
	struct dolnod	*f_savearg;
	STKPTR		f_staksave;
	struct sh_scoped	f_st;
	struct jobs	f_jobstat;
	struct fileblk	*f_iotable[NFILE];
	int		f_inpipe[2];
	int		f_outpipe[2];
 	int		*f_sh_inp;
 	int		*f_sh_outp;
};

/* The following routines are defined by this module */
int	vfork_check();
void	vfork_restore();
int	vfork_save();

static struct f_save *save_fork;	/* most recently saved data */

/*
 * Save state on fork
 */

int	vfork_save()
{
	register struct f_save *fp;
	register int i;
	if((fp = new_of(struct f_save,0))==0)
		return(-1);
	stak_begin();
	fp->f_save_fork = save_fork;
	save_fork = fp;
	fp->f_staksave = stak_word();
	fp->f_st = st;
	fp->f_jobstat = job;
	job.pwlist = 0;
	fp->f_savearg = arg_use();
	bcopy(io_ftable,fp->f_iotable,sizeof(fp->f_iotable));
	for(i=0; i < NFILE; i++)
		fp->f_iotable[i] = io_ftable[i];
 	fp->f_sh_inp = sh.inpipe;
 	fp->f_sh_outp = sh.outpipe;
	if(sh.inpipe)
	{
		fp->f_inpipe[0] = sh.inpipe[0];
		fp->f_inpipe[1] = sh.inpipe[1];
	}
	if(sh.outpipe)
	{
		fp->f_outpipe[0] = sh.outpipe[0];
		fp->f_outpipe[1] = sh.outpipe[1];
	}
	st.states |= VFORKED;
	return(0);
}

/*
 * Restore state and exit
 */

void	vfork_restore()
{
	register struct f_save *fp = save_fork;
	STKPTR fstaksvp;

	if((st.states&VFORKED)==0)
		return;
 	sh.inpipe = fp->f_sh_inp;
 	sh.outpipe = fp->f_sh_outp;
	if(sh.inpipe)
	{
		sh.inpipe[0] = fp->f_inpipe[0];
		sh.inpipe[1] = fp->f_inpipe[1];
	}
	if(sh.outpipe)
	{
		sh.outpipe[0] = fp->f_outpipe[0];
		sh.outpipe[1] = fp->f_outpipe[1];
	}
	bcopy(fp->f_iotable,io_ftable,NFILE*sizeof(char*));
	st = fp->f_st;
	job = fp->f_jobstat;
	arg_free(fp->f_savearg,0);
	fstaksvp = fp->f_staksave;
	save_fork = fp->f_save_fork;
	free(fp);
	stak_reset(fstaksvp);
}


/*
 * returns non-zero if process should vfork, 0 otherwise
 * we do not vfork for functions and built-ins in the background
 */
int	vfork_check(t)
union anynode *t;
{
	register union anynode	*tf;
	register struct argnod *arg;
	register char *arg0 = NIL;
	struct namnod *np;
	int bltno;
	/* simple command */
	if((t->tre.tretyp&COMMSK)==TCOM)
		return(1);
	tf = t->fork.forktre;
	if((tf->com.comtyp&COMMSK)!=TCOM)
		return(0);
	/* background command */
	arg = tf->com.comarg;
	bltno = tf->com.comtyp>>(COMBITS+1);
	/* can't vfork assignments or most built-ins */
	if(arg==0 || bltno > SYSLOGIN)
		return(0);
	if(tf->com.comtyp&COMSCAN)
	{
		if(arg->argflag&A_RAW)
			arg0 = arg->argval;
	}
	else
		arg0 = *(((struct dolnod*)arg)->dolarg+1);
	/* no vfork if not sure */
	if(arg0==NIL)
		return(0);
	/* eliminate functions */
	if((np=nam_search(arg0,sh.var_tree,0)) &&  np->value.namval.ip)
		return(0);
	/* command substitution with i/o redirection use fork */
	if((t->tre.tretyp&FCOMSUB) && t->tre.treio==(struct ionod*)0)
		return(0);
	return(2);
}
#endif /* VFORK */

#ifdef NOBCOPY
/*
 * copy <n> bytes from <sp> to <dp>
 */ 

int bcopy(sp,dp,n)
register char *sp, *dp;
register int n;
{
	while(n-- >0)
		*dp++ = *sp++;
}
#endif /* NOBCOPY */
