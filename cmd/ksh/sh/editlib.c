/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/editlib.c	1.2.3.1"
/*
 * Miscellaneous routine needed for standalone library for edit modes
 */

#include	"io.h"
#include	"terminal.h"
#include	"history.h"
#include	"edit.h"
#undef read

#define e_create	"cannot create"

extern char *strrchr();
extern char *getenv();
char opt_flag;
char ed_errbuf[IOBSIZE+1];
struct fileblk *io_ftable[NFILE];
static struct fileblk outfile = { ed_errbuf, ed_errbuf, ed_errbuf+IOBSIZE, 2, IOWRT};
static int	editfd;
static int	output = 0;

/*
 * read routine with edit modes
 */

int read(fd,buff,n)
char *buff;
{
	register int r;
	register int flag;
	register char *sp;
	static char beenhere;
	if(fd==editfd && beenhere==0)
	{
		beenhere++;
		hist_open();
		sp  = getenv("VISUAL");
		if(sp==0)
			sp = getenv("EDITOR");
		if(sp)
		{
			if(strrchr(sp,'/'))
				sp = strrchr(sp,'/')+1;
			if(strcmp(sp,"vi") == 0)
				opt_flag = EDITVI;
			else if(strcmp(sp,"emacs")==0)
				opt_flag = EMACS;
			else if(strcmp(sp,"gmacs")==0)
				opt_flag = GMACS;
		}
	}
	flag = (fd==editfd?opt_flag&EDITMASK:0);
	switch(flag)
	{
		case EMACS:
		case GMACS:
			tty_set(-1);
			r = emacs_read(fd,buff,n);
			break;

		case VIRAW:
		case EDITVI:
			tty_set(-1);
			r = vi_read(fd,buff,n);
			break;
		default:
#ifdef SYSCALL
			r = syscall(3,fd,buff,n);
#else
			r = rEAd(fd,buff,n);
#endif /* SYSCALL */
	}
	if(fd==editfd && hist_ptr && (opt_flag&NOHIST)==0 && r>0)
	{
		/* write and flush history */
		int c = buff[r];
		buff[r] = 0;
		hist_eof();
		p_setout(hist_ptr->fixfd);
		p_str(buff,0);
		hist_flush();
		buff[r] = c;
	}
	return(r);
}


/*
 * enable edit mode <mode> on file number <fd>
 * the NOHIST bit can also be set to avoid writing the history file
 * <fd> cannot be file two
 */

int	set_edit(fd,mode)
{
	opt_flag = mode;
	if(editfd==2)
		return(-1);
	editfd = fd;
}

/*
 *  flush the output queue and reset the output stream
 */

void	p_setout(fd)
register int fd;
{
	register struct fileblk *fp;
	if(!io_ftable[fd])
		io_ftable[fd] = &outfile;
	fp = io_ftable[fd];
	fp->last = fp->base + IOBSIZE;
	fp->flag &= ~(IOREAD|IOERR|IOEOF);
	if(output==fd)
		return;
	if(io_ftable[fd]==io_ftable[output])
		p_flush();
	output = fd;
}

/*
 * flush the output if necessary and null terminate the buffer
 */

void p_flush()
{
	register struct fileblk *fp = io_ftable[output];
	register int count;
	if(fp && (count=fp->ptr-fp->base))
	{
		if(write(output,fp->base,count) < 0)
			fp->flag |= IOERR;
		/* leave previous buffer as a null terminated string */
		*fp->ptr = 0;
		fp->ptr = fp->base;
	}
}

/*
 * print a given character
 */

void	p_char(c)
register int c;
{
	register struct fileblk *fp = io_ftable[output];
	if(fp->ptr >= fp->last)
		p_flush();
	*fp->ptr++ = c;
}

/*
 * print a string optionally followed by a character
 */

void	p_str(string,c)
register char *string;
int c;
{
	register struct fileblk *fp = io_ftable[output];
	register int cc;
	while(1)
	{
		if((cc= *string)==0)
			cc = c,c = 0;
		else
			string++;
		if(cc==0)
			break;
		if(fp->ptr >= fp->last)
			p_flush();
		*fp->ptr++ = cc;
	}
}

/*
 * copy string a to string b and return pointer to end of string b
 */

char *ed_movstr(a,b)
register char *a,*b;
{
	while(*b++ = *a++);
	return(--b);
}

/*
 * print and error message and exit
 */

ed_failed(name,message)
char *name,*message;
{
	p_setout(ERRIO);
	p_str(name,' ');
	p_char(':');
	p_char(' ');
	p_str(message,'\n');
	exit(2);
}


#ifndef F_DUPFD
#   define F_DUPFD	0
static int fcntl(f1,type,arg)
register int arg;
{
	struct stat statbuf;
	if(type==F_DUPFD)
	{
		register int fd;
		/* find first non-open file */
		while(arg < NFILE &&  (fstat(arg,&statbuf)>=0))
			arg++;
		if(arg >= NFILE)
			return(-1);
		fd = dup2(f1, arg);
		return(fd);
	   }
	else 
		return(0);
}
#endif	/* F_DUPFD */

/*
 * print a prompt
 */
void pr_prompt(string)
register char *string;
{
	register int c;
	register char *dp = editb.e_prbuff;
#ifdef BSD
	int mode;
#include	<sys/ioctl.h>
	mode = LFLUSHO;
	ioctl(ERRIO,TIOCLBIC,&mode);
#endif	/* BSD */
	p_setout(ERRIO);
	while(c= *string++)
	{
		if(dp < editb.e_prbuff+PRSIZE)
			*dp++ = c;
		p_char(c);
	}
	*dp = 0;
}

